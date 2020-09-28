
#include "GLayer.h"
#include "GClipmap.h"
#include "CHgtFile.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <math.h>

double RAW_FILE_DEGREE = 45;


GLayer::GLayer(GClipmap* clipmapPointer, float inDegree, float inHgtFileDegree, float inScale, int inLOD, int inLayerIndex, 
               int inHgtSkipping, int inHgtFileResolution, int inRawSkipping, int inRawFileResolution, int inN) : clipmap(clipmapPointer){

    readDegree = inDegree;
    HgtFiledegree = inHgtFileDegree;

    scale = inScale;
    LOD = inLOD;
    layerIndex = inLayerIndex;
    HgtSkipping = inHgtSkipping;
    HgtFileResolution = inHgtFileResolution;
    rawSkipping = inRawSkipping;
    rawFileResolution = inRawFileResolution;

    firstGothrough = true;
    n = inN;

    heightMap = new QImage(n, n, QImage::Format_Grayscale16);
    pixelMap =  new QImage(n-1, n-1, QImage::Format_RGB888);

    QOpenGLTexture* heightTexture;
    textureBegginingX = 0;  //x
    textureBegginingY = n - 1;  //y

    program = clipmapPointer->program;
    vaoA = clipmapPointer->vaoA;
    vaoB = clipmapPointer->vaoB;
    vaoC = clipmapPointer->vaoC;
    vaoD = clipmapPointer->vaoD;
    vaoE = clipmapPointer->vaoE;
    vaoF = clipmapPointer->vaoF;

    indexA_Buffer = clipmapPointer->indexA_Buffer;
    indexB_Buffer = clipmapPointer->indexB_Buffer;
    indexC_Buffer = clipmapPointer->indexC_Buffer;
    indexD_Buffer = clipmapPointer->indexD_Buffer;
    indexE_Buffer = clipmapPointer->indexE_Buffer;
    indexF_Buffer = clipmapPointer->indexF_Buffer;


    drawMode = &(clipmapPointer->drawingMode);

    cumputeOffsets();
}


void GLayer::mapPixelDataIntoTexture(double tlon, double tlat) {
    
    double latDifference, lonDifference;
    double lonLeft, lonRight;
    double latTop, latDown;


    if (firstGothrough == true) {
        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        
        oldLon = tlon;
        oldLonLeft = lonLeft;
        oldLonRight = lonRight;

        oldLat = tlat;
        oldLatTop = latTop;
        oldLatDown = latDown;
    }

    //checking if there was a movement
    lonDifference = (-1) * (oldLon - tlon - fmod(oldLon - tlon, readDegree)) / readDegree;
    latDifference = (-1) * (oldLat - tlat - fmod(oldLat - tlat, readDegree)) / readDegree;


    if (firstGothrough == true || (abs(latDifference) > n - 1 || abs(lonDifference) > n - 1)) {

        fullRawTextureReading(lonLeft, lonRight, latTop, latDown);

        //updating old lon and lat
        oldLat = oldLat + latDifference * readDegree;
        oldLon = oldLon + lonDifference * readDegree;
        oldLonLeft =  oldLonLeft  + lonDifference * readDegree;
        oldLonRight = oldLonRight + lonDifference * readDegree;
        oldLatTop =  oldLatTop  + latDifference * readDegree;
        oldLatDown = oldLatDown + latDifference * readDegree;
        
        //reseting texture beggining coordinate
        textureBegginingX = 0;  //x
        textureBegginingY = n - 1;  //y
            
        firstGothrough = false;
    }
    else if (lonDifference != 0 || latDifference != 0) {

        point texBegHor, texBegVer;
        
        //computing coordinates of corners of new moved clipmap 
        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        //checking texture begginings for horizontal and vertical reading based on movement case 
        computeTextureOffsets(latDifference, lonDifference, &texBegHor, &texBegVer);
        
        //pixelMap->save("mapka.png", nullptr, -1);

        if (latDifference != 0)   
            horizontalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, texBegHor);
          
        //pixelMap->save("poHor.png", nullptr, -1);

        if (lonDifference != 0)
            verticalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, texBegVer);

        //pixelMap->save("poVer.png", nullptr, -1);

        pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
        pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);
        program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);

        if (latDifference > 0) 
            textureBegginingY = (int)(textureBegginingY + latDifference) % (n - 1);
        if (latDifference < 0) 
            textureBegginingY = (int)(textureBegginingY + latDifference + n - 1) % (n - 1);
        if (lonDifference > 0) 
            textureBegginingX = (int)(textureBegginingX + lonDifference) % (n - 1);
        if (lonDifference < 0) 
            textureBegginingX = (int)(textureBegginingX + lonDifference + n - 1) % (n - 1);

        

        oldLat = oldLat + latDifference * readDegree;
        oldLon = oldLon + lonDifference * readDegree;
        oldLatTop  = oldLatTop  + latDifference * readDegree;
        oldLatDown = oldLatDown + latDifference * readDegree;
        oldLonLeft  = oldLonLeft  + lonDifference * readDegree;
        oldLonRight = oldLonRight + lonDifference * readDegree;
    }

}

