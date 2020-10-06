
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

   // pixelTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    //pixelTexture->create();

    hgtTextureBegginingX = 0;  //x
    hgtTextureBegginingY = n;  //y

    rawTextureBegginingX = 0;  //x
    rawTextureBegginingY = n - 1;  //y


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

        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        fullRawTextureReading(lonLeft, lonRight, latTop, latDown);
        fullHgtTextureReading(lonLeft, lonRight, latTop, latDown);
        

        //reseting old lon and lat
        oldLon = tlon;
        oldLonLeft = lonLeft;
        oldLonRight = lonRight;
        oldLat = tlat;
        oldLatTop = latTop;
        oldLatDown = latDown;

        //reseting texture beggining coordinate
        hgtTextureBegginingX = 0;
        hgtTextureBegginingX = n;
        rawTextureBegginingX = 0;  //x
        rawTextureBegginingY = n - 1;  //y
            
        firstGothrough = false;
    }
    else if (lonDifference != 0 || latDifference != 0) {

        point texBegHor, texBegVer;
        
        //computing coordinates of corners of new moved clipmap 
        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        //checking texture begginings for horizontal and vertical reading based on movement case 
        computeTextureOffsets(latDifference, lonDifference, &rTexBegHor, &rTexBegVer, true);
        computeTextureOffsets(latDifference, lonDifference, &hTexBegHor, &hTexBegVer, false);


        if (latDifference != 0) {
            horizontalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, rTexBegHor);
            horizontalBlockHgtTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, hTexBegHor);
        }
         
        if (lonDifference != 0) {
            verticalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, rTexBegVer);
            verticalBlockHgtTextureReading(lonDifference, latDifference, lonLeft, lonRight, latTop, latDown, hTexBegVer);
        }

        pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
        heightTexture = new QOpenGLTexture(*heightMap, QOpenGLTexture::DontGenerateMipMaps);

        bool success = heightMap->save("hgtTextureLookup.png");
        QColor kolor = heightMap->pixelColor(50,50);

        pixelTexture->bind(layerIndex);
        program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);
        heightTexture->bind(layerIndex + 13);
        program->setUniformValue(clipmap->heightTextureLocation[layerIndex], layerIndex+13);


        if (latDifference > 0) {
            rawTextureBegginingY = (int)(rawTextureBegginingY + latDifference) % (n - 1);
            hgtTextureBegginingY = (int)(hgtTextureBegginingY + latDifference) % (n);
        }
        if (latDifference < 0) {
            rawTextureBegginingY = (int)(rawTextureBegginingY + latDifference + n - 1) % (n - 1);
            hgtTextureBegginingY = (int)(hgtTextureBegginingY + latDifference + n) % (n);
        }
        if (lonDifference > 0) {
            rawTextureBegginingX = (int)(rawTextureBegginingX + lonDifference) % (n - 1);
            hgtTextureBegginingX = (int)(hgtTextureBegginingX + lonDifference) % (n);
        }
        if (lonDifference < 0) {
            rawTextureBegginingX = (int)(rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
            hgtTextureBegginingX = (int)(hgtTextureBegginingX + lonDifference + n) % (n);
        }

        

        oldLat = oldLat + latDifference * readDegree;
        oldLon = oldLon + lonDifference * readDegree;
        oldLatTop  = oldLatTop  + latDifference * readDegree;
        oldLatDown = oldLatDown + latDifference * readDegree;
        oldLonLeft  = oldLonLeft  + lonDifference * readDegree;
        oldLonRight = oldLonRight + lonDifference * readDegree;
    }

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
    program->setUniformValue("rawTexOffset", QVector2D(rawTextureBegginingX, rawTextureBegginingY));
    program->setUniformValue("hgtTexOffset", QVector2D(hgtTextureBegginingX, hgtTextureBegginingY));


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

    if (clipmap->activeLvlOfDetail == layerIndex) {
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


void GLayer::checkHowManyPixelsToReadFromRaw_X(int* howManyToReadX, int horizontalPosition, double lonLeft, double lonRight, float i, float fileDegree, bool rawReading) {
    float tmp;

    if (horizontalPosition == 0)
        tmp = ((i + fileDegree) - lonLeft) / readDegree;
    else if (horizontalPosition == 1)
        tmp = fileDegree / readDegree;
    else if (horizontalPosition == 2)
        tmp = (lonRight - i) / readDegree;
    else if (horizontalPosition == 3)
        tmp = (lonRight - lonLeft) / readDegree; //+1;

    if (rawReading == true) {
        if (fmod(tmp, 1) > 0.5)
            *howManyToReadX = tmp + 1;
        else
            *howManyToReadX = tmp;
    }
    else if (rawReading == false) {
        *howManyToReadX = tmp + 1;
    }

}

void GLayer::checkHowManyPixelsToReadFromRaw_Y(int *howManyToReadY, int verticalPosition, double latTop, double latDown, float j, float fileDegree, bool rawReading) {
    float tmp;

    if (verticalPosition == 0) 
        tmp = (latTop - (j - fileDegree)) / readDegree;
    else if (verticalPosition == 1) 
        tmp = fileDegree / readDegree;
    else if (verticalPosition == 2) 
        tmp = (j - latDown) / readDegree;
    else if (verticalPosition == 3) 
        tmp = (latTop - latDown) / readDegree;
     
    if (rawReading == true) {
        if (fmod(tmp, 1) > 0.5)
            *howManyToReadY = tmp + 1;
        else
            *howManyToReadY = tmp;
    }
    else if (rawReading == false) {
        *howManyToReadY = tmp + 1;
    }
}

void GLayer::checkRawFileOffset_X(int* fileHorizontalOffset, int horizontalPosition, double lonLeft, float maxTilesLon) {

    if (horizontalPosition == 0)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;
    else if (horizontalPosition == 1)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 2)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 3)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;
   
}

