
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
    textureBegginingX = 0;      //x
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

    double tlonConst;
    if (firstGothrough == true)
        tlonConst = tlon;

    double lonTopLeft = tlonConst - horizontalOffset * readDegree; //Left Right
    double lonTopRight = lonTopLeft + (n - 1) * readDegree;

    double latTopLeft = tlat + verticalOffset * readDegree;   //Top Down
    double latDownLeft = latTopLeft - (n - 1) * readDegree;

    if (firstGothrough == true) {
        oldLonTopLeft = lonTopLeft;

        oldLatTopLeft = latTopLeft;
        oldLatDownLeft = latDownLeft;
    }

    lonDifference = (-1)*(oldLonTopLeft - lonTopLeft - fmod(oldLonTopLeft - lonTopLeft, readDegree))/readDegree;
    latDifference = (-1)*(oldLatTopLeft - latTopLeft - fmod(oldLatTopLeft - latTopLeft, readDegree))/readDegree;


    if (lonDifference != 0 || latDifference != 0) {

        if (latDifference != 0) {
            
            verticalRawTextureReading(lonDifference, latDifference, lonTopLeft, lonTopRight, latTopLeft, latDownLeft);
            //int movementCaseX;
            //int movementCaseY;
            //int orientationPointX;
            //int orientationPointY;

            //if (lonDifference > 0)
            //    movementCaseX = 1;
            //else if (lonDifference == 0)
            //    movementCaseX = 0;
            //else if (lonDifference < 0)
            //    movementCaseX = -1;

            //if (latDifference > 0)
            //    movementCaseY = 1;
            //else if (latDifference == 0)
            //    movementCaseY = 0;
            //else if (latDifference < 0)
            //    movementCaseY = -1;

            //double lonTopLeftHor = lonTopLeft + lonDifference * readDegree;
            //double lonTopRightHor = lonTopRight + lonDifference * readDegree;


            ////finding latitude of corners
            //double latTopLeftHor;
            //double latDownLeftHor;
            //if (movementCaseY == 1) {
            //    latTopLeftHor = latTopLeft;
            //    latDownLeftHor = oldLatTopLeft;
            //}
            //else if (movementCaseY == -1) {
            //    latTopLeftHor = oldLatDownLeft;
            //    latDownLeftHor = latDownLeft;
            //}

            //

            ////finding longitude of corners         !!!!!!!!DO OGARNIECIA!!!!!!!
            ////if (movementCaseY == 1) {
            ///*     latTopLeftHor = latTopLeft + latDifference * degree;
            //    latDownLeftHor = latTopLeft;
            //}
            //else if (movementCaseY == -1) {
            //    latTopLeftHor = latDownLeft;
            //    latDownLeftHor = latDownLeft + latDifference * degree;
            //}*/


            ////Finding top left tile 
            //if (lonTopLeftHor < 0) {
            //    if (fmod(lonTopLeftHor, RAW_FILE_DEGREE) != 0)
            //        maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, RAW_FILE_DEGREE) - RAW_FILE_DEGREE;
            //    else
            //        maxTilesLon = lonTopLeftHor;
            //}
            //else {
            //    maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, RAW_FILE_DEGREE);
            //}

            //if (latTopLeftHor < 0) {
            //    maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, RAW_FILE_DEGREE);
            //}
            //else {
            //    if (fmod(latTopLeftHor * 2, RAW_FILE_DEGREE) != 0)
            //        maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, RAW_FILE_DEGREE) + RAW_FILE_DEGREE;
            //    else
            //        maxTilesLat = latTopLeftHor;
            //}

            //int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
            //int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
            //int filePositionHorizontalOffset;
            //int filePositionVerticalOffset;

            //int imageOffsetX = 0;
            //int imageOffsetY = 0;
            //int howManyToReadX;
            //int howManyToReadY;

            //if (movementCaseY == 1) {
            //    textureBegginingY = textureBegginingY + latDifference;

            //    if (textureBegginingY > n - 1)
            //        textureBegginingY = fmod(textureBegginingY, n - 1);               
            //}

            //QString filePath;

            //if ((maxTilesLon + RAW_FILE_DEGREE) - lonTopLeftHor > lonTopRightHor - lonTopLeftHor)
            //    horizontalPosition = 3; //left and right wall at once   
            //else
            //    horizontalPosition = 0; //left wall
            //

            //////////////////////////X///////////////////////////
            //for (float i = maxTilesLon; i < lonTopRightHor; i += RAW_FILE_DEGREE) {

            //    

            //    int readingCheck = abs(latDifference);

            //    if (latTopLeftHor - (maxTilesLat - RAW_FILE_DEGREE) > latTopLeftHor - latDownLeftHor)
            //        verticalPosition = 3; //left and right wall at once   
            //    else
            //        verticalPosition = 0; //top wall

            //    imageOffsetY = 0;


            //    ///////////horizontal
            //    checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonTopLeftHor, lonTopRightHor, i);
            //    checkRawFileOffset_X(&filePositionHorizontalOffset, horizontalPosition, lonTopLeftHor, maxTilesLon);


            //    //////////////////////////Y///////////////////////////
            //    for (float j = maxTilesLat; j > latDownLeftHor; j -= RAW_FILE_DEGREE) {

            //       
            //        checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopLeftHor, latDownLeftHor, j);
            //        checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTopLeftHor, maxTilesLat);


            //        if (readingCheck - howManyToReadY < 0) 
            //            howManyToReadY = readingCheck;
            //   

            //        readingCheck -= howManyToReadY;

            //        if (howManyToReadY > 0) {

            //            CRawFile::sphericalToFilePath(&filePath, i, j, LOD);

            //            CRawFile::loadPixelDataToImage2(pixelMap, imageOffsetX, imageOffsetY,
            //                filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
            //                filePositionHorizontalOffset, filePositionVerticalOffset,
            //                textureBegginingX, n - 1 - textureBegginingY);
            //        }


            //        if (j - 2 * RAW_FILE_DEGREE > latDownLeft)
            //            verticalPosition = 1;   //middle 
            //        else
            //            verticalPosition = 2;   //down wall

            //        imageOffsetY -= howManyToReadY;

            //        
            //    }

            //    if (i + 2 * RAW_FILE_DEGREE < lonTopRight)
            //        horizontalPosition = 1;   //middle 
            //    else
            //        horizontalPosition = 2;   //right wall

            //    imageOffsetX += howManyToReadX;

            //}

            //if (movementCaseY == -1) {
            //    textureBegginingY = textureBegginingY + latDifference;

            //    if (textureBegginingY < 1)
            //        textureBegginingY = n - 1 + textureBegginingY;
            //}
            //
            //oldLatTopLeft  = oldLatTopLeft  + latDifference * readDegree;
            //oldLatDownLeft = oldLatDownLeft + latDifference * readDegree;
        }

        bool result = pixelMap->save("mapka.png", nullptr, -1);

        pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
        pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);
        program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);

        oldLonTopLeft = lonTopLeft;
        
        CCommons::doubleIntoVSConsole(textureBegginingY);
    }

    ////////////////////////////////////////////////////////////////////////////

    if (firstGothrough == true) {//(latDifference > n - 1 || lonDifference > n - 1) {

        float maxTilesLon, maxTilesLat;
        //Finding top left tile 
        if (lonTopLeft < 0) {
            if (fmod(lonTopLeft, RAW_FILE_DEGREE) != 0)
                maxTilesLon = lonTopLeft - fmod(lonTopLeft, RAW_FILE_DEGREE) - RAW_FILE_DEGREE;
            else
                maxTilesLon = lonTopLeft;
        }
        else {
            maxTilesLon = lonTopLeft - fmod(lonTopLeft, RAW_FILE_DEGREE);
        }

        if (latTopLeft < 0) {
            maxTilesLat = latTopLeft - fmod(latTopLeft, RAW_FILE_DEGREE);
        }
        else {
            if (fmod(latTopLeft * 2, RAW_FILE_DEGREE) != 0)
                maxTilesLat = latTopLeft - fmod(latTopLeft, RAW_FILE_DEGREE) + RAW_FILE_DEGREE;
            else
                maxTilesLat = latTopLeft;
        }

        int imageOffsetX = 0;
        int imageOffsetY = 0;
        int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
        int verticalPosition; //0 means left wall, 1 middle, 2 right wall

        int howManyToReadX;
        int howManyToReadY;
        int filePositionHorizontalOffset;
        int filePositionVerticalOffset;

        QString filePath;

        if ((maxTilesLon + RAW_FILE_DEGREE) - lonTopLeft > lonTopRight - lonTopLeft) {
            horizontalPosition = 3; //left and right wall at once 
        }
        else {
            horizontalPosition = 0; //left wall
        }

        for (float i = maxTilesLon; i < lonTopRight; i += RAW_FILE_DEGREE) {

            if (latTopLeft - (maxTilesLat - RAW_FILE_DEGREE) > latTopLeft - latDownLeft) {
                verticalPosition = 3; //left and right wall at once 
            }
            else {
                verticalPosition = 0; //top wall
            }

            imageOffsetY = 0;

                ///////////horizontal
                if (horizontalPosition == 0) {
                    howManyToReadX = ((i + RAW_FILE_DEGREE) - lonTopLeft) / readDegree + 1;
                    filePositionHorizontalOffset = (lonTopLeft - maxTilesLon) / readDegree;
                }
                else if (horizontalPosition == 1) {
                    howManyToReadX = RAW_FILE_DEGREE / readDegree;
                    filePositionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 2) {
                    howManyToReadX = (lonTopRight - i) / readDegree + 1;
                    filePositionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 3) {
                    howManyToReadX = (lonTopRight - lonTopLeft) / readDegree + 1;
                    filePositionHorizontalOffset = (lonTopLeft - maxTilesLon) / readDegree;
                }
                

            for (float j = maxTilesLat; j > latDownLeft; j -= RAW_FILE_DEGREE) {

                CRawFile::sphericalToFilePath(&filePath, i, j, LOD);
                
           
                checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopLeft, latDownLeft, j);
                checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTopLeft, maxTilesLat);


                CRawFile::loadPixelDataToImage(pixelMap, imageOffsetX, imageOffsetY, 
                                               filePath, howManyToReadX, howManyToReadY,
                                               rawFileResolution, rawSkipping, 
                                               filePositionHorizontalOffset, filePositionVerticalOffset);

                if (j - 2 * RAW_FILE_DEGREE > latDownLeft)
                    verticalPosition = 1;   //middle 
                else
                    verticalPosition = 2;   //down wall

                imageOffsetY += howManyToReadY;
            }

            if (i + 2 * RAW_FILE_DEGREE < lonTopRight)
                horizontalPosition = 1;   //middle 
            else
                horizontalPosition = 2;   //right wall

            imageOffsetX += howManyToReadX;
    }

    
    //bool result = pixelMap->save("mapka.png", nullptr, -1);


    pixelMap->setPixelColor(0,0, QColor(0,255,14));
    pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    pixelTexture->create();
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);


    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);

    firstGothrough = false;
    }

}