void GLayer::mapHeightDataIntoTexture(double tlon, double tlat) {


    //int n = 15;  //should be (2^n)-1 e.g. 3,7,15,31
 
    double lonLeft = tlon - horizontalOffset * readDegree;    //Left Right
    double lonRight = lonLeft + (n - 1) * readDegree;

    double latTop = tlat + verticalOffset * readDegree;   //Top Down
    double latDown = latTop - (n - 1) * readDegree;

    float maxTilesLon, maxTilesLat;

    //Finding top left tile 
    if (lonLeft < 0) {
        if (fmod(lonLeft, HgtFiledegree) != 0)
            maxTilesLon = lonLeft - fmod(lonLeft, HgtFiledegree) - HgtFiledegree;
        else
            maxTilesLon = lonLeft;
    }
    else {
        maxTilesLon = lonLeft - fmod(lonLeft, HgtFiledegree);
    }

    if (latTop < 0) {
        maxTilesLat = latTop - fmod(latTop, HgtFiledegree) - 30;
    }
    else {
        if (fmod(latTop * 2, HgtFiledegree) != 0)
            maxTilesLat = latTop - fmod(latTop, HgtFiledegree) + HgtFiledegree;
        else
            maxTilesLat = latTop;
    }


    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition; //0 means left wall, 1 middle, 2 right wall


    int howManyToReadX;
    int howManyToReadY;

    QString filePath;



    if ((maxTilesLon + HgtFiledegree) - lonLeft > lonRight - lonLeft) {
        horizontalPosition = 3; //left and right wall at once 
    }
    else {
        horizontalPosition = 0; //left wall
    }

    for (float i = maxTilesLon; i < lonRight; i += HgtFiledegree) {

        if (latTop - (maxTilesLat - HgtFiledegree) > latTop - latDown) {
            verticalPosition = 3; //left and right wall at once 
        }
        else {
            verticalPosition = 0; //top wall
        }

        imageOffsetY = 0;

        for (float j = maxTilesLat; j > latDown; j -= HgtFiledegree) {

            CHgtFile::sphericalToFilePath(&filePath, i, j, LOD);

            if (horizontalPosition == 0)
                howManyToReadX = ((i + HgtFiledegree) - lonLeft) / readDegree + 1;
            else if (horizontalPosition == 1)
                howManyToReadX = HgtFiledegree / readDegree;
            else if (horizontalPosition == 2)
                howManyToReadX = (lonRight - i) / readDegree + 1;
            else if (horizontalPosition == 3)
                howManyToReadX = (lonRight - lonLeft) / readDegree + 1;

            if (verticalPosition == 0)
                howManyToReadY = (latTop - (j - HgtFiledegree)) / readDegree + 1;
            else if (verticalPosition == 1)
                howManyToReadY = HgtFiledegree / readDegree;
            else if (verticalPosition == 2)
                howManyToReadY = (j - latDown) / readDegree + 1;
            else if (verticalPosition == 3)
                howManyToReadY = (latTop - latDown) / readDegree + 1;



            CHgtFile::loadHeightDataToImage(heightMap, imageOffsetX, imageOffsetY, filePath, verticalPosition, horizontalPosition,
                howManyToReadX, howManyToReadY, HgtFileResolution, HgtSkipping);
            

            if (j - 2 * HgtFiledegree > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;
        }

        if (i + 2 * HgtFiledegree < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;
    }
   
    bool resutl = heightMap->save("zjencie.png", nullptr, -1);

    heightTexture = new QOpenGLTexture(*heightMap, QOpenGLTexture::DontGenerateMipMaps);
    //heightTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    heightTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    heightTexture->create();


   // heightTexture->bind(0, QOpenGLTexture::DontResetTextureUnit);
   // int x = clipmap->HgtTextureLocation;
   // program->setUniformValue(clipmap->HgtTextureLocation, 0);
    //// given some `width`, `height` and `data_ptr`
    //heightTexture->setSize(15, 15, 1);
    ////heightTexture->
    //heightTexture->setFormat(QOpenGLTexture::R16U);
    //heightTexture->allocateStorage();
    //heightTexture->setData(*heightMap, QOpenGLTexture::DontGenerateMipMaps);

}


void GLayer::cumputeOffsets() {

    if (layerIndex == 0) {
        horizontalOffset = (n - 1) / 2;
        verticalOffset = (n - 1) / 2;
    }
    else {

        double horOffHigherLayer = clipmap->layer[layerIndex - 1].horizontalOffset;
        double verOffHigherLayer = clipmap->layer[layerIndex - 1].verticalOffset;

        int m = (n + 1) / 4;

        horizontalOffset = horOffHigherLayer / 2 + m - 1;
        verticalOffset = verOffHigherLayer / 2 + m - 1;

    }
}

void GLayer::buildLayer(double tlon, double tlat) {
    

    QVector2D offsets;


    if (!layerIndex == 0) {

        if (clipmap->layer[layerIndex - 1].positionHorizontal == 0)  //left side 
            offsets.setX(tlon - horizontalOffset * readDegree);
        else if (clipmap->layer[layerIndex - 1].positionHorizontal == 1) //right side
            offsets.setX(tlon - (horizontalOffset+1) * readDegree);
        
        if (clipmap->layer[layerIndex - 1].positionVertical == 0) //down side    
            offsets.setY(tlat - (verticalOffset) * readDegree);
        else if (clipmap->layer[layerIndex - 1].positionVertical == 1) //top side    
            offsets.setY(tlat - (verticalOffset+1) * readDegree);
    }
    else {
        offsets.setX(tlon - horizontalOffset * readDegree);
        offsets.setY(tlat - verticalOffset * readDegree);
    }
    
    program->setUniformValue("worldOffset", offsets);

    program->setUniformValue("levelScaleFactor", QVector2D(scale, scale));
    program->setUniformValue("layerIndex", layerIndex);

    //mapHeightDataIntoTexture(tlon, tlat);

    mapPixelDataIntoTexture(tlon, tlat);

    program->setUniformValue("n", n);
    program->setUniformValue("texOffset", QVector2D(textureBegginingX, textureBegginingY));

    int m = (n + 1) / 4 - 1;

    drawA(0.0,    0.0);
    drawA(m,      0.0);
    drawB(2*m,    0.0);
    drawA(2*m+2,  0.0);
    drawA(3*m+2,  0.0);
    drawA(3*m+2,  m);
    drawC(3*m+2,  2*m);
    drawA(3*m+2,  2*m+2);
    drawA(3*m+2,  3*m+2);
    drawA(2*m+2,  3*m+2);
    drawB(2*m,    3*m+2);
    drawA(m,      3*m+2);
    drawA(0.0,    3*m+2);
    drawA(0.0,    2*m+2);
    drawC(0.0,    2*m);
    drawA(0.0,    m);

    if (clipmap->activeLevelOfDetail == layerIndex) {
        drawA(m,        m);
        drawB(2*m,      m);
        drawA(2*m+2,    m);
        drawC(2*m+2,    2*m);
        drawA(2*m+2,    2*m+2);
        drawB(2*m,      2*m+2);
        drawA(m,        2*m+2);
        drawC(m,        2*m);
        drawF(2*m,      2*m);
    }
    else {

        if (fillerPositionVertical == 0) { //down wall
            drawD(m, m);

            if (fillerPositionHorizontal == 0) { //left wall
                drawE(m, m+1);
            }
            else if (fillerPositionHorizontal == 1) { //right wall
                drawE(3*m+1, m+1);
            }

        }
        else if (fillerPositionVertical == 1) { //top wall

            drawD(m, 3*m+1);
            
            //drawD(3.0, 10.0);

            if (fillerPositionHorizontal == 0) { //left wall
                drawE(m, m);
            }
            else if (fillerPositionHorizontal == 1) { //right wall
                drawE(3*m+1, m);
            }
        }

    }

}


void GLayer::checkHowManyPixelsToReadFromRaw_X(int* howManyToReadX, int horizontalPosition, double lonLeftHor, double lonRightHor, float i) {
    float tmp;

    if (horizontalPosition == 0)
        tmp = ((i + RAW_FILE_DEGREE) - lonLeftHor) / readDegree + 1;
    else if (horizontalPosition == 1)
        tmp = RAW_FILE_DEGREE / readDegree;
    else if (horizontalPosition == 2)
        tmp = (lonRightHor - i) / readDegree + 1;
    else if (horizontalPosition == 3)
        tmp = (lonRightHor - lonLeftHor) / readDegree + 1;

    if (fmod(tmp, 1) > 0.5)
        *howManyToReadX = tmp + 1;
    else
        *howManyToReadX = tmp;
}

void GLayer::checkHowManyPixelsToReadFromRaw_Y(int *howManyToReadY, int verticalPosition, double latTopHor, double latDownHor, float j) {
    float tmp;

    if (verticalPosition == 0) 
        tmp = (latTopHor - (j - RAW_FILE_DEGREE)) / readDegree + 1;
    else if (verticalPosition == 1) 
        tmp = RAW_FILE_DEGREE / readDegree;
    else if (verticalPosition == 2) 
        tmp = (j - latDownHor) / readDegree + 1;
    else if (verticalPosition == 3) 
        tmp = (latTopHor - latDownHor) / readDegree;
     
    if (fmod(tmp, 1) > 0.5)
        *howManyToReadY = tmp + 1;
    else
        *howManyToReadY = tmp;

}

void GLayer::checkRawFileOffset_X(int* filePositionHorizontalOffset, int horizontalPosition, double lonLeftHor, float maxTilesLon) {

    if (horizontalPosition == 0)
        *filePositionHorizontalOffset = (lonLeftHor - maxTilesLon) / readDegree;
    else if (horizontalPosition == 1)
        *filePositionHorizontalOffset = 0;
    else if (horizontalPosition == 2)
        *filePositionHorizontalOffset = 0;
    else if (horizontalPosition == 3)
        *filePositionHorizontalOffset = (lonLeftHor - maxTilesLon) / readDegree;
   
}

void GLayer::checkRawFileOffset_Y(int* filePositionVerticalOffset, int verticalPosition, double latTopHor, float maxTilesLat) {
    
    if (verticalPosition == 0) 
        *filePositionVerticalOffset = (maxTilesLat - latTopHor) / readDegree;
    else if (verticalPosition == 1) 
        *filePositionVerticalOffset = 0;   
    else if (verticalPosition == 2)       
        *filePositionVerticalOffset = 0;  
    else if (verticalPosition == 3)    
        *filePositionVerticalOffset = (maxTilesLat - latTopHor) / readDegree;
 
}

void GLayer::fullRawTextureReading(double lonLeft, double lonRight, double latTop, double latDown) {

    float maxTilesLon, maxTilesLat;
    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeft, latTop);
   
    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall

    int howManyToReadX;
    int howManyToReadY;
    int filePositionHorizontalOffset;
    int filePositionVerticalOffset;

    QString filePath;

    if ((maxTilesLon + RAW_FILE_DEGREE) - lonLeft > lonRight - lonLeft) 
        horizontalPosition = 3; //left and right wall at once 
    else 
        horizontalPosition = 0; //left wall
    

    for (float i = maxTilesLon; i < lonRight; i += RAW_FILE_DEGREE) {

        if (latTop - (maxTilesLat - RAW_FILE_DEGREE) > latTop - latDown)
            verticalPosition = 3; //left and right wall at once 
        else 
            verticalPosition = 0; //top wall
        

        imageOffsetY = 0;

        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeft, lonRight, i);
        checkRawFileOffset_X(&filePositionHorizontalOffset, horizontalPosition, lonLeft, maxTilesLon);


        for (float j = maxTilesLat; j > latDown; j -= RAW_FILE_DEGREE) {

            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTop, latDown, j);
            checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTop, maxTilesLat);

            CRawFile::sphericalToFilePath(&filePath, i, j, LOD);
            CRawFile::loadPixelDataToImage(pixelMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY,
                rawFileResolution, rawSkipping,
                filePositionHorizontalOffset, filePositionVerticalOffset);

            if (j - 2 * RAW_FILE_DEGREE > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;
        }

        if (i + 2 * RAW_FILE_DEGREE < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;
    }

    pixelMap->setPixelColor(0, 0, QColor(0, 255, 14));
    pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    pixelTexture->create();
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);

    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);
}

