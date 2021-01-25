/*
 *   -------------------------------------------------------------------------
 *    HgtReader v1.0
 *                                                    (c) Robert Rypula 156520
 *                                   Wroclaw University of Technology - Poland
 *                                                      http://www.pwr.wroc.pl
 *                                                           2011.01 - 2011.06
 *   -------------------------------------------------------------------------
 *
 *   What is this:
 *     - graphic system based on OpenGL to visualize entire Earth including
 *       terrain topography & satellite images
 *     - part of my thesis "Rendering of complex 3D scenes"
 *
 *   What it use:
 *     - Nokia Qt cross-platform C++ application framework
 *     - OpenGL graphic library
 *     - NASA SRTM terrain elevation data:
 *         oryginal dataset
 *           http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/
 *         corrected part of earth:
 *           http://www.viewfinderpanoramas.org/dem3.html
 *         SRTM v4 highest quality SRTM dataset avaiable:
 *           http://srtm.csi.cgiar.org/
 *     - TrueMarble satellite images
 *         free version from Unearthed Outdoors (250m/pix):
 *           http://www.unearthedoutdoors.net/global_data/true_marble/download
 *     - ALGLIB cross-platform numerical analysis and data processing library
 *       for SRTM dataset bicubic interpolation from 90m to 103m (more
 *       flexible LOD division)
 *         ALGLIB website:
 *           http://www.alglib.net/
 *
 *   Contact to author:
 *            phone    +48 505-363-331
 *            e-mail   robert.rypula@gmail.com
 *            GG       1578139
 *
 *                                                   program under GNU licence
 *   -------------------------------------------------------------------------
 */

#include <fstream>
#include <iostream>
#include <vector>
#include "CHgtFile.h"
#include "CCommons.h"

using namespace std;

CHgtFile::CHgtFile()
{
    sizeX = 0;
    sizeY = 0;
    height = 0;
}

CHgtFile::~CHgtFile()
{
    if (height!=0)
        delete []height;
}

void CHgtFile::init(int sX, int sY)
{
    if (height!=0)
        delete []height;

    sizeX = sX;
    sizeY = sY;

    height = new quint16[sizeX*sizeY];
}

void CHgtFile::exchangeEndian()
{
    unsigned char byte1, byte0;
    int i;

    for (i=0; i<sizeX*sizeY; i++) {
        byte1 = (height[i] & 0xFF00) >> 8;
        byte0 = height[i] & 0xFF;

        height[i] = (byte0 << 8) + byte1;
    }
}

void CHgtFile::savePGM(QString name)
{
    if (height==0) return;
    fstream filePGM;
    int height;
    int nr, x, y;

    // save PGM file to disk
    
    filePGM.open(name.toUtf8(), fstream::out);

    filePGM << "P2" << endl;
    filePGM << sizeX << " " << sizeY << endl << 255;
    nr = 0;
    for (y=0; y<sizeY; y++)
      for (x=0; x<sizeX; x++) {
        if (nr%15==0) filePGM << endl;

        height = getHeight(x, y);
        if (height!=0) {
            height = (int)(((double)height / 3000.0) * 200.0) + 55;
            if (height>255) height = 255;
        }
        filePGM << height << " ";
        nr++;
      }

    filePGM.close();
}

void CHgtFile::saveFile(QString name)
{
    if (height==0) return;
    fstream fileHgt;

    // save HGT file to disk
    fileHgt.open(name.toUtf8(), fstream::out | fstream::binary);
    exchangeEndian();
    fileHgt.write((char *)height, sizeX*sizeY*2);
    fileHgt.close();
}

void CHgtFile::loadFile(QString name, int x, int y)
{
    init(x, y);
    fstream fileHgt;

    // load HGT file to memory
    fileHgt.open(name.toUtf8(), fstream::in | fstream::binary);
    fileHgt.read((char *)height, sizeX*sizeY*2);
    exchangeEndian();
    fileHgt.close();
}

