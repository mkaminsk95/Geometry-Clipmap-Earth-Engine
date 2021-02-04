
#include "GLevel.h"
#include "GClipmap.h"
#include "GPixel.h"
#include "GHeight.h"
#include "CHgtFile.h"


#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <math.h>
#include <chrono>


GLevel::GLevel(GClipmap* clipmapPointer, float inDegree, float inHgtFileDegree, float inScale, int inlevelIndex, 
               int inHgtSkipping, int inHgtFileResolution, int inRawSkipping, int inRawFileResolution, int inN) : clipmap(clipmapPointer){

    readDegree = inDegree;
    scale = inScale;
    levelIndex = inlevelIndex;
    n = inN;

    firstGothrough = true;
    

    hgttextureOriginX = 0;  //x
    hgttextureOriginY = n;  //y

    rawtextureOriginX = 0;  //x
    rawtextureOriginY = (n - 1);  //y

    allFinerHorizontalSum = 0;
    allFinerVerticalSum = 0;

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
 
    pixelManager = new GPixel(this, inlevelIndex, inDegree, inRawSkipping, inRawFileResolution);
    heightManager = new GHeight(this, inlevelIndex, inDegree, inHgtSkipping, inHgtFileResolution, inHgtFileDegree);

    pixelManager->initializeImage();
    imageReleased = false;

    performance = CPerformance::getInstance();

    //cumputeOffsets();
    
}

void GLevel::buildLevel() {
    
    int m = (n + 1) / 4 - 1;
   
    //setting shader variables
    program->setUniformValue("worldOffset", offsets);                      //placing level in world coordinates   
    program->setUniformValue("levelScaleFactor", QVector2D(scale, scale)); //setting right scale 
    program->setUniformValue("levelIndex", levelIndex);                    //setting right index
  
    program->setUniformValue("rawTexOffset", QVector2D(rawtextureOriginXRdy, rawtextureOriginYRdy));
    program->setUniformValue("hgtTexOffset", QVector2D(hgttextureOriginXRdy, hgttextureOriginYRdy));
    program->setUniformValue("highestLvlOfDetail", clipmap->highestLvlOfDetail);

    //loop for blending parameters
    if (clipmap->highestLvlOfDetail > levelIndex) {
    
        program->setUniformValue("rawTexOffsetFnr", QVector2D(clipmap->level[levelIndex + 1].rawtextureOriginXRdy, clipmap->level[levelIndex + 1].rawtextureOriginYRdy));
        program->setUniformValue("hgtTexOffsetFnr", QVector2D(clipmap->level[levelIndex + 1].hgttextureOriginXRdy, clipmap->level[levelIndex + 1].hgttextureOriginYRdy));
    
        program->setUniformValue("cameraPosition", QVector2D( (horizontalOffset + allFinerHorizontalSum), (verticalOffset + allFinerVerticalSum)));
        
    
        if (clipmap->level[levelIndex ].positionVertical == 0) { 
            if (clipmap->level[levelIndex ].positionHorizontal == 0) 
                program->setUniformValue("coarserInFinerOffset", QVector2D(m, m));
            else if (clipmap->level[levelIndex ].positionHorizontal == 1)
                program->setUniformValue("coarserInFinerOffset", QVector2D(m+1,m));
        }
        else if (clipmap->level[levelIndex].positionVertical == 1) {
            if (clipmap->level[levelIndex ].positionHorizontal == 0) 
                program->setUniformValue("coarserInFinerOffset", QVector2D(m,m+1));
            else if (clipmap->level[levelIndex ].positionHorizontal == 1) 
                program->setUniformValue("coarserInFinerOffset", QVector2D(m+1,m+1));
        }
    }
     

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

    if (clipmap->activeLvlOfDetail == levelIndex) {
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

        if (clipmap->level[levelIndex - 1].positionVertical == 0) { //down wall
            
            drawD(m, 3*m+1);

            if (clipmap->level[levelIndex - 1].positionHorizontal == 0) { //left wall
                drawE(3*m+1, m+1);
            }
            else if (clipmap->level[levelIndex - 1].positionHorizontal == 1) { //right wall
                drawE(m, m+1);
            }

        }
        else if (clipmap->level[levelIndex - 1].positionVertical == 1) { //top wall

            drawD(m, m);
            
            if (clipmap->level[levelIndex - 1].positionHorizontal == 0) { //left wall
                drawE(3*m+1, m);
            }
            else if (clipmap->level[levelIndex - 1].positionHorizontal == 1) { //right wall
                drawE(m, m);
            }
        }

    }

}