void GLayer::horizontalBlockRawTextureReading(int lonDifference, int latDifference, 
                double lonLeft, double lonRight, double latTop, double latDown, point texBegHor) {

    float maxTilesLon, maxTilesLat;
    int orientationPointX, orientationPointY;

    double lonLeftHor = oldLonLeft + lonDifference * readDegree;
    double lonRightHor = oldLonRight + lonDifference * readDegree;


    //finding latitude of corners
    double latTopHor;
    double latDownHor;
    if (latDifference > 0) {
        latTopHor = oldLatTop + latDifference * readDegree;
        latDownHor = oldLatTop;
    }
    else if (latDifference < 0) {
        latTopHor = oldLatDown;
        latDownHor = oldLatDown + latDifference * readDegree;
    }

    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftHor, latTopHor);

    int horizontalPosition; //0 means top edge, 1 middle edge, 2 bottom edge of clipmap
    int verticalPosition;   //0 means left edge, 1 middle edge, 2 right edge of clipmap
    int filePositionHorizontalOffset;
    int filePositionVerticalOffset;

    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int howManyToReadX;
    int howManyToReadY;
 

    QString filePath;

    if ((maxTilesLon + RAW_FILE_DEGREE) - lonLeftHor > lonRightHor - lonLeftHor)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall

    int readingCheckX = n - 1;
    int readingCheckY;


    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightHor; i += RAW_FILE_DEGREE) {

        

        readingCheckY = abs(latDifference);

        if (latTopHor - (maxTilesLat - RAW_FILE_DEGREE) > latTopHor - latDownHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftHor, lonRightHor, i);
        checkRawFileOffset_X(&filePositionHorizontalOffset, horizontalPosition, lonLeftHor, maxTilesLon);

        if (readingCheckX - howManyToReadX < 0)
           howManyToReadX = readingCheckX;

        readingCheckX -= howManyToReadX;

        
        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownHor; j -= RAW_FILE_DEGREE) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopHor, latDownHor, j);
            checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTopHor, maxTilesLat);
            
            if (readingCheckY - howManyToReadY < 0)
                howManyToReadY = readingCheckY;

            readingCheckY -= howManyToReadY;

            if (howManyToReadY > 0 && howManyToReadX > 0) {

                CRawFile::sphericalToFilePath(&filePath, i, j, LOD);

                CRawFile::loadPixelDataToImage2(pixelMap, imageOffsetX, imageOffsetY,
                    filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                    filePositionHorizontalOffset, filePositionVerticalOffset,
                    texBegHor.x, n - 1 - texBegHor.y);
            }


            if (j - 2 * RAW_FILE_DEGREE > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;

        }

        if (i + 2 * RAW_FILE_DEGREE < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;

    }
   
}

