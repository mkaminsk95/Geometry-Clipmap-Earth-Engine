#include "GPixel.h"
#include "GLayer.h"
#include "GClipmap.h"
#include "CRawFile.h"

#include <qopenglshaderprogram.h>
#include <qstring.h>

double RAW_FILE_DEGREE = 45;

GPixel::GPixel(GLayer* layerPointer, int inLayerIndex, double inReadDegree, int inRawSkipping, int inRawFileResolution) : layer(layerPointer) {

    clipmap = layerPointer->clipmap;
    program = layerPointer->program;

    layerIndex = inLayerIndex;
 
    readDegree = inReadDegree;
    rawSkipping = inRawSkipping;
    rawFileResolution = inRawFileResolution;

    n = layer->n;
    pixelMap = new QImage(n - 1, n - 1, QImage::Format_RGB888);
}

void GPixel::fullRawTextureReading(double lonLeft, double lonRight, double latDown, double latTop) {
   
    double maxTilesLon, maxTilesLat;
    
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


    for (double i = maxTilesLon; i < lonRight; i += RAW_FILE_DEGREE) {

        if (latTop - (maxTilesLat - RAW_FILE_DEGREE) > latTop - latDown)
            verticalPosition = 3; //left and right wall at once 
        else
            verticalPosition = 0; //top wall


        imageOffsetY = 0;

        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeft, lonRight, i, RAW_FILE_DEGREE);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeft, maxTilesLon);


        for (double j = maxTilesLat; j > latDown; j -= RAW_FILE_DEGREE) {

            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTop, latDown, j, RAW_FILE_DEGREE);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTop, maxTilesLat);

            CRawFile::sphericalToRawFilePath(&filePath, i, j, layerIndex);
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
    pixelMap->save("mapa.png");

    /*pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);
   
    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);*/
}


void GPixel::horizontalBlockRawTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop,
    double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegHor) {

    double maxTilesLon, maxTilesLat;

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

    bool withOvelflow = false;
     if (maxTilesLon > lonRightHor) {
         withOvelflow = true;
         lonRightHor = 360;
     }
    ////////////////////////X///////////////////////////
    for (double i = maxTilesLon; i < lonRightHor; i += RAW_FILE_DEGREE) {

        if (withOvelflow && i + RAW_FILE_DEGREE == 360) {
            i = 0;
            lonRightHor = oldLonRight + lonDifference * readDegree;
        }


        if (latTopHor - (maxTilesLat - RAW_FILE_DEGREE) > latTopHor - latDownHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeftHor, lonRightHor, i, RAW_FILE_DEGREE);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftHor, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (double j = maxTilesLat; j > latDownHor; j -= RAW_FILE_DEGREE) {


            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTopHor, latDownHor, j, RAW_FILE_DEGREE);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopHor, maxTilesLat);


            CRawFile::sphericalToRawFilePath(&filePath, i, j, layerIndex);

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
    pixelMap->save("mapa.png");


    /*pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);

    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);*/
}


void GPixel::verticalBlockRawTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop,
    double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegVer) {

    double maxTilesLon, maxTilesLat;

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

    bool withOvelflow = false;
    //if (maxTilesLon > lonRightVer) {
    //    withOvelflow = true;
    //    lonRightVer = 360;
    //}
    ////////////////////////X///////////////////////////
    for (double i = maxTilesLon; i < lonRightVer; i += RAW_FILE_DEGREE) {

        if (withOvelflow && i + RAW_FILE_DEGREE == 360) {
            i = 0;
            lonRightVer = oldLonRight + lonDifference * readDegree;
        }

        if (latTopVer - (maxTilesLat - RAW_FILE_DEGREE) > latTopVer - latDownVer)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeftVer, lonRightVer, i, RAW_FILE_DEGREE);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftVer, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (double j = maxTilesLat; j > latDownVer; j -= RAW_FILE_DEGREE) {


            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTopVer, latDownVer, j, RAW_FILE_DEGREE);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopVer, maxTilesLat);


            CRawFile::sphericalToRawFilePath(&filePath, i, j, layerIndex);

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
    pixelMap->save("mapa.png");

    /*pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(layerIndex, QOpenGLTexture::DontResetTextureUnit);

    program->setUniformValue(clipmap->pixelTextureLocation[layerIndex], layerIndex);*/
}

void GPixel::findingTopLeftFileToRead(double* maxTilesLon, double* maxTilesLat, double lonLeft, double latTop, double degree) {

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

void GPixel::checkHowManyPointsToRead_X(int* howManyToReadX, int horizontalPosition, double lonLeft, double lonRight, float i, float fileDegree) {
    float tmp;

    if (horizontalPosition == 0)
        tmp = ((i + fileDegree) - lonLeft) / readDegree;
    else if (horizontalPosition == 1)
        tmp = fileDegree / readDegree;
    else if (horizontalPosition == 2)
        tmp = (lonRight - i) / readDegree;
    else if (horizontalPosition == 3)
        tmp = (lonRight - lonLeft) / readDegree; //+1;

    if (fmod(tmp, 1) > 0.5)
        *howManyToReadX = tmp + 1;
    else
        *howManyToReadX = tmp;
  

}

void GPixel::checkHowManyPointsToRead_Y(int* howManyToReadY, int verticalPosition, double latTop, double latDown, float j, float fileDegree) {
    float tmp;

    if (verticalPosition == 0)
        tmp = (latTop - (j - fileDegree)) / readDegree;
    else if (verticalPosition == 1)
        tmp = fileDegree / readDegree;
    else if (verticalPosition == 2)
        tmp = (j - latDown) / readDegree;
    else if (verticalPosition == 3)
        tmp = (latTop - latDown) / readDegree;

   
    if (fmod(tmp, 1) > 0.5)
        *howManyToReadY = tmp + 1;
    else
        *howManyToReadY = tmp;
 
}

void GPixel::checkFileOffset_X(int* fileHorizontalOffset, int horizontalPosition, double lonLeft, float maxTilesLon) {

    if (horizontalPosition == 0)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;
    else if (horizontalPosition == 1)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 2)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 3)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;

}

void GPixel::checkFileOffset_Y(int* fileVerticalOffset, int verticalPosition, double latTop, float maxTilesLat) {

    if (verticalPosition == 0)
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;
    else if (verticalPosition == 1)
        *fileVerticalOffset = 0;
    else if (verticalPosition == 2)
        *fileVerticalOffset = 0;
    else if (verticalPosition == 3)
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;

}



