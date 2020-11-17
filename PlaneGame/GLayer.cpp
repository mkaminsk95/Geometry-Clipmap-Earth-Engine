
#include "GLayer.h"
#include "GClipmap.h"
#include "GPixel.h"
#include "GHeight.h"
#include "CHgtFile.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <math.h>


GLayer::GLayer(GClipmap* clipmapPointer, float inDegree, float inHgtFileDegree, float inScale, int inLayerIndex, 
               int inHgtSkipping, int inHgtFileResolution, int inRawSkipping, int inRawFileResolution, int inN) : clipmap(clipmapPointer){

    readDegree = inDegree;
    scale = inScale;
    layerIndex = inLayerIndex;
    n = inN;

    firstGothrough = true;
    

    hgtTextureBegginingX = 0;  //x
    hgtTextureBegginingY = n;  //y

    rawTextureBegginingX = 0;  //x
    rawTextureBegginingY = n - 1;  //y

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
    pixelManager = new GPixel(this, inLayerIndex, inDegree, inRawSkipping, inRawFileResolution);
    heightManager = new GHeight(this, inLayerIndex, inDegree, inHgtSkipping, inHgtFileResolution, inHgtFileDegree);

    cumputeOffsets();
}

void GLayer::buildLayer(double tlon, double tlat) {
    

    QVector2D offsets;
    double latDifference, lonDifference;
    
    
    //checking if there was a movement
    lonDifference = (-1) * (oldLon - tlon - fmod(oldLon - tlon, readDegree)) / readDegree;
    latDifference = (-1) * (oldLat - tlat - fmod(oldLat - tlat, readDegree)) / readDegree;

    //adjusting layers in right place in relation to each other     
    allFinerHorizontalSum = 0;
    allFinerVerticalSum = 0;
        
    if (!layerIndex == 0) {

        if (clipmap->layer[layerIndex - 1].positionHorizontal == 0)         //left side
            allFinerHorizontalSum = clipmap->layer[layerIndex - 1].allFinerHorizontalSum / 2;
        else if (clipmap->layer[layerIndex - 1].positionHorizontal == 1)    //right side
            allFinerHorizontalSum = clipmap->layer[layerIndex - 1].allFinerHorizontalSum/2 + 1;

        if (clipmap->layer[layerIndex - 1].positionVertical == 0)           //down side    
            allFinerVerticalSum = clipmap->layer[layerIndex - 1].allFinerVerticalSum / 2;
        else if (clipmap->layer[layerIndex - 1].positionVertical == 1)      //top side  
            allFinerVerticalSum = clipmap->layer[layerIndex - 1].allFinerVerticalSum / 2 + 1;
           
        offsets.setX(tlon - (horizontalOffset+allFinerHorizontalSum) * readDegree);
        offsets.setX(tlon - (horizontalOffset+ allFinerHorizontalSum) * readDegree);
        offsets.setY(tlat - (verticalOffset + allFinerVerticalSum) * readDegree);
        offsets.setY(tlat - (verticalOffset + allFinerVerticalSum) * readDegree);

    }
    else {
        offsets.setX(tlon - horizontalOffset * readDegree);
        offsets.setY(tlat - verticalOffset * readDegree);
    }
    
    
    //setting shader variables
    program->setUniformValue("worldOffset", offsets);                      //placing layer in world coordinates
    program->setUniformValue("levelScaleFactor", QVector2D(scale, scale)); //setting right scale 
    program->setUniformValue("layerIndex", layerIndex);                    //setting right index

    //mapping data
    mapDataIntoTextures(tlon, tlat, lonDifference, latDifference);

    
    program->setUniformValue("rawTexOffset", QVector2D(rawTextureBegginingX, rawTextureBegginingY));
    program->setUniformValue("hgtTexOffset", QVector2D(hgtTextureBegginingX, hgtTextureBegginingY));


    //changing layer coordinates depending on movement
    if (firstGothrough == true || abs(latDifference) > n - 1 || abs(lonDifference) > n - 1) {
        //totally reseting old lon and lat
        oldLat = tlat;
        oldLon = tlon;

        firstGothrough = false;
    }
    else if (lonDifference != 0 || latDifference != 0) {
        //moving old lon and lat by difference
        oldLat = oldLat + latDifference * readDegree;
        oldLon = oldLon + lonDifference * readDegree;
    }


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

        if (clipmap->layer[layerIndex - 1].positionVertical == 0) { //down wall
            
            drawD(m, m);

            if (fillerPositionHorizontal == 0) { //left wall
                drawE(m, m+1);
            }
            else if (fillerPositionHorizontal == 1) { //right wall
                drawE(3*m+1, m+1);
            }

        }
        else if (clipmap->layer[layerIndex - 1].positionVertical == 1) { //top wall

            drawD(m, 3*m+1);
            
            if (fillerPositionHorizontal == 0) { //left wall
                drawE(m, m);
            }
            else if (fillerPositionHorizontal == 1) { //right wall
                drawE(3*m+1, m);
            }
        }

    }

}

void GLayer::mapDataIntoTextures(double tlon, double tlat, int lonDifference, int latDifference) {
    
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


    if (firstGothrough == true || (abs(latDifference) > n - 1 || abs(lonDifference) > n - 1)) {

        computeNewLonAndLat(tlon, tlat, &lonLeft, &lonRight, &latTop, &latDown);
        pixelManager->fullRawTextureReading(lonLeft, lonRight, latDown, latTop);
        heightManager->fullHgtTextureReading(lonLeft, lonRight, latTop, latDown);
           
        //reseting texture beggining coordinate
        hgtTextureBegginingX = 0;
        hgtTextureBegginingX = n;
        rawTextureBegginingX = 0;  //x
        rawTextureBegginingY = n - 1;  //y
            
        oldLatTop = latTop;
        oldLonRight = lonRight;
        oldLonLeft = lonLeft;
        oldLatDown = latDown;


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

        
        oldLatTop = oldLatTop + latDifference * readDegree;
        oldLatDown = oldLatDown + latDifference * readDegree;
        oldLonLeft = oldLonLeft + lonDifference * readDegree;
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

        horizontalOffset = (horOffHigherLayer / 2) + m - 1;
        verticalOffset = (verOffHigherLayer / 2) + m - 1;

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
