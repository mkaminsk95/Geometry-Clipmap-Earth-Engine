#include "GHeight.h"
#include "GLevel.h"
#include "GClipmap.h"
#include "CHgtFile.h"

GHeight::GHeight(GLevel* levelPointer, int inlevelIndex, double inReadDegree, int inHgtSkipping, int inHgtFileResolution, double inHgtFileDegree) : level(levelPointer) {
		
	clipmap = levelPointer->clipmap;
	program = levelPointer->program;

    levelIndex = inlevelIndex;

    readDegree = inReadDegree;
    hgtSkipping = inHgtSkipping;
    hgtFileResolution = inHgtFileResolution;
    hgtFileDegree = inHgtFileDegree;

    n = level->n;
    heightMap = new QImage(n, n, QImage::Format_RGB888);
}


void GHeight::fullHgtTextureReading(double lonLeft, double lonRight, double latDown, double latTop) {

    double maxTilesLon, maxTilesLat;

    int imageOffsetX = 0, imageOffsetY = 0;
    int horizontalPosition; //0 means top wall, 1 middle, 2 bottom wall
    int verticalPosition;   //0 means left wall, 1 middle, 2 right wall
    int howManyToReadX, howManyToReadY;
    int fileHorizontalOffset, fileVerticalOffset;

    QString filePath;

    //Finding top left tile 
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeft, latTop, hgtFileDegree);

    if ((maxTilesLon + hgtFileDegree) - lonLeft > lonRight - lonLeft)
        horizontalPosition = 3; //left and right wall at once 
    else
        horizontalPosition = 0; //left wall
     

    int checkHowManyToReadX = 0;

    for (float i = maxTilesLon; i < lonRight; i += hgtFileDegree) {

        if (latTop - (maxTilesLat - hgtFileDegree) > latTop - latDown)
            verticalPosition = 3; //left and right wall at once 
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;

        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeft, lonRight, i, hgtFileDegree);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeft, maxTilesLon);


        for (float j = maxTilesLat; j > latDown; j -= hgtFileDegree) {

            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTop, latDown, j, hgtFileDegree);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTop, maxTilesLat);

        
            CHgtFile::sphericalToHeightFilePath(&filePath, i, j, levelIndex);
            CHgtFile::loadHeightDataToImageFull(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY,
                hgtFileResolution, hgtSkipping,
                fileHorizontalOffset, fileVerticalOffset);


            if (j - 2 * hgtFileDegree > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;
        }

        if (i + 2 * hgtFileDegree < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;
    }

   
}

void GHeight::horizontalBlockHgtTextureReading(int lonDifference, int latDifference,
    double lonLeft, double lonRight, double latDown, double latTop, 
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftHor, latTopHor, hgtFileDegree);


    if ((maxTilesLon + hgtFileDegree) - lonLeftHor > lonRightHor - lonLeftHor)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall


    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightHor; i += hgtFileDegree) {



        if (latTopHor - (maxTilesLat - hgtFileDegree) > latTopHor - latDownHor)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeftHor, lonRightHor, i, hgtFileDegree);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftHor, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownHor; j -= hgtFileDegree) {


            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTopHor, latDownHor, j, hgtFileDegree);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopHor, maxTilesLat);


            CHgtFile::sphericalToHeightFilePath(&filePath, i, j, levelIndex);
            CHgtFile::loadHeightDataToImagePart(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, hgtFileResolution, hgtSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegHor.x, n - 1 - texBegHor.y);


            if (j - 2 * hgtFileDegree > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;

        }

        if (i + 2 * hgtFileDegree < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;

    }

    
}