void GLayer::checkRawFileOffset_Y(int* fileVerticalOffset, int verticalPosition, double latTop, float maxTilesLat) {
    
    if (verticalPosition == 0) 
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;
    else if (verticalPosition == 1) 
        *fileVerticalOffset = 0;   
    else if (verticalPosition == 2)       
        *fileVerticalOffset = 0;  
    else if (verticalPosition == 3)    
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;
 
}

void GLayer::fullHgtTextureReading(double lonLeft, double lonRight, double latTop, double latDown) {

    float maxTilesLon, maxTilesLat;

    int imageOffsetX = 0, imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int howManyToReadX, howManyToReadY;
    int fileHorizontalOffset, fileVerticalOffset;

    QString filePath;

    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeft, latTop, HgtFiledegree);

//    fstream file;
//    CRawFile::sphericalToHeightFilePath(&filePath, maxTilesLon, maxTilesLat, LOD);
//    file.open(filePath.toUtf8(), fstream::in | fstream::binary);


    if ((maxTilesLon + HgtFiledegree) - lonLeft > lonRight - lonLeft)
        horizontalPosition = 3; //left and right wall at once 
    else
        horizontalPosition = 0; //left wall
     

    int checkHowManyToReadX = 0;

    for (float i = maxTilesLon; i < lonRight; i += HgtFiledegree) {

        if (latTop - (maxTilesLat - HgtFiledegree) > latTop - latDown)
            verticalPosition = 3; //left and right wall at once 
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;

        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeft, lonRight, i, HgtFiledegree, false);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeft, maxTilesLon);


        for (float j = maxTilesLat; j > latDown; j -= HgtFiledegree) {

            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTop, latDown, j, HgtFiledegree, false);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTop, maxTilesLat);

        
            CRawFile::sphericalToHeightFilePath(&filePath, i, j, LOD);
            CRawFile::loadHeightDataToImageFull(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY,
                HgtFileResolution, HgtSkipping,
                fileHorizontalOffset, fileVerticalOffset);


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

  
    heightTexture = new QOpenGLTexture(*heightMap, QOpenGLTexture::DontGenerateMipMaps);
    heightTexture->bind(layerIndex+13, QOpenGLTexture::DontResetTextureUnit);
    
    program->setUniformValue(clipmap->heightTextureLocation[layerIndex], layerIndex+13);

}