//void CHgtFile::loadFileBlock(QString name, int x, int y, int fileResolution, int skip)
//{
//    fstream fileHgt;
//
//    int XtoRead = 3;
//    int YtoRead = 3;
//    quint16 *buffo2r = new quint16[XtoRead * YtoRead * 2];
//    quint16 buffor[18];
//    int iterator = 0;
//
//    int xyOffset = (x-1) * 2 + fileResolution * 2 * (y - 1); //(3,10)
//
//    // load HGT file to memory
//    fileHgt.open(name.toUtf8(), fstream::in | fstream::binary);
//    bool isOpened = fileHgt.is_open();
//
//    for (int i = 0; i < YtoRead; i++) {
//
//        fileHgt.seekg(xyOffset + 2 * i * fileResolution);
//        
//        int tmp = xyOffset + 2 * i * fileResolution;
//
//        for (int j = 0; j < XtoRead; j++) {
//
//            fileHgt.read((char*)&buffor[iterator], 2);
//            iterator += 1;
//            fileHgt.seekg(skip, fileHgt.cur);
//            quint16 tmp = buffor[iterator];
//            tmp = buffor[iterator + 1];
//        }
//    }
//    //exchangeEndian();
//    fileHgt.close();
//}

void CHgtFile::loadHeightDataToImage(QImage *image, int imageOffsetX, int imageOffsetY, QString name, 
               int verticalPosition, int horizontalPosition, int xToRead, int yToRead, int fileResolution, int skip)
{
    fstream fileHgt;
    quint16 heightPoint[2];
    unsigned char byte1, byte0;

    int offset;
    int fileSize = 2 * fileResolution * fileResolution;

    if (verticalPosition == 0) { //top wall
        offset = fileSize - yToRead * skip * 2 * fileResolution - 2 * fileResolution;
    }
    else {  //middle/down
        offset = 0;
    }

    if (horizontalPosition == 0) { //left wall
        offset = offset + 2 * fileResolution - 2 * xToRead * skip - 2;
    }
    else { //middle/right wall
        //nothing
    }

    // load HGT file to memory
    fileHgt.open(name.toUtf8(), fstream::in | fstream::binary);
    
    bool opened = fileHgt.is_open();

    if (!opened) {

    }

    for (int y = 0; y < yToRead; y++) {

        fileHgt.seekg(offset + 2 * y * fileResolution);
        //fileHgt.seekg(0);
        
        for (int x = 0; x < xToRead; x++) {

            fileHgt.read((char*)heightPoint, 2);
            fileHgt.seekg(skip*2-2, fileHgt.cur);

            byte1 = (heightPoint[0] & 0xFF00) >> 8;
            byte0 = heightPoint[0] & 0xFF;
            
            heightPoint[0] = (byte0 << 8) + byte1;
            int x1 = imageOffsetX + x ;
            int y1 = imageOffsetY + y ;
          
            image->setPixelColor(imageOffsetX+x,imageOffsetY+y, heightPoint[0]);

        }
    }
    //exchangeEndian();
    fileHgt.close();
}

void CHgtFile::getHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = getHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::getHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = (quint16)getHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::getHeightBlockToBuffer(quint16* buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y = 0; Y < sy; Y++)
        for (X = 0; X < sx; X++) {
            buffer[i] = (quint16)getHeight(x + X * skip, y + Y * skip);
            i++;
        }
}

void CHgtFile::setHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setHeight(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CHgtFile::setHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setHeight(x + X*skip, y + Y*skip, (int)buffer[i]);
            i++;
        }
}

void CHgtFile::fileOpen(QString name, int sX, int sY)
{
    sizeX = sX;
    sizeY = sY;
    file.open(name.toUtf8(), fstream::in | fstream::out | fstream::binary);
}

void CHgtFile::fileClose()
{
    file.close();
}

void CHgtFile::fileSetHeight(int x, int y, int hgt)
{
    unsigned char byte[2];

    byte[0] = (hgt & 0xFF00) >> 8;
    byte[1] = hgt & 0xFF;

    file.seekp((y*sizeX + x)*2);
    file.write((char *)byte, 2);
}