void GLevel::refreshTextures(double tlon, double tlat) {
    
    double latDifference, lonDifference;

    //checking if there was a movement
    lonDifference = (-1) * (oldLon - tlon - fmod(oldLon - tlon, readDegree)) / readDegree;
    latDifference = (-1) * (oldLat - tlat - fmod(oldLat - tlat, readDegree)) / readDegree;

    //mapping data
    mapDataIntoImages(tlon, tlat, lonDifference, latDifference);

}

void GLevel::updateOriginPoints() {
    rawtextureOriginXRdy = rawtextureOriginX;
    rawtextureOriginYRdy = rawtextureOriginY;
    hgttextureOriginXRdy = hgttextureOriginX;
    hgttextureOriginYRdy = hgttextureOriginY;
}


void GLevel::updateTextures() {
    
    //bool result = pixelManager->pixelMap->save("image.png");
    //updating texture using new picture 
    pixelTexture = new QOpenGLTexture(*pixelManager->pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(levelIndex, QOpenGLTexture::DontResetTextureUnit);
    heightTexture = new QOpenGLTexture(*heightManager->heightMap, QOpenGLTexture::DontGenerateMipMaps);
    heightTexture->bind(levelIndex+13);


    program->setUniformValue(clipmap->pixelTextureLocation[levelIndex], levelIndex);
    program->setUniformValue(clipmap->heightTextureLocation[levelIndex], levelIndex+13);

}

//preperation to a data mapping
void GLevel::mapDataIntoImages(double tlon, double tlat, int lonDifference, int latDifference) {
    
    double lonLeft, lonRight;
    double latTop, latDown;


    if (firstGothrough == true) {

        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        
        oldLon = tlon;
        oldLat = tlat;
        oldLonLeft = lonLeft;
        oldLonRight = lonRight;
        oldLatDown = latDown;
        oldLatTop = latTop;

    }
   

    if(firstGothrough == true || (abs(latDifference) > n - 1 || abs(lonDifference) > n - 1)) {

        //computing coordinates of corners of new moved clipmap 
        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);

        //updating pictures
        pixelManager->fullRawTextureReading(lonLeft, lonRight, latDown, latTop);
        heightManager->fullHgtTextureReading(lonLeft, lonRight, latDown, latTop);
           
        //reseting texture beggining coordinate
        hgttextureOriginX = 0;
        hgttextureOriginY = n;
        rawtextureOriginX = 0;        //x
        rawtextureOriginY = (n - 1);  //y    

        //updating old coordinates
        oldLat = tlat;
        oldLon = tlon;
        oldLatTop = latTop;
        oldLonRight = lonRight;
        oldLonLeft = lonLeft;
        oldLatDown = latDown;

        firstGothrough = false;    

        performance->trianglesRead += 2 * n * n;
        

    }
    else if (lonDifference != 0 || latDifference != 0) {

        point texBegHor, texBegVer;
        
        //computing coordinates of corners of new moved clipmap 
        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        
        //checking texture begginings for horizontal and vertical reading based on movement case 
        computeTextureOffsets(latDifference, lonDifference, &rTexBegHor, &rTexBegVer, true);
        computeTextureOffsets(latDifference, lonDifference, &hTexBegHor, &hTexBegVer, false);


        if (latDifference != 0) {
            pixelManager->horizontalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latDown, latTop, oldLonLeft, oldLonRight, oldLatDown, oldLatTop, rTexBegHor);
            heightManager->horizontalBlockHgtTextureReading(lonDifference, latDifference, lonLeft, lonRight, latDown, latTop, oldLonLeft, oldLonRight, oldLatDown, oldLatTop, hTexBegHor);
        }
         
        if (lonDifference != 0) {
            pixelManager->verticalBlockRawTextureReading(lonDifference, latDifference, lonLeft, lonRight, latDown, latTop, oldLonLeft, oldLonRight, oldLatDown, oldLatTop, rTexBegVer);
            heightManager->verticalBlockHgtTextureReading(lonDifference, latDifference, lonLeft, lonRight, latDown, latTop, oldLonLeft, oldLonRight, oldLatDown, oldLatTop, hTexBegVer);
        }


        if (latDifference > 0) {
            rawtextureOriginY = (int)(rawtextureOriginY + latDifference) % (n - 1);
            hgttextureOriginY = (int)(hgttextureOriginY + latDifference) % (n);
        }
        if (latDifference < 0) {
            rawtextureOriginY = (int)(rawtextureOriginY + latDifference + n - 1) % (n - 1);
            hgttextureOriginY = (int)(hgttextureOriginY + latDifference + n) % (n);
        }
        if (lonDifference > 0) {
            rawtextureOriginX = (int)(rawtextureOriginX + lonDifference) % (n - 1);
            hgttextureOriginX = (int)(hgttextureOriginX + lonDifference) % (n);
        }
        if (lonDifference < 0) {
            rawtextureOriginX = (int)(rawtextureOriginX + lonDifference + n - 1) % (n - 1);
            hgttextureOriginX = (int)(hgttextureOriginX + lonDifference + n) % (n);
        }

        oldLat = oldLat + latDifference * readDegree;
        oldLon = oldLon + lonDifference * readDegree;
        oldLatTop = oldLatTop + latDifference * readDegree;
        oldLatDown = oldLatDown + latDifference * readDegree;
        oldLonLeft = oldLonLeft + lonDifference * readDegree;
        oldLonRight = oldLonRight + lonDifference * readDegree;
        
        

        performance->trianglesRead += 2 * (abs(lonDifference) * (n - 1 - latDifference) ) + 2 * (abs(latDifference) * (n - 1));
        
    }

}