void GLayer::verticalBlockRawTextureReading(int lonDifference, int latDifference, 
                double lonLeft, double lonRight, double latTop, double latDown, point texBegVer) {
    
    float maxTilesLon, maxTilesLat;
    int orientationPointX, orientationPointY;

    double lonLeftVer, lonRightVer;
    double latTopVer, latDownVer;

    //finding longitude of corners
    if (lonDifference < 0) {
        lonLeftVer = oldLonLeft + lonDifference * readDegree;
        lonRightVer = oldLonLeft;
    }
    else if (lonDifference > 0) {
        lonLeftVer = oldLonRight;
        lonRightVer = oldLonRight + lonDifference * readDegree;
    }

    if (latDifference > 0) {
        latTopVer = oldLatTop;
        latDownVer = oldLatDown + latDifference * readDegree;
    }
    else if (latDifference == 0) {
        latTopVer = oldLatTop;
        latDownVer = oldLatDown;
    }
    else if (latDifference < 0) {
        latTopVer = oldLatTop + latDifference * readDegree;
        latDownVer = oldLatDown;
    }


    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftVer, latTopVer);

    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int filePositionHorizontalOffset;
    int filePositionVerticalOffset;

    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int howManyToReadX;
    int howManyToReadY;
     
    QString filePath;

    if ((maxTilesLon + RAW_FILE_DEGREE) - lonLeftVer > lonRightVer - lonLeftVer)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall

    int readingCheckX = abs(lonDifference);

    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightVer; i += RAW_FILE_DEGREE) {


        int readingCheckY = n - 1 - abs(latDifference);

        if (latTopVer - (maxTilesLat - RAW_FILE_DEGREE) > latTopVer - latDownVer)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftVer, lonRightVer, i);
        checkRawFileOffset_X(&filePositionHorizontalOffset, horizontalPosition, lonLeftVer, maxTilesLon);

        if (readingCheckX - howManyToReadX < 0)
           howManyToReadX = readingCheckX;

        readingCheckX -= howManyToReadX;

        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownVer; j -= RAW_FILE_DEGREE) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopVer, latDownVer, j);
            checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTopVer, maxTilesLat);

            if (readingCheckY - howManyToReadY < 0)
                howManyToReadY = readingCheckY;

            readingCheckY -= howManyToReadY;          

            if (howManyToReadY > 0 && howManyToReadX > 0) {

                CRawFile::sphericalToFilePath(&filePath, i, j, LOD);
                    
                CRawFile::loadPixelDataToImage2(pixelMap, imageOffsetX, imageOffsetY,
                        filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                        filePositionHorizontalOffset, filePositionVerticalOffset,
                        texBegVer.x, n - 1 - texBegVer.y);
                
            }


            if (j - 2 * RAW_FILE_DEGREE > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;

        }

        if (i + 2 * RAW_FILE_DEGREE < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;

    }
    
    
}