int CHgtFile::fileGetHeight(int x, int y)
{
    char byte[2];

    file.seekg((y*sizeX + x)*2);
    file.read(byte, 2);

    return (int)( (((unsigned char)byte[0]) << 8) + ((unsigned char)byte[1]) );
}

void CHgtFile::fileGetHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = fileGetHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::fileGetHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = (quint16)fileGetHeight(x + X*skip, y + Y*skip);
            i++;
        }
}

void CHgtFile::fileSetHeightBlock(int *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetHeight(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CHgtFile::fileSetHeightBlock(quint16 *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetHeight(x + X*skip, y + Y*skip, (int)buffer[i]);
            i++;
        }
}

void CHgtFile::loadHeightDataToImageFull(QImage* image, int imageOffsetX, int imageOffsetY, QString name,
    int xToRead, int yToRead, int fileResolution, int skip, int positionHorizontalOffset, int positionVerticalOffset)
{
    fstream fileHeight;
    quint8 height[2];
    quint8 heightNew[2];
    quint8 heightOld[2];

    uchar* bits;


    int howManyTimes;
    long offset;
    int n = image->height() - 1;
    int rowCheck = 0; //variable tracks next rows in raw file

    //calculation offset for reading exact bites in hgt files
    offset = (positionVerticalOffset * fileResolution + positionHorizontalOffset) * skip * 2;

    // load HGT file to memory
    fileHeight.open(name.toUtf8(), fstream::in | fstream::out | fstream::binary);
    bool ifOpened = fileHeight.is_open();
    
    if (ifOpened) {

        for (int y = imageOffsetY; y < imageOffsetY + yToRead; y++) {

            //finding right input data
            fileHeight.seekg(offset + 2 * skip * rowCheck * fileResolution);

            //finding right row of image (texture)
            bits = image->scanLine(n - y);

            for (int x = imageOffsetX; x < imageOffsetX + xToRead; x++) {

                
                //reading next two bites (2bite format - height 0 - 65536) from hgt file 
                fileHeight.read((char*)&height[0], 1);
                fileHeight.read((char*)&height[1], 1);

                if (height[0] > 40) {
                    CCommons::stringIntoVSConsole("Jeb! ");
                    CCommons::doubleIntoVSConsole(height[0]);

                    fileHeight.seekg(-4, fileHeight.cur);
                    howManyTimes = 1;
                    fileHeight.read((char*)&heightNew[0], 1);
                    fileHeight.read((char*)&heightNew[1], 1);

                    while (heightNew[0] > 40) {

                        fileHeight.seekg(-4, fileHeight.cur);
                        howManyTimes++;
                        fileHeight.read((char*)&heightNew[0], 1);
                        fileHeight.read((char*)&heightNew[1], 1);

                    }

                    fileHeight.seekp(2, fileHeight.cur);
                    fileHeight.seekp(-2, fileHeight.cur);
                    for (int i = 0; i < howManyTimes; i++) {
                        fileHeight.write((char*)&heightNew[0], 1);
                        fileHeight.write((char*)&heightNew[1], 1);
                    }

                    fileHeight.flush();

                    CCommons::stringIntoVSConsole("\n");
                }


                //jumping to the next bites block depending on the layer resolution
                fileHeight.seekg(skip * 2 - 2, fileHeight.cur);

                if (height[0] > 40) {
                    //writing to the image
                    *(bits + 3 * x) =     heightNew[1];
                    *(bits + 3 * x + 1) = heightNew[0];
                }
                else {
                    //writing to the image
                    *(bits + 3 * x) =     height[1];
                    *(bits + 3 * x + 1) = height[0];
                }

                //jumping to the next bites block depending on the layer resolution
                //fileHeight.seekg(skip * 2 - 2, fileHeight.cur);

                //writing to the image
                //*(bits + 3 * x) = height[1];
                //*(bits + 3 * x + 1) = height[0];

            }

           
            rowCheck++;
        }

      
    }
    else {
        for (int y = imageOffsetY; y < imageOffsetY + yToRead; y++) {

            //finding right row of image (texture)
            bits = image->scanLine(n - y);

            for (int x = imageOffsetX; x < imageOffsetX + xToRead; x++) {

                //writing to the image
                *(bits + 3 * x) = 0x00;
                *(bits + 3 * x + 1) = 0x00;

            }

            rowCheck++;
        }
    }



    fileHeight.close();
}

void CHgtFile::loadHeightDataToImagePart(QImage* image, int imageOffsetX, int imageOffsetY, QString name,
    int xToRead, int yToRead, int fileResolution, int skip,
    int filePositionHorizontalOffset, int filePositionVerticalOffset,
    int textureBegginingX, int textureBegginingY)
{
    fstream fileHeight;
    quint8 height[2];
    quint8 heightNew[2];

    int howManyTimes;

    uchar* bits;

    long offset;

    int x, y;
    int yStopCondition, xStopCondition;
    int rowCheck = 0;  //variable tracks next rows in raw file
    int n = image->height();

    y = imageOffsetY;
    x = imageOffsetX;
    yStopCondition = imageOffsetY + yToRead;
    xStopCondition = imageOffsetX + xToRead;

    //calculation offset for reading exact bites in hgt files
    offset = (filePositionVerticalOffset * fileResolution + filePositionHorizontalOffset) * skip * 2;

    //load HGT file to memory
    fileHeight.open(name.toUtf8(), fstream::in | fstream::out | fstream::binary);
    //fileHeight.open("test.hgt");
    bool ifOpened = fileHeight.is_open();

    if (ifOpened) {
        for (y = imageOffsetY; y < yStopCondition; y++) {

            //finding right input data
            fileHeight.seekg(offset + 2 * skip * rowCheck * fileResolution);
    

            //finding right row of image (texture)
            bits = image->scanLine((n - 1) - fmod(textureBegginingY + y, n));

            for (x = imageOffsetX; x < xStopCondition; x++) {


                //reading next three bites (3bite format - rgb) from raw file 
                fileHeight.read((char*)&height[0], 1);
                fileHeight.read((char*)&height[1], 1);

                
                if (height[0] > 40) {
                    CCommons::stringIntoVSConsole("Jeb! ");
                    CCommons::doubleIntoVSConsole(height[0]);
                    
                    fileHeight.seekg(-4, fileHeight.cur);
                    howManyTimes = 1;
                    fileHeight.read((char*)&heightNew[0], 1);
                    fileHeight.read((char*)&heightNew[1], 1);

                    while (heightNew[0] > 40) {

                        fileHeight.seekg(-4, fileHeight.cur);
                        howManyTimes++;
                        fileHeight.read((char*)&heightNew[0], 1);
                        fileHeight.read((char*)&heightNew[1], 1);

                    }

                    fileHeight.seekp(2, fileHeight.cur);
                    fileHeight.seekp(-2, fileHeight.cur);
                    for (int i = 0; i < howManyTimes; i++) {
                        fileHeight.write((char*)&heightNew[0], 1);
                        fileHeight.write((char*)&heightNew[1], 1);
                    }

                    fileHeight.flush();

                    CCommons::stringIntoVSConsole("\n");
                }


                //jumping to the next bites block depending on the layer resolution
                fileHeight.seekg(skip * 2 - 2, fileHeight.cur);

                if (height[0] > 40) {
                    //writing to the image
                    *(bits + 3 * ((textureBegginingX + x) % (n))) = heightNew[1];
                    *(bits + 3 * ((textureBegginingX + x) % (n)) + 1) = heightNew[0];
                }
                else {
                    //writing to the image
                    *(bits + 3 * ((textureBegginingX + x) % (n))) = height[1];
                    *(bits + 3 * ((textureBegginingX + x) % (n)) + 1) = height[0];
                }
                //*(bits + 3 * ((textureBegginingX + x) % (n))) = height[1];
                //*(bits + 3 * ((textureBegginingX + x) % (n)) + 1) = height[0];

            }

            rowCheck++;
        }
    }
    else {
        for (y = imageOffsetY; y < yStopCondition; y++) {

            //finding right row of image (texture)
            bits = image->scanLine((n - 1) - fmod(textureBegginingY + y, n));

            for (x = imageOffsetX; x < xStopCondition; x++) {

                //writing to the image
                *(bits + 3 * ((textureBegginingX + x) % (n))) = 0;
                *(bits + 3 * ((textureBegginingX + x) % (n)) + 1) = 0;

            }

        }
    }



    fileHeight.close();
}

void CHgtFile::sphericalToHeightFilePath(QString* filePath, float lon, float lat, int layerIndex) {

    QString filePathTmp;

    if (layerIndex >= 9) {
        filePathTmp = "E:\\HgtReader_data\\L00-L03";
    }
    else if (layerIndex >= 4) {
        filePathTmp = "E:\\HgtReader_data\\L04-L08";
    }
    else {
        filePathTmp = "E:\\HgtReader_data\\L09-L13";
    }

    if (lon > 0 && lon < 180)  //0-180
        lon = lon;
    else if (lon > 180)  //180-360
        lon = lon - 360;
    else if (lon < 0)     //-180-0
        lon = lon;
    else if (lon == 180)
        lon = -180;


    char latString[6] = "00,00";
    char lonString[7] = "000,00";
    if (abs(lat) < 10) {
        sprintf(&latString[1], "%g", abs(lat));
    }
    else {
        sprintf(latString, "%g", abs(lat));
    }

    if (latString[3] == '5')
        latString[4] = '0';

    if (abs(lon) < 10) {
        sprintf(&lonString[2], "%g", abs(lon));
    }
    else if (abs(lon) < 100) {
        sprintf(&lonString[1], "%g", abs(lon));
    }
    else {
        sprintf(lonString, "%g", abs(lon));
    }

    if (lonString[4] == '5')
        lonString[5] = '0';

    latString[2] = ',';
    lonString[3] = ',';

    if (lat < 0) {
        filePathTmp = filePathTmp + "\\S" + latString;
    }
    else {
        filePathTmp = filePathTmp + "\\N" + latString;
    }

    if (lon < 0) {
        filePathTmp = filePathTmp + "_W" + lonString + ".hgt";
    }
    else {
        filePathTmp = filePathTmp + "_E" + lonString + ".hgt";
    }

    *filePath = filePathTmp;
}

void CHgtFile::sphericalToFilePath(QString *filePath, float lon, float lat, int LOD) {

    QString filePathTmp;

    if (LOD < 4) {
        filePathTmp = "E:\\HgtReader_data\\L00_L03";
    }
    else if (LOD < 9) {
        filePathTmp = "E:\\HgtReader_data\\L04_L08";
    }
    else if (LOD < 14) {
        filePathTmp = "E:\\HgtReader_data\\L09_L13";
    }

    if (lon > 0 && lon < 180)  //0-180
        lon = lon;
    else if (lon > 180)  //180-360
        lon = lon - 360;
    else if (lon < 0)     //-180-0
        lon = lon;
    else if (lon == 180) {
        lon = -180;
    }

    //lon = lon - 180;
    //if (lon == 180)
    
    char latString[6] = "00,00";
    char lonString[7] = "000,00";
    if (abs(lat) < 10) {
        sprintf(&latString[1], "%g", abs(lat));
    }
    else {
        sprintf(latString, "%g", abs(lat));
    }

    if (abs(lon) < 10) {
        sprintf(&lonString[2], "%g", abs(lon));
    }
    else if (abs(lon) < 100) {
        sprintf(&lonString[1], "%g", abs(lon));
    }
    else {
        sprintf(lonString, "%g", abs(lon));
    }

    latString[2] = ',';
    lonString[3] = ',';

    if (lat < 0) {
        filePathTmp = filePathTmp + "\\S" + latString;
    }               
    else {          
        filePathTmp = filePathTmp + "\\N" + latString;
    }               
                    
    if (lon <= 0) {  
        filePathTmp = filePathTmp + "_W" + lonString + ".hgt";
    }               
    else {          
        filePathTmp = filePathTmp + "_E" + lonString + ".hgt";
    }

    *filePath = filePathTmp;
}