void GLevel::updatePosition() {
    offsets.setX(levelPositionLon);
    offsets.setY(levelPositionLat);
}

//calculating position of bottom-left corner of a level in reference to camera
void GLevel::computeOffsets() {

    if (levelIndex == 0) {
        horizontalOffset = (n - 1) / 2;
        verticalOffset = (n - 1) / 2;
    }
    else {

        double horOffHigherlevel = clipmap->level[levelIndex - 1].horizontalOffset;
        double verOffHigherlevel = clipmap->level[levelIndex - 1].verticalOffset;

        int m = (n + 1) / 4;

        if (clipmap->level[levelIndex - 1].positionHorizontal == 0)         //left side
            horizontalOffset = (horOffHigherlevel / 2) + m - 1;
        else if (clipmap->level[levelIndex - 1].positionHorizontal == 1)    //right side
            horizontalOffset = (horOffHigherlevel / 2) + m;

        if (clipmap->level[levelIndex - 1].positionVertical == 0)           //down side    
            verticalOffset = (verOffHigherlevel / 2) + m - 1;
        else if (clipmap->level[levelIndex - 1].positionVertical == 1)      //top side  
            verticalOffset = (verOffHigherlevel / 2) + m;

    }
}

//toroidal access scenarios
void GLevel::computeTextureOffsets(int latDifference, int lonDifference, point *texBegHor, point *texBegVer, bool rawReading) {

    if (rawReading) {
        if (latDifference > 0) {

            if (lonDifference > 0) {
                texBegHor->x = (rawtextureOriginX + lonDifference) % (n - 1);
                texBegHor->y = (rawtextureOriginY + latDifference) % (n - 1);
                texBegVer->x = rawtextureOriginX;
                texBegVer->y = rawtextureOriginY;
            }
            else if (lonDifference == 0) {
                texBegHor->x = rawtextureOriginX;
                texBegHor->y = (rawtextureOriginY + latDifference) % (n - 1);
            }
            else if (lonDifference < 0) {
                texBegHor->x = (rawtextureOriginX + lonDifference + n - 1) % (n - 1);
                texBegHor->y = (rawtextureOriginY + latDifference) % (n - 1);
                texBegVer->x = (rawtextureOriginX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = rawtextureOriginY;
            }
        }
        else if (latDifference == 0) {

            if (lonDifference > 0) {
                texBegVer->x = rawtextureOriginX;
                texBegVer->y = rawtextureOriginY;
            }
            else if (lonDifference < 0) {
                texBegVer->x = (rawtextureOriginX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = rawtextureOriginY;
            }
        }
        else if (latDifference < 0) {

            if (lonDifference > 0) {
                texBegHor->x = (rawtextureOriginX + lonDifference) % (n - 1);
                texBegHor->y = rawtextureOriginY;
                texBegVer->x = rawtextureOriginX;
                texBegVer->y = (rawtextureOriginY + latDifference + n - 1) % (n - 1);
            }
            else if (lonDifference == 0) {
                texBegHor->x = rawtextureOriginX;
                texBegHor->y = rawtextureOriginY;
            }
            else if (lonDifference < 0) {
                texBegHor->x = (rawtextureOriginX + lonDifference + n - 1) % (n - 1);
                texBegHor->y = rawtextureOriginY;
                texBegVer->x = (rawtextureOriginX + lonDifference + n - 1) % (n - 1);
                texBegVer->y = (rawtextureOriginY + latDifference + n - 1) % (n - 1);
            }
        }
    }
    else {
        if (latDifference > 0) {

            if (lonDifference > 0) {
                texBegHor->x = (hgttextureOriginX + lonDifference) % (n);
                texBegHor->y = (hgttextureOriginY + latDifference) % (n);
                texBegVer->x = hgttextureOriginX;
                texBegVer->y = hgttextureOriginY;
            }
            else if (lonDifference == 0) {
                texBegHor->x = hgttextureOriginX;
                texBegHor->y = (hgttextureOriginY + latDifference) % (n);
            }
            else if (lonDifference < 0) {
                texBegHor->x = (hgttextureOriginX + lonDifference + n) % (n);
                texBegHor->y = (hgttextureOriginY + latDifference) % (n);
                texBegVer->x = (hgttextureOriginX + lonDifference + n) % (n);
                texBegVer->y = hgttextureOriginY;
            }
        }
        else if (latDifference == 0) {

            if (lonDifference > 0) {
                texBegVer->x = hgttextureOriginX;
                texBegVer->y = hgttextureOriginY;
            }
            else if (lonDifference < 0) {
                texBegVer->x = (hgttextureOriginX + lonDifference + n) % (n);
                texBegVer->y = hgttextureOriginY;
            }
        }
        else if (latDifference < 0) {

            if (lonDifference > 0) {
                texBegHor->x = (hgttextureOriginX + lonDifference) % (n);
                texBegHor->y = hgttextureOriginY;
                texBegVer->x = hgttextureOriginX;
                texBegVer->y = (hgttextureOriginY + latDifference + n) % (n);
            }
            else if (lonDifference == 0) {
                texBegHor->x = hgttextureOriginX;
                texBegHor->y = hgttextureOriginY;
            }
            else if (lonDifference < 0) {
                texBegHor->x = (hgttextureOriginX + lonDifference + n) % (n);
                texBegHor->y = hgttextureOriginY;
                texBegVer->x = (hgttextureOriginX + lonDifference + n) % (n);
                texBegVer->y = (hgttextureOriginY + latDifference + n) % (n);
            }
        }
    }
}

//determining position of moved level
void GLevel::computeNewLonAndLat(double tlon, double tlat, double *lonLeft, double *lonRight, double *latTop, double *latDown) {

    *lonLeft = (tlon - (horizontalOffset) * readDegree); //Left Right
    *lonRight = *lonLeft + (n - 1) * readDegree;
    *latDown = (tlat - (verticalOffset) * readDegree);  //Top Down
    *latTop = *latDown + (n - 1) * readDegree;

   
}

void GLevel::refreshPositions(double tlon, double tlat) {
    levelPositionLon = (tlon - (horizontalOffset)*readDegree);
    levelPositionLat = (tlat - (verticalOffset)*readDegree);
}



void GLevel::drawA(float originX, float originY) {
    
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoA->bind();
    indexA_Buffer->bind();
    
    glDrawElements(*drawMode, clipmap->howManyToRenderA, GL_UNSIGNED_INT, 0);
    
    indexA_Buffer->release();
    vaoA->release();
}

void GLevel::drawB(float originX, float originY) {

    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
   
    vaoB->bind();
    indexB_Buffer->bind();
    
    glDrawElements(*drawMode, clipmap->howManyToRenderB, GL_UNSIGNED_INT, 0);
    
    indexB_Buffer->release();
    vaoB->release();
}

void GLevel::drawC(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoC->bind();
    indexC_Buffer->bind();

    glDrawElements(*drawMode, clipmap->howManyToRenderC, GL_UNSIGNED_INT, 0);

    indexC_Buffer->release();
    vaoC->release();
}

void GLevel::drawD(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));
    
    vaoD->bind();
    indexD_Buffer->bind();

    glDrawElements(*drawMode, clipmap->howManyToRenderD, GL_UNSIGNED_INT, 0);

    indexD_Buffer->release();
    vaoD->release();
}
 
void GLevel::drawE(float originX, float originY) {
    program->setUniformValue("patchOrigin", QVector2D(originX, originY));

    vaoE->bind();
    indexE_Buffer->bind();

    glDrawElements(*drawMode, clipmap->howManyToRenderE, GL_UNSIGNED_INT, 0);

    indexE_Buffer->release();
    vaoE->release();
}

void GLevel::drawF(float originX, float originY) {

    program->setUniformValue("patchOrigin", QVector2D(originX, originY));

    vaoB->bind();
    indexB_Buffer->bind();

    glDrawElements(*drawMode, 16, GL_UNSIGNED_INT, 0);

    indexB_Buffer->release();
    vaoB->release();
}

void GLevel::releaseImage() {

    if(imageReleased == false)
        pixelManager->releaseImage();
    
    imageReleased = true;
}

void GLevel::initializeImage() {

    if(imageReleased == true)
        pixelManager->initializeImage();
    
    imageReleased = false;
}