void GLayer::computeTextureOffsets(int latDifference, int lonDifference, point *texBegHor, point *texBegVer) {


    if (latDifference > 0) {
        
        if (lonDifference > 0) {
            texBegHor->x = (textureBegginingX + lonDifference) % (n - 1);
            texBegHor->y = (textureBegginingY + latDifference) % (n - 1);
            texBegVer->x = textureBegginingX;
            texBegVer->y = textureBegginingY;
        }
        else if (lonDifference == 0) {
            texBegHor->x = textureBegginingX;
            texBegHor->y = (textureBegginingY + latDifference) % (n - 1);
        }
        else if (lonDifference < 0) {
            texBegHor->x = (textureBegginingX + lonDifference + n - 1) % (n - 1);
            texBegHor->y = (textureBegginingY + latDifference) % (n - 1);
            texBegVer->x = (textureBegginingX + lonDifference + n - 1) % (n - 1);
            texBegVer->y = textureBegginingY;
        }
    }
    else if (latDifference == 0) {
        
        if (lonDifference > 0) {
            texBegVer->x = textureBegginingX;
            texBegVer->y = textureBegginingY;
        }
        else if (lonDifference < 0) {
            texBegVer->x = (textureBegginingX + lonDifference + n - 1) % (n - 1);
            texBegVer->y = textureBegginingY;
        }
    } 
    else if (latDifference < 0) {
        
        if (lonDifference > 0) {
            texBegHor->x = (textureBegginingX + lonDifference) % (n - 1);
            texBegHor->y = textureBegginingY;
            texBegVer->x = textureBegginingX;
            texBegVer->y = (textureBegginingY + latDifference + n - 1) % (n - 1);
        }
        else if (lonDifference == 0) {
            texBegHor->x = textureBegginingX;
            texBegHor->y = textureBegginingY;
        }
        else if (lonDifference < 0) {
            texBegHor->x = (textureBegginingX + lonDifference + n - 1) % (n - 1);
            texBegHor->y = textureBegginingY;
            texBegVer->x = (textureBegginingX + lonDifference + n - 1) % (n - 1);
            texBegVer->y = (textureBegginingY + latDifference + n - 1) % (n - 1);
        }
    }
}