void GLayer::fullRawTextureReading(double lonLeft, double lonRight, double latTop, double latDown) {

    float maxTilesLon, maxTilesLat;
   
    int imageOffsetX = 0, imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int howManyToReadX, howManyToReadY;
    int fileHorizontalOffset, fileVerticalOffset;

    QString filePath;

    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeft, latTop, RAW_FILE_DEGREE);

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
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeft, lonRight, i, RAW_FILE_DEGREE, true);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeft, maxTilesLon);


        for (float j = maxTilesLat; j > latDown; j -= RAW_FILE_DEGREE) {

            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTop, latDown, j, RAW_FILE_DEGREE, true);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTop, maxTilesLat);

            CRawFile::sphericalToRawFilePath(&filePath, i, j, LOD);
            CRawFile::loadPixelDataToImageFull(pixelMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY,
                rawFileResolution, rawSkipping,
                fileHorizontalOffset, fileVerticalOffset);

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

    pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);

    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);
}

void GLayer::horizontalBlockHgtTextureReading(int lonDifference, int latDifference,
    double lonLeft, double lonRight, double latTop, double latDown, point texBegHor) {

    float maxTilesLon, maxTilesLat;

    int horizontalPosition; //0 means top edge, 1 middle edge, 2 bottom edge of clipmap
    int verticalPosition;   //0 means left edge, 1 middle edge, 2 right edge of clipmap
    int fileHorizontalOffset, fileVerticalOffset;
    int howManyToReadX, howManyToReadY;
    int imageOffsetX = 0, imageOffsetY = 0;

    QString filePath;



    //finding longitude of corners
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftHor, latTopHor, HgtFiledegree);


    if ((maxTilesLon + HgtFiledegree) - lonLeftHor > lonRightHor - lonLeftHor)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall


    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightHor; i += HgtFiledegree) {



        if (latTopHor - (maxTilesLat - HgtFiledegree) > latTopHor - latDownHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftHor, lonRightHor, i, HgtFiledegree, false);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftHor, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownHor; j -= HgtFiledegree) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopHor, latDownHor, j, HgtFiledegree, false);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopHor, maxTilesLat);


            CRawFile::sphericalToHeightFilePath(&filePath, i, j, LOD);

            CRawFile::loadHeightDataToImagePart(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, HgtFileResolution, HgtSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegHor.x, n - 1 - texBegHor.y);


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

}


void GLayer::horizontalBlockRawTextureReading(int lonDifference, int latDifference, 
                double lonLeft, double lonRight, double latTop, double latDown, point texBegHor) {

    float maxTilesLon, maxTilesLat;

    int horizontalPosition; //0 means top edge, 1 middle edge, 2 bottom edge of clipmap
    int verticalPosition;   //0 means left edge, 1 middle edge, 2 right edge of clipmap
    int fileHorizontalOffset, fileVerticalOffset;
    int howManyToReadX, howManyToReadY;
    int imageOffsetX = 0, imageOffsetY = 0;

    QString filePath;



    //finding longitude of corners
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftHor, latTopHor, RAW_FILE_DEGREE);

 
    if ((maxTilesLon + RAW_FILE_DEGREE) - lonLeftHor > lonRightHor - lonLeftHor)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall


    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightHor; i += RAW_FILE_DEGREE) {

        

        if (latTopHor - (maxTilesLat - RAW_FILE_DEGREE) > latTopHor - latDownHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftHor, lonRightHor, i, RAW_FILE_DEGREE, true);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftHor, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownHor; j -= RAW_FILE_DEGREE) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopHor, latDownHor, j, RAW_FILE_DEGREE, true);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopHor, maxTilesLat);
       

            CRawFile::sphericalToRawFilePath(&filePath, i, j, LOD);

            CRawFile::loadPixelDataToImagePart(pixelMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegHor.x, n - 1 - texBegHor.y);
        

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

