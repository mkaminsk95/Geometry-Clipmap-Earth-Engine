#include "GPixel.h"
#include "GLevel.h"
#include "GClipmap.h"
#include "CRawFile.h"

#include <qopenglshaderprogram.h>
#include <qstring.h>

double RAW_FILE_DEGREE = 45;

GPixel::GPixel(GLevel* levelPointer, int inlevelIndex, double inReadDegree, int inRawSkipping, int inRawFileResolution) : level(levelPointer) {

    clipmap = levelPointer->clipmap;
    program = levelPointer->program;

    levelIndex = inlevelIndex;
 
    rawSkipping = inRawSkipping;
    rawFileResolution = inRawFileResolution;
    pointDegree = inReadDegree;

    n = level->n;

    if (levelIndex > 1)
        readDegree = inReadDegree / 4;
    else if (levelIndex == 1)
        readDegree = inReadDegree / 2;
    else if (levelIndex == 0)
        readDegree = inReadDegree;
       
    
}

void GPixel::initializeImage() {


    if (levelIndex > 1)
        pixelMap = new QImage((n - 1) * 4, (n - 1) * 4, QImage::Format_RGB888);
    else if (levelIndex == 1)
        pixelMap = new QImage((n - 1) * 2, (n - 1) * 2, QImage::Format_RGB888);
    else if (levelIndex == 0)
        pixelMap = new QImage((n - 1), (n - 1), QImage::Format_RGB888);

    imageReleased = false;
}

void GPixel::releaseImage() {
    delete pixelMap;

    imageReleased = true;
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
            howManyToReadY = howManyToReadY;
            
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTop, maxTilesLat);

            CRawFile::sphericalToRawFilePath(&filePath, i, j, levelIndex);
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
  

    /*pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(levelIndex, QOpenGLTexture::DontResetTextureUnit);
   
    program->setUniformValue(clipmap->pixelTextureLocation[levelIndex], levelIndex);*/
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
    double lonLeftHor = oldLonLeft + lonDifference * pointDegree;
    double lonRightHor = oldLonRight + lonDifference * pointDegree;

    //finding latitude of corners
    double latTopHor;
    double latDownHor;
    if (latDifference > 0) {
        latTopHor = oldLatTop + latDifference * pointDegree;
        latDownHor = oldLatTop;
    }
    else if (latDifference < 0) {
        latTopHor = oldLatDown;
        latDownHor = oldLatDown + latDifference * pointDegree;
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
            lonRightHor = oldLonRight + lonDifference * pointDegree;
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


            CRawFile::sphericalToRawFilePath(&filePath, i, j, levelIndex);

            CRawFile::loadPixelDataToImagePart(pixelMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegHor.x, n - 1 - texBegHor.y, levelIndex);


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

    /*pixelTexture = new QOpenGLTexture(*pixelMap, QOpenGLTexture::DontGenerateMipMaps);
    pixelTexture->bind(levelIndex, QOpenGLTexture::DontResetTextureUnit);

    program->setUniformValue(clipmap->pixelTextureLocation[levelIndex], levelIndex);*/
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
        lonLeftVer = oldLonLeft + lonDifference * pointDegree;
        lonRightVer = oldLonLeft;
    }
    else if (lonDifference > 0) {
        lonLeftVer = oldLonRight;
        lonRightVer = oldLonRight + lonDifference * pointDegree;
    }

    //finding latitude of corners
    double latTopVer, latDownVer;
    if (latDifference > 0) {
        latTopVer = oldLatTop;
        latDownVer = oldLatDown + latDifference * pointDegree;
    }
    else if (latDifference == 0) {
        latTopVer = oldLatTop;
        latDownVer = oldLatDown;
    }
    else if (latDifference < 0) {
        latTopVer = oldLatTop + latDifference * pointDegree;
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
            lonRightVer = oldLonRight + lonDifference * pointDegree;
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


            CRawFile::sphericalToRawFilePath(&filePath, i, j, levelIndex);

            CRawFile::loadPixelDataToImagePart(pixelMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, rawFileResolution, rawSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegVer.x, n - 1 - texBegVer.y, levelIndex);


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