void GLayer::computeNewLonAndLat(double tlon, double tlat, double *lonLeft, double *lonRight, double *latTop, double *latDown) {

    *lonLeft = tlon - horizontalOffset * readDegree; //Left Right
    *lonRight = *lonLeft + (n - 1) * readDegree;
    *latTop = tlat + verticalOffset * readDegree;   //Top Down
    *latDown = *latTop - (n - 1) * readDegree;
}

void GLayer::findingTopLeftFileToRead(float *maxTilesLon, float *maxTilesLat, double lonLeft, double latTop) {

    if (lonLeft < 0) {
        if (fmod(lonLeft, RAW_FILE_DEGREE) != 0)
            *maxTilesLon = lonLeft - fmod(lonLeft, RAW_FILE_DEGREE) - RAW_FILE_DEGREE;
        else
            *maxTilesLon = lonLeft;
    }
    else {
        *maxTilesLon = lonLeft - fmod(lonLeft, RAW_FILE_DEGREE);
    }

    if (latTop < 0) {
        *maxTilesLat = latTop - fmod(latTop, RAW_FILE_DEGREE);
    }
    else {
        if (fmod(latTop * 2, RAW_FILE_DEGREE) != 0)
            *maxTilesLat = latTop - fmod(latTop, RAW_FILE_DEGREE) + RAW_FILE_DEGREE;
        else
            *maxTilesLat = latTop;
    }
}

