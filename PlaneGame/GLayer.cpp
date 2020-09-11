
#include "GLayer.h"
#include "GClipmap.h"
#include "CHgtFile.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <math.h>



GLayer::GLayer(GClipmap* clipmapPointer, float inDegree, float inHgtFileDegree, float inScale, int inLOD, int inLayerIndex, 
               int inHgtSkipping, int inHgtFileResolution, int inRawSkipping, int inRawFileResolution, int inN) : clipmap(clipmapPointer){

    degree = inDegree;
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

void GLayer::drawLayer() {

}

void GLayer::mapPixelDataIntoTexture(double tlon, double tlat) {
    
    double latDifference, lonDifference;

    //int n = 14;  //should be (2^n)-1 e.g. 3,7,15,31
    double readResolution = degree;

    double rawFileDegree = 45;

    double tlonConst;
    if (firstGothrough == true)
        tlonConst = tlon;

    double lonTopLeft = tlonConst - horizontalOffset * readResolution; //Left Right
    double lonTopRight = lonTopLeft + (n - 1) * readResolution;

    double latTopLeft = tlat + verticalOffset * readResolution;   //Top Down
    double latDownLeft = latTopLeft - (n - 1) * readResolution;

    float maxTilesLon, maxTilesLat;

    if (firstGothrough == true) {
        oldLonTopLeft = lonTopLeft;

        oldLatTopLeft = latTopLeft;
        oldLatDownLeft = latDownLeft;
    }

    lonDifference = (-1)*(oldLonTopLeft - lonTopLeft - fmod(oldLonTopLeft - lonTopLeft, degree))/degree;
    latDifference = (-1)*(oldLatTopLeft - latTopLeft - fmod(oldLatTopLeft - latTopLeft, degree))/degree;


    if (lonDifference != 0 || latDifference != 0) {

        if (latDifference != 0) {
            
            int movementCaseX;
            int movementCaseY;
            int orientationPointX;
            int orientationPointY;


            //latDifference > 1 || latDifference < -1

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

            double lonTopLeftHor = lonTopLeft + lonDifference * degree;
            double lonTopRightHor = lonTopRight + lonDifference * degree;


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
                if (fmod(lonTopLeftHor, rawFileDegree) != 0)
                    maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, rawFileDegree) - rawFileDegree;
                else
                    maxTilesLon = lonTopLeftHor;
            }
            else {
                maxTilesLon = lonTopLeftHor - fmod(lonTopLeftHor, rawFileDegree);
            }

            if (latTopLeftHor < 0) {
                maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, rawFileDegree);
            }
            else {
                if (fmod(latTopLeftHor * 2, rawFileDegree) != 0)
                    maxTilesLat = latTopLeftHor - fmod(latTopLeftHor, rawFileDegree) + rawFileDegree;
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


            //Let the fun begin
            QString filePath;

            if ((maxTilesLon + rawFileDegree) - lonTopLeftHor > lonTopRightHor - lonTopLeftHor)
                horizontalPosition = 3; //left and right wall at once   
            else
                horizontalPosition = 0; //left wall

            

            ////////////////////////X///////////////////////////
            for (float i = maxTilesLon; i < lonTopRightHor; i += rawFileDegree) {

                

                int readingCheck = abs(latDifference);

                if (latTopLeftHor - (maxTilesLat - rawFileDegree) > latTopLeftHor - latDownLeftHor)
                    verticalPosition = 3; //left and right wall at once   
                else
                    verticalPosition = 0; //top wall

                imageOffsetY = 0;


                ///////////horizontal
                if (horizontalPosition == 0) {
                    howManyToReadX = ((i + rawFileDegree) - lonTopLeftHor) / readResolution + 1;
                    filePositionHorizontalOffset = (lonTopLeftHor - maxTilesLon) / readResolution;
                }
                else if (horizontalPosition == 1) {
                    howManyToReadX = rawFileDegree / readResolution;
                    filePositionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 2) {
                    howManyToReadX = (lonTopRightHor - i) / readResolution + 1;
                    filePositionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 3) {
                    howManyToReadX = (lonTopRightHor - lonTopLeftHor) / readResolution + 1;
                    filePositionHorizontalOffset = (lonTopLeftHor - maxTilesLon) / readResolution;
                }


                //////////////////////////Y///////////////////////////
                for (float j = maxTilesLat; j > latDownLeftHor; j -= rawFileDegree) {

                    

                    ///////////vertical
                    if (verticalPosition == 0) {
                        double tmp = (latTopLeftHor - (j - rawFileDegree));
                        howManyToReadY = (latTopLeftHor - (j - rawFileDegree)) / readResolution + 1;
                        filePositionVerticalOffset = (maxTilesLat - latTopLeftHor) / readResolution;
                    }
                    else if (verticalPosition == 1) {
                        howManyToReadY = rawFileDegree / readResolution;
                        filePositionVerticalOffset = 0;
                    }
                    else if (verticalPosition == 2) {
                        double tmp = (j - latDownLeftHor);
                         howManyToReadY = (j - latDownLeftHor) / readResolution + 1;
                        filePositionVerticalOffset = 0;
                    }
                    else if (verticalPosition == 3) {
                        howManyToReadY = (latTopLeftHor - latDownLeftHor) / readResolution;
                        filePositionVerticalOffset = (maxTilesLat - latTopLeftHor) / readResolution;
                    }


                    if (readingCheck - howManyToReadY < 0) {
                        howManyToReadY = readingCheck;
                    }

                    readingCheck -= howManyToReadY;

                    if (howManyToReadY > 0) {

                    CRawFile::sphericalToFilePath(&filePath, i, j, LOD);

                    CRawFile::loadPixelDataToImage2(pixelMap, imageOffsetX, imageOffsetY,
                        filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                        filePositionHorizontalOffset, filePositionVerticalOffset,
                        textureBegginingX, n - 1 - textureBegginingY);
                    }


                    if (j - 2 * rawFileDegree > latDownLeft)
                        verticalPosition = 1;   //middle 
                    else
                        verticalPosition = 2;   //down wall

                    imageOffsetY -= howManyToReadY;

                    
                }

                if (i + 2 * rawFileDegree < lonTopRight)
                    horizontalPosition = 1;   //middle 
                else
                    horizontalPosition = 2;   //right wall

                imageOffsetX += howManyToReadX;

            }

            //textureBegginingX += lonDifference;
            if (movementCaseY == -1) {
                textureBegginingY = textureBegginingY + latDifference;

                if (textureBegginingY < 1)
                    textureBegginingY = n - 1 + textureBegginingY;
            }

            


            /*if (textureBegginingX > n)
                textureBegginingX = fmod(textureBegginingX, n);
            else if (textureBegginingX < 0)
                textureBegginingX = n + textureBegginingX;*/

            //oldLatTopLeft = latTopLeft;
            //oldLatDownLeft = latDownLeft;
            oldLatTopLeft  = oldLatTopLeft  + latDifference * readResolution;
            oldLatDownLeft = oldLatDownLeft + latDifference * readResolution;
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

        
        //Finding top left tile 
        if (lonTopLeft < 0) {
            if (fmod(lonTopLeft, rawFileDegree) != 0)
                maxTilesLon = lonTopLeft - fmod(lonTopLeft, rawFileDegree) - rawFileDegree;
            else
                maxTilesLon = lonTopLeft;
        }
        else {
            maxTilesLon = lonTopLeft - fmod(lonTopLeft, rawFileDegree);
        }

        if (latTopLeft < 0) {
            maxTilesLat = latTopLeft - fmod(latTopLeft, rawFileDegree);
        }
        else {
            if (fmod(latTopLeft * 2, rawFileDegree) != 0)
                maxTilesLat = latTopLeft - fmod(latTopLeft, rawFileDegree) + rawFileDegree;
            else
                maxTilesLat = latTopLeft;
        }

        int imageOffsetX = 0;
        int imageOffsetY = 0;
        int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
        int verticalPosition; //0 means left wall, 1 middle, 2 right wall

        int howManyToReadX;
        int howManyToReadY;
        int positionHorizontalOffset;
        int positionVerticalOffset;

        QString filePath;

        if ((maxTilesLon + rawFileDegree) - lonTopLeft > lonTopRight - lonTopLeft) {
            horizontalPosition = 3; //left and right wall at once 
        }
        else {
            horizontalPosition = 0; //left wall
        }

        for (float i = maxTilesLon; i < lonTopRight; i += rawFileDegree) {

            if (latTopLeft - (maxTilesLat - rawFileDegree) > latTopLeft - latDownLeft) {
                verticalPosition = 3; //left and right wall at once 
            }
            else {
                verticalPosition = 0; //top wall
            }

            imageOffsetY = 0;

                ///////////horizontal
                if (horizontalPosition == 0) {
                    howManyToReadX = ((i + rawFileDegree) - lonTopLeft) / readResolution + 1;
                    positionHorizontalOffset = (lonTopLeft - maxTilesLon) / readResolution;
                }
                else if (horizontalPosition == 1) {
                    howManyToReadX = rawFileDegree / readResolution;
                    positionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 2) {
                    howManyToReadX = (lonTopRight - i) / readResolution + 1;
                    positionHorizontalOffset = 0;
                }
                else if (horizontalPosition == 3) {
                    howManyToReadX = (lonTopRight - lonTopLeft) / readResolution + 1;
                    positionHorizontalOffset = (lonTopLeft - maxTilesLon) / readResolution;
                }
                

            for (float j = maxTilesLat; j > latDownLeft; j -= rawFileDegree) {

                CRawFile::sphericalToFilePath(&filePath, i, j, LOD);
                
                ///////////vertical
                if (verticalPosition == 0) {
                    howManyToReadY = (latTopLeft - (j - rawFileDegree)) / readResolution + 1;
                    positionVerticalOffset = (maxTilesLat - latTopLeft) / readResolution;
                }
                else if (verticalPosition == 1) {
                    howManyToReadY = rawFileDegree / readResolution;
                    positionVerticalOffset = 0;
                }
                else if (verticalPosition == 2) {
                    howManyToReadY = (j - latDownLeft) / readResolution + 1;
                    positionVerticalOffset = 0;
                }
                else if (verticalPosition == 3) {
                    howManyToReadY = (latTopLeft - latDownLeft) / readResolution + 1;
                    positionVerticalOffset = (maxTilesLat - latTopLeft) / readResolution;
                }


                CRawFile::loadPixelDataToImage(pixelMap, imageOffsetX, imageOffsetY, 
                                               filePath, howManyToReadX, howManyToReadY,
                                               rawFileResolution, rawSkipping, 
                                               positionHorizontalOffset, positionVerticalOffset);



                if (j - 2 * rawFileDegree > latDownLeft)
                    verticalPosition = 1;   //middle 
                else
                    verticalPosition = 2;   //down wall

                imageOffsetY += howManyToReadY;
            }

            if (i + 2 * rawFileDegree < lonTopRight)
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
    float readResolution = degree;

    double lonTopLeft = tlon - horizontalOffset * readResolution;    //Left Right
    double lonTopRight = lonTopLeft + (n - 1) * readResolution;

    double latTopLeft = tlat + verticalOffset * readResolution;   //Top Down
    double latDownLeft = latTopLeft - (n - 1) * readResolution;

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
                howManyToReadX = ((i + HgtFiledegree) - lonTopLeft) / readResolution + 1;
            else if (horizontalPosition == 1)
                howManyToReadX = HgtFiledegree / readResolution;
            else if (horizontalPosition == 2)
                howManyToReadX = (lonTopRight - i) / readResolution + 1;
            else if (horizontalPosition == 3)
                howManyToReadX = (lonTopRight - lonTopLeft) / readResolution + 1;

            if (verticalPosition == 0)
                howManyToReadY = (latTopLeft - (j - HgtFiledegree)) / readResolution + 1;
            else if (verticalPosition == 1)
                howManyToReadY = HgtFiledegree / readResolution;
            else if (verticalPosition == 2)
                howManyToReadY = (j - latDownLeft) / readResolution + 1;
            else if (verticalPosition == 3)
                howManyToReadY = (latTopLeft - latDownLeft) / readResolution + 1;



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
            offsets.setX(tlon - horizontalOffset * degree);
        else if (clipmap->layer[layerIndex - 1].positionHorizontal == 1) //right side
            offsets.setX(tlon - (horizontalOffset+1) * degree);
        
        if (clipmap->layer[layerIndex - 1].positionVertical == 0) //down side    
            offsets.setY(tlat - (verticalOffset) * degree);
        else if (clipmap->layer[layerIndex - 1].positionVertical == 1) //top side    
            offsets.setY(tlat - (verticalOffset+1) * degree);
    }
    else {
        offsets.setX(tlon - horizontalOffset * degree);
        offsets.setY(tlat - verticalOffset * degree);
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