void GLayer::verticalBlockHgtTextureReading(int lonDifference, int latDifference,
    double lonLeft, double lonRight, double latTop, double latDown, point texBegVer) {

    float maxTilesLon, maxTilesLat;

    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int fileHorizontalOffset, fileVerticalOffset;
    int howManyToReadX, howManyToReadY;
    int imageOffsetX = 0, imageOffsetY = 0;

    QString filePath;



    //finding longitude of corners
    double lonLeftVer, lonRightVer;
    if (lonDifference < 0) {
        lonLeftVer = oldLonLeft + lonDifference * readDegree;
        lonRightVer = oldLonLeft;
    }
    else if (lonDifference > 0) {
        lonLeftVer = oldLonRight;
        lonRightVer = oldLonRight + lonDifference * readDegree;
    }

    //finding latitude of corners
    double latTopVer, latDownVer;
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftVer, latTopVer, HgtFiledegree);


    if ((maxTilesLon + HgtFiledegree) - lonLeftVer > lonRightVer - lonLeftVer)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall

    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightVer; i += HgtFiledegree) {


        if (latTopVer - (maxTilesLat - HgtFiledegree) > latTopVer - latDownVer)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftVer, lonRightVer, i, HgtFiledegree, false);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftVer, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownVer; j -= HgtFiledegree) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopVer, latDownVer, j, HgtFiledegree, false);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopVer, maxTilesLat);


            CRawFile::sphericalToHeightFilePath(&filePath, i, j, LOD);

            CRawFile::loadPixelDataToImagePart(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, HgtFileResolution, HgtSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegVer.x, n - 1 - texBegVer.y);


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


}

void GLayer::verticalBlockRawTextureReading(int lonDifference, int latDifference, 
                double lonLeft, double lonRight, double latTop, double latDown, point texBegVer) {
    
    float maxTilesLon, maxTilesLat;

    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int fileHorizontalOffset, fileVerticalOffset;
    int howManyToReadX, howManyToReadY;
    int imageOffsetX = 0, imageOffsetY = 0;
     
    QString filePath;



    //finding longitude of corners
    double lonLeftVer, lonRightVer;
    if (lonDifference < 0) {
        lonLeftVer = oldLonLeft + lonDifference * readDegree;
        lonRightVer = oldLonLeft;
    }
    else if (lonDifference > 0) {
        lonLeftVer = oldLonRight;
        lonRightVer = oldLonRight + lonDifference * readDegree;
    }

    //finding latitude of corners
    double latTopVer, latDownVer;
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftVer, latTopVer, RAW_FILE_DEGREE);


    if ((maxTilesLon + RAW_FILE_DEGREE) - lonLeftVer > lonRightVer - lonLeftVer)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall

    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightVer; i += RAW_FILE_DEGREE) {


        if (latTopVer - (maxTilesLat - RAW_FILE_DEGREE) > latTopVer - latDownVer)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonLeftVer, lonRightVer, i, RAW_FILE_DEGREE, true);
        checkRawFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftVer, maxTilesLon);

    
        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownVer; j -= RAW_FILE_DEGREE) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopVer, latDownVer, j, RAW_FILE_DEGREE, true);
            checkRawFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopVer, maxTilesLat);

          
            CRawFile::sphericalToRawFilePath(&filePath, i, j, LOD);
                
            CRawFile::loadPixelDataToImagePart(pixelMap, imageOffsetX, imageOffsetY,
                    filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                    fileHorizontalOffset, fileVerticalOffset,
                    texBegVer.x, n - 1 - texBegVer.y);
            
          
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