void GLayer::drawA(float originX, float originY) {
    
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoA->bind();
    indexA_Buffer->bind();
    
    glDrawElements(*drawMode, clipmap->howManyToRenderA, GL_UNSIGNED_INT, 0);
    
    indexA_Buffer->release();
    vaoA->release();
}

void GLayer::drawB(float originX, float originY) {

    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
   
    vaoB->bind();
    indexB_Buffer->bind();
    
    glDrawElements(*drawMode, clipmap->howManyToRenderB, GL_UNSIGNED_INT, 0);
    
    indexB_Buffer->release();
    vaoB->release();
}

void GLayer::drawC(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoC->bind();
    indexC_Buffer->bind();
    glDrawElements(*drawMode, clipmap->howManyToRenderC, GL_UNSIGNED_INT, 0);
    indexC_Buffer->release();
    vaoC->release();
}

void GLayer::drawD(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoD->bind();
    indexD_Buffer->bind();
    glDrawElements(*drawMode, clipmap->howManyToRenderD, GL_UNSIGNED_INT, 0);
    indexD_Buffer->release();
    vaoD->release();
}
 
void GLayer::drawE(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));

    vaoE->bind();
    indexE_Buffer->bind();
    glDrawElements(*drawMode, clipmap->howManyToRenderE, GL_UNSIGNED_INT, 0);
    indexE_Buffer->release();
    vaoE->release();
}

void GLayer::drawF(float originX, float originY) {

    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    //program->setUniformValue("color", QVector3D(R, G, B));

    vaoB->bind();
    indexB_Buffer->bind();

    glDrawElements(*drawMode, 16, GL_UNSIGNED_INT, 0);

    indexB_Buffer->release();
    vaoB->release();
}