void GHeight::verticalBlockHgtTextureReading(int lonDifference, int latDifference,
    double lonLeft, double lonRight, double latDown, double latTop, 
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
    findingTopLeftFileToRead(&maxTilesLon, &maxTilesLat, lonLeftVer, latTopVer, hgtFileDegree);


    if ((maxTilesLon + hgtFileDegree) - lonLeftVer > lonRightVer - lonLeftVer)
        horizontalPosition = 3; //left and right wall at once   
    else
        horizontalPosition = 0; //left wall

    ////////////////////////X///////////////////////////
    for (float i = maxTilesLon; i < lonRightVer; i += hgtFileDegree) {


        if (latTopVer - (maxTilesLat - hgtFileDegree) > latTopVer - latDownVer)
            verticalPosition = 3; //left and right wall at once   
        else
            verticalPosition = 0; //top wall

        imageOffsetY = 0;


        ///////////horizontal
        checkHowManyPointsToRead_X(&howManyToReadX, horizontalPosition, lonLeftVer, lonRightVer, i, hgtFileDegree);
        checkFileOffset_X(&fileHorizontalOffset, horizontalPosition, lonLeftVer, maxTilesLon);


        //////////////////////////Y///////////////////////////
        for (float j = maxTilesLat; j > latDownVer; j -= hgtFileDegree) {


            checkHowManyPointsToRead_Y(&howManyToReadY, verticalPosition, latTopVer, latDownVer, j, hgtFileDegree);
            checkFileOffset_Y(&fileVerticalOffset, verticalPosition, latTopVer, maxTilesLat);


            CHgtFile::sphericalToHeightFilePath(&filePath, i, j, levelIndex);
            
            CHgtFile::loadHeightDataToImagePart(heightMap, imageOffsetX, imageOffsetY,
                filePath, howManyToReadX, howManyToReadY, hgtFileResolution, hgtSkipping,
                fileHorizontalOffset, fileVerticalOffset,
                texBegVer.x, n - 1 - texBegVer.y);


            if (j - 2 * hgtFileDegree > latDown)
                verticalPosition = 1;   //middle 
            else
                verticalPosition = 2;   //down wall

            imageOffsetY += howManyToReadY;

        }

        if (i + 2 * hgtFileDegree < lonRight)
            horizontalPosition = 1;   //middle 
        else
            horizontalPosition = 2;   //right wall

        imageOffsetX += howManyToReadX;

    }

    
}

void GHeight::findingTopLeftFileToRead(double* maxTilesLon, double* maxTilesLat, double lonLeft, double latTop, double degree) {
    
    if (levelIndex <= 8) {

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
    else {
    
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
            *maxTilesLat = latTop - fmod(latTop, degree) + 30;
        }
        else {
            if (fmod(latTop * 2, degree) != 0)
                *maxTilesLat = latTop - fmod(latTop, degree) + degree + 30;
            else
                *maxTilesLat = latTop + 30;
        }


    }
}

void GHeight::checkHowManyPointsToRead_X(int* howManyToReadX, int horizontalPosition, double lonLeft, double lonRight, float i, float fileDegree) {
    float tmp;

    if (horizontalPosition == 0)
        tmp = ((i + fileDegree) - lonLeft) / readDegree;
    else if (horizontalPosition == 1)
        tmp = fileDegree / readDegree;
    else if (horizontalPosition == 2)
        tmp = (lonRight - i) / readDegree;
    else if (horizontalPosition == 3)
        tmp = (lonRight - lonLeft) / readDegree; //+1;

    *howManyToReadX = tmp + 1;
  
}

void GHeight::checkHowManyPointsToRead_Y(int* howManyToReadY, int verticalPosition, double latTop, double latDown, float j, float fileDegree) {
    float tmp;

    if (verticalPosition == 0)
        tmp = (latTop - (j - fileDegree)) / readDegree;
    else if (verticalPosition == 1)
        tmp = fileDegree / readDegree;
    else if (verticalPosition == 2)
        tmp = (j - latDown) / readDegree;
    else if (verticalPosition == 3)
        tmp = (latTop - latDown) / readDegree;

    *howManyToReadY = tmp + 1;
   
}

void GHeight::checkFileOffset_X(int* fileHorizontalOffset, int horizontalPosition, double lonLeft, float maxTilesLon) {

    if (horizontalPosition == 0)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;
    else if (horizontalPosition == 1)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 2)
        *fileHorizontalOffset = 0;
    else if (horizontalPosition == 3)
        *fileHorizontalOffset = (lonLeft - maxTilesLon) / readDegree;

}

void GHeight::checkFileOffset_Y(int* fileVerticalOffset, int verticalPosition, double latTop, float maxTilesLat) {

    if (verticalPosition == 0)
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;
    else if (verticalPosition == 1)
        *fileVerticalOffset = 0;
    else if (verticalPosition == 2)
        *fileVerticalOffset = 0;
    else if (verticalPosition == 3)
        *fileVerticalOffset = (maxTilesLat - latTop) / readDegree;

}