void GLayer::mapHeightDataIntoTexture(double tlon, double tlat) {


    //int n = 15;  //should be (2^n)-1 e.g. 3,7,15,31
 
    double lonTopLeft = tlon - horizontalOffset * readDegree;    //Left Right
    double lonTopRight = lonTopLeft + (n - 1) * readDegree;

    double latTopLeft = tlat + verticalOffset * readDegree;   //Top Down
    double latDownLeft = latTopLeft - (n - 1) * readDegree;

    float maxTilesLon, maxTilesLat;

    //Finding top left tile 
    if (lonTopLeft < 0) {
        if (fmod(lonTopLeft, HgtFiledegree) != 0)
            maxTilesLon = lonTopLeft - fmod(lonTopLeft, HgtFiledegree) - HgtFiledegree;
        else
            maxTilesLon = lonTopLeft;
    }
    else {
        maxTilesLon = lonTopLeft - fmod(lonTopLeft, HgtFiledegree);
    }

    if (latTopLeft < 0) {
        maxTilesLat = latTopLeft - fmod(latTopLeft, HgtFiledegree) - 30;
    }
    else {
        if (fmod(latTopLeft * 2, HgtFiledegree) != 0)
            maxTilesLat = latTopLeft - fmod(latTopLeft, HgtFiledegree) + HgtFiledegree;
        else
            maxTilesLat = latTopLeft;
    }


    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition; //0 means left wall, 1 middle, 2 right wall


    int howManyToReadX;
    int howManyToReadY;

    QString filePath;



    if ((maxTilesLon + HgtFiledegree) - lonTopLeft > lonTopRight - lonTopLeft) {
        horizontalPosition = 3; //left and right wall at once 
    }
    else {
        horizontalPosition = 0; //left wall
    }

    for (float i = maxTilesLon; i < lonTopRight; i += HgtFiledegree) {

        if (latTopLeft - (maxTilesLat - HgtFiledegree) > latTopLeft - latDownLeft) {
            verticalPosition = 3; //left and right wall at once 
        }
        else {
            verticalPosition = 0; //top wall
        }

        imageOffsetY = 0;

        for (float j = maxTilesLat; j > latDownLeft; j -= HgtFiledegree) {

            CHgtFile::sphericalToFilePath(&filePath, i, j, LOD);

            if (horizontalPosition == 0)
                howManyToReadX = ((i + HgtFiledegree) - lonTopLeft) / readDegree + 1;
            else if (horizontalPosition == 1)
                howManyToReadX = HgtFiledegree / readDegree;
            else if (horizontalPosition == 2)
                howManyToReadX = (lonTopRight - i) / readDegree + 1;
            else if (horizontalPosition == 3)
                howManyToReadX = (lonTopRight - lonTopLeft) / readDegree + 1;

            if (verticalPosition == 0)
                howManyToReadY = (latTopLeft - (j - HgtFiledegree)) / readDegree + 1;
            else if (verticalPosition == 1)
                howManyToReadY = HgtFiledegree / readDegree;
            else if (verticalPosition == 2)
                howManyToReadY = (j - latDownLeft) / readDegree + 1;
            else if (verticalPosition == 3)
                howManyToReadY = (latTopLeft - latDownLeft) / readDegree + 1;



            CHgtFile::loadHeightDataToImage(heightMap, imageOffsetX, imageOffsetY, filePath, verticalPosition, horizontalPosition,
                howManyToReadX, howManyToReadY, HgtFileResolution, HgtSkipping);
            

            if (j - 2 * HgtFiledegree > latDownLeft)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;
        }

        if (i + 2 * HgtFiledegree < lonTopRight)
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


void GLayer::checkHowManyPixelsToReadFromRaw_X(int* howManyToReadX, int horizontalPosition, double lonTopLeftHor, double lonTopRightHor, float i) {

    if (horizontalPosition == 0)
        *howManyToReadX = ((i + RAW_FILE_DEGREE) - lonTopLeftHor) / readDegree + 1;
    else if (horizontalPosition == 1)
        *howManyToReadX = RAW_FILE_DEGREE / readDegree;
    else if (horizontalPosition == 2)
        *howManyToReadX = (lonTopRightHor - i) / readDegree + 1;
    else if (horizontalPosition == 3)
        *howManyToReadX = (lonTopRightHor - lonTopLeftHor) / readDegree + 1;

}

void GLayer::checkHowManyPixelsToReadFromRaw_Y(int *howManyToReadY, int verticalPosition, double latTopLeftHor, double latDownLeftHor, float j) {
    
    if (verticalPosition == 0) 
        *howManyToReadY = (latTopLeftHor - (j - RAW_FILE_DEGREE)) / readDegree + 1;
    else if (verticalPosition == 1) 
        *howManyToReadY = RAW_FILE_DEGREE / readDegree;
    else if (verticalPosition == 2) 
        *howManyToReadY = (j - latDownLeftHor) / readDegree + 1;
    else if (verticalPosition == 3) 
        *howManyToReadY = (latTopLeftHor - latDownLeftHor) / readDegree;
     
}

void GLayer::checkRawFileOffset_X(int* filePositionHorizontalOffset, int horizontalPosition, double lonTopLeftHor, float maxTilesLon) {

    if (horizontalPosition == 0)
        *filePositionHorizontalOffset = (lonTopLeftHor - maxTilesLon) / readDegree;
    else if (horizontalPosition == 1)
        *filePositionHorizontalOffset = 0;
    else if (horizontalPosition == 2)
        *filePositionHorizontalOffset = 0;
    else if (horizontalPosition == 3)
        *filePositionHorizontalOffset = (lonTopLeftHor - maxTilesLon) / readDegree;
   
}

void GLayer::checkRawFileOffset_Y(int* filePositionVerticalOffset, int verticalPosition, double latTopLeftHor, float maxTilesLat) {
    
    if (verticalPosition == 0) 
        *filePositionVerticalOffset = (maxTilesLat - latTopLeftHor) / readDegree;
    else if (verticalPosition == 1) 
        *filePositionVerticalOffset = 0;   
    else if (verticalPosition == 2)       
        *filePositionVerticalOffset = 0;  
    else if (verticalPosition == 3)    
        *filePositionVerticalOffset = (maxTilesLat - latTopLeftHor) / readDegree;
    
}

void GLayer::verticalRawTextureReading(int lonDifference, int latDifference, double lonTopLeft, double lonTopRight, double latTopLeft, double latDownLeft) {
    
    float maxTilesLon, maxTilesLat;
    int movementCaseX, movementCaseY;
    int orientationPointX, orientationPointY;

    if (lonDifference > 0)
        movementCaseX = 1;
    else if (lonDifference == 0)
        movementCaseX = 0;
    else if (lonDifference < 0)
        movementCaseX = -1;

    if (latDifference > 0)
        movementCaseY = 1;
    else if (latDifference == 0)
        movementCaseY = 0;
    else if (latDifference < 0)
        movementCaseY = -1;

    double lonTopLeftHor = lonTopLeft + lonDifference * readDegree;
    double lonTopRightHor = lonTopRight + lonDifference * readDegree;


    //finding latitude of corners
    double latTopLeftHor;
    double latDownLeftHor;
    if (movementCaseY == 1) {
        latTopLeftHor = latTopLeft;
        latDownLeftHor = oldLatTopLeft;
    }
    else if (movementCaseY == -1) {
        latTopLeftHor = oldLatDownLeft;
        latDownLeftHor = latDownLeft;
    }



    //finding longitude of corners         !!!!!!!!DO OGARNIECIA!!!!!!!
    //if (movementCaseY == 1) {
    /*     latTopLeftHor = latTopLeft + latDifference * degree;
        latDownLeftHor = latTopLeft;
    }
    else if (movementCaseY == -1) {
        latTopLeftHor = latDownLeft;
        latDownLeftHor = latDownLeft + latDifference * degree;
    }*/


    //Finding top left tile 
    if (lonTopLeftHor < 0) {
        if (fmod(lonTopLeftHor, RAW_FILE_DEGREE) != 0)
            maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, RAW_FILE_DEGREE) - RAW_FILE_DEGREE;
        else
            maxTilesLon = lonTopLeftHor;
    }
    else {
        maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, RAW_FILE_DEGREE);
    }

    if (latTopLeftHor < 0) {
        maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, RAW_FILE_DEGREE);
    }
    else {
        if (fmod(latTopLeftHor * 2, RAW_FILE_DEGREE) != 0)
            maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, RAW_FILE_DEGREE) + RAW_FILE_DEGREE;
        else
            maxTilesLat = latTopLeftHor;
    }

    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int filePositionHorizontalOffset;
    int filePositionVerticalOffset;

    int imageOffsetX = 0;
    int imageOffsetY = 0;
    int howManyToReadX;
    int howManyToReadY;

    if (movementCaseY == 1) {
        textureBegginingY = textureBegginingY + latDifference;

        if (textureBegginingY > n - 1)
            textureBegginingY = fmod(textureBegginingY, n - 1);
    }

    QString filePath;

    if ((maxTilesLon + RAW_FILE_DEGREE) - lonTopLeftHor > lonTopRightHor - lonTopLeftHor)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall


    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonTopRightHor; i += RAW_FILE_DEGREE) {



        int readingCheck = abs(latDifference);

        if (latTopLeftHor - (maxTilesLat - RAW_FILE_DEGREE) > latTopLeftHor - latDownLeftHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPixelsToReadFromRaw_X(&howManyToReadX, horizontalPosition, lonTopLeftHor, lonTopRightHor, i);
        checkRawFileOffset_X(&filePositionHorizontalOffset, horizontalPosition, lonTopLeftHor, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownLeftHor; j -= RAW_FILE_DEGREE) {


            checkHowManyPixelsToReadFromRaw_Y(&howManyToReadY, verticalPosition, latTopLeftHor, latDownLeftHor, j);
            checkRawFileOffset_Y(&filePositionVerticalOffset, verticalPosition, latTopLeftHor, maxTilesLat);


            if (readingCheck - howManyToReadY < 0)
                howManyToReadY = readingCheck;


            readingCheck -= howManyToReadY;

            if (howManyToReadY > 0) {

                CRawFile::sphericalToFilePath(&filePath, i, j, LOD);

                CRawFile::loadPixelDataToImage2(pixelMap, imageOffsetX, imageOffsetY,
                    filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                    filePositionHorizontalOffset, filePositionVerticalOffset,
                    textureBegginingX, n - 1 - textureBegginingY);
            }


            if (j - 2 * RAW_FILE_DEGREE > latDownLeft)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY -= howManyToReadY;


        }

        if (i + 2 * RAW_FILE_DEGREE < lonTopRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;

    }

    if (movementCaseY == -1) {
        textureBegginingY = textureBegginingY + latDifference;

        if (textureBegginingY < 1)
            textureBegginingY = n - 1 + textureBegginingY;
    }

    oldLatTopLeft = oldLatTopLeft + latDifference * readDegree;
    oldLatDownLeft = oldLatDownLeft + latDifference * readDegree;
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