void GLayer::computeTextureOffsets(int latDifference, int lonDifference, point *texBegHor, point *texBegVer, bool rawReading) {

    if (rawReading) {
        if (latDifference > 0) {

            if (lonDifference > 0) {
                texBegHor->x = (rawTextureBegginingX + lonDifference) % (n - 1);
                texBegHor->y = (rawTextureBegginingY + latDifference) % (n - 1);
                texBegVer->x = rawTextureBegginingX;
                texBegVer->y = rawTextureBegginingY;
            }
            else if (lonDifference == 0) {
                texBegHor->x = rawTextureBegginingX;
                texBegHor->y = (rawTextureBegginingY + latDifference) % (n - 1);
            }
            else if (lonDifference < 0) {
                texBegHor->x = (rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
                texBegHor->y = (rawTextureBegginingY + latDifference) % (n - 1);
                texBegVer->x = (rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = rawTextureBegginingY;
            }
        }
        else if (latDifference == 0) {

            if (lonDifference > 0) {
                texBegVer->x = rawTextureBegginingX;
                texBegVer->y = rawTextureBegginingY;
            }
            else if (lonDifference < 0) {
                texBegVer->x = (rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = rawTextureBegginingY;
            }
        }
        else if (latDifference < 0) {

            if (lonDifference > 0) {
                texBegHor->x = (rawTextureBegginingX + lonDifference) % (n - 1);
                texBegHor->y = rawTextureBegginingY;
                texBegVer->x = rawTextureBegginingX;
                texBegVer->y = (rawTextureBegginingY + latDifference + n - 1) % (n - 1);
            }
            else if (lonDifference == 0) {
                texBegHor->x = rawTextureBegginingX;
                texBegHor->y = rawTextureBegginingY;
            }
            else if (lonDifference < 0) {
                texBegHor->x = (rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
                texBegHor->y = rawTextureBegginingY;
                texBegVer->x = (rawTextureBegginingX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = (rawTextureBegginingY + latDifference + n - 1) % (n - 1);
            }
        }
    }
    else {
        if (latDifference > 0) {

            if (lonDifference > 0) {
                texBegHor->x = (hgtTextureBegginingX + lonDifference) % (n);
                texBegHor->y = (hgtTextureBegginingY + latDifference) % (n);
                texBegVer->x = hgtTextureBegginingX;
                texBegVer->y = hgtTextureBegginingY;
            }
            else if (lonDifference == 0) {
                texBegHor->x = hgtTextureBegginingX;
                texBegHor->y = (hgtTextureBegginingY + latDifference) % (n);
            }
            else if (lonDifference < 0) {
                texBegHor->x = (hgtTextureBegginingX + lonDifference + n) % (n);
                texBegHor->y = (hgtTextureBegginingY + latDifference) % (n);
                texBegVer->x = (hgtTextureBegginingX + lonDifference + n) % (n);
                texBegVer->y = hgtTextureBegginingY;
            }
        }
        else if (latDifference == 0) {

            if (lonDifference > 0) {
                texBegVer->x = hgtTextureBegginingX;
                texBegVer->y = hgtTextureBegginingY;
            }
            else if (lonDifference < 0) {
                texBegVer->x = (hgtTextureBegginingX + lonDifference + n) % (n);
                texBegVer->y = hgtTextureBegginingY;
            }
        }
        else if (latDifference < 0) {

            if (lonDifference > 0) {
                texBegHor->x = (hgtTextureBegginingX + lonDifference) % (n);
                texBegHor->y = hgtTextureBegginingY;
                texBegVer->x = hgtTextureBegginingX;
                texBegVer->y = (hgtTextureBegginingY + latDifference + n) % (n);
            }
            else if (lonDifference == 0) {
                texBegHor->x = hgtTextureBegginingX;
                texBegHor->y = hgtTextureBegginingY;
            }
            else if (lonDifference < 0) {
                texBegHor->x = (hgtTextureBegginingX + lonDifference + n) % (n);
                texBegHor->y = hgtTextureBegginingY;
                texBegVer->x = (hgtTextureBegginingX + lonDifference + n) % (n);
                texBegVer->y = (hgtTextureBegginingY + latDifference + n) % (n);
            }
        }
    }
}

void GLayer::computeNewLonAndLat(double tlon, double tlat, double *lonLeft, double *lonRight, double *latTop, double *latDown) {

    *lonLeft = tlon - horizontalOffset * readDegree; //Left Right
    *lonRight = *lonLeft + (n - 1) * readDegree;
    *latTop = tlat + verticalOffset * readDegree;   //Top Down
    *latDown = *latTop - (n - 1) * readDegree;
}

void GLayer::findingTopLeftFileToRead(float *maxTilesLon, float *maxTilesLat, double lonLeft, double latTop, double degree) {

    if (lonLeft < 0) {
        if (fmod(lonLeft, degree) != 0)
            *maxTilesLon = lonLeft - fmod(lonLeft, degree) - degree;
        else
            *maxTilesLon = lonLeft;
    }
    else {
        *maxTilesLon = lonLeft - fmod(lonLeft, degree);
    }

    if (latTop < 0) {
        *maxTilesLat = latTop - fmod(latTop, degree);
    }
    else {
        if (fmod(latTop * 2, degree) != 0)
            *maxTilesLat = latTop - fmod(latTop, degree) + degree;
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
