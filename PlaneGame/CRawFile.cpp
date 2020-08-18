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
#include "CRawFile.h"

using namespace std;

CRawFile::CRawFile()
{
    sizeX = 0;
    sizeY = 0;
    pixel = 0;
    externalPixelPointer = false;
}

CRawFile::~CRawFile()
{
    if (!externalPixelPointer && pixel!=0)
        delete []pixel;
}

void CRawFile::init(int sX, int sY)
{
    if (pixel!=0)
        delete []pixel;

    sizeX = sX;
    sizeY = sY;

    pixel = new CRawPixel[sizeX*sizeY];
}

unsigned char *CRawFile::getPixelsPointer()
{
    return (unsigned char *)pixel;
}

void CRawFile::setPixelsPointer(int sx, int sy, CRawPixel *p)
{
    externalPixelPointer = true;
    sizeX = sx;
    sizeY = sy;
    pixel = p;
}

void CRawFile::savePGM(QString name)
{
    if (pixel==0) return;
    fstream filePGM;
    CRawPixel pixel;
    int pixelPGM;
    int nr, x, y;

    // save PGM file to disk
    filePGM.open(name.toUtf8(), fstream::out);

    filePGM << "P2" << endl;
    filePGM << sizeX << " " << sizeY << endl << 255;
    nr = 0;
    for (y=0; y<sizeY; y++)
      for (x=0; x<sizeX; x++) {
        if (nr%15==0) filePGM << endl;

        pixel = getPixel(x, y);
        pixelPGM = (int)( ((double)(pixel.r + pixel.g + pixel.b))/3.0 );
        if (pixelPGM>255)
            pixelPGM = 255;
        filePGM << pixelPGM << " ";
        nr++;
      }

    filePGM.close();
}

void CRawFile::saveFile(QString name)
{
    if (pixel==0) return;
    fstream fileRaw;

    // save RAW file to disk
    fileRaw.open(name.toUtf8(), fstream::out | fstream::binary);
    fileRaw.write((char *)pixel, sizeX*sizeY*3);
    fileRaw.close();
}

void CRawFile::loadFile(QString name, int x, int y)
{
    init(x, y);
    fstream fileRaw;

    // load RAW file to memory
    fileRaw.open(name.toUtf8(), fstream::in | fstream::binary);
    fileRaw.read((char *)pixel, sizeX*sizeY*3);
    fileRaw.close();
}

void CRawFile::getPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = getPixel(x + X*skip, y + Y*skip);
            i++;
        }
}

void CRawFile::setPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            setPixel(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}

void CRawFile::fileOpen(QString name, int sX, int sY)
{
    sizeX = sX;
    sizeY = sY;
    file.open(name.toUtf8(), fstream::in | fstream::out | fstream::binary);
}

void CRawFile::fileClose()
{
    file.close();
}

void CRawFile::fileSetPixel(int x, int y, CRawPixel pix)
{
    file.seekp((y*sizeX + x)*3);
    file.write((char *)(&pix), 3);
}

CRawPixel CRawFile::fileGetPixel(int x, int y)
{
    CRawPixel pix;

    file.seekg((y*sizeX + x)*3);
    file.read((char *)(&pix), 3);

    return pix;
}

void CRawFile::fileGetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            buffer[i] = fileGetPixel(x + X*skip, y + Y*skip);
            i++;
        }
}

void CRawFile::fileSetPixelBlock(CRawPixel *buffer, int x, int y, int sx, int sy, int skip)
{
    int i;
    int X, Y;

    i = 0;
    for (Y=0; Y<sy; Y++)
        for (X=0; X<sx; X++) {
            fileSetPixel(x + X*skip, y + Y*skip, buffer[i]);
            i++;
        }
}


void CRawFile::loadPixelDataToImage(QImage* image, int imageOffsetX, int imageOffsetY, QString name,
    int verticalPosition, int horizontalPosition, int xToRead, int yToRead, int fileResolution, int skip,
    int positionHorizontalOffset, int positionVerticalOffset)
{
    fstream filePixel;
    quint8 pixel[3];
    uchar* bits;
        
    long offset;
    int n = image->height() - 1;
  
    offset = (positionVerticalOffset * fileResolution + positionHorizontalOffset) * skip * 3;

    // load HGT file to memory
    filePixel.open(name.toUtf8(), fstream::in | fstream::binary);

    bool opened = filePixel.is_open();

    int z = 0;
    for (int y = imageOffsetY; y < imageOffsetY+yToRead; y++) {
        
        filePixel.seekg(offset + 3 * skip * z * fileResolution);
        long tmp = offset + 3 * skip * z * fileResolution;
        
        bits = image->scanLine(n-y);

        for (int x = imageOffsetX; x < imageOffsetX+xToRead; x++) {

            if (true) {

            }

            filePixel.read((char*)&pixel[0], 1);
            filePixel.read((char*)&pixel[1], 1);
            filePixel.read((char*)&pixel[2], 1);
          
            filePixel.seekg(skip*3-3, filePixel.cur);

            *(bits + 3*x) =       pixel[0];
            *(bits + 3*x + 1) =   pixel[1];
            *(bits + 3*x + 2) =   pixel[2];
       
        }

        z++;
    }
    
    filePixel.close();
}

void CRawFile::loadPixelDataToImage2(QImage* image, int imageOffsetX, int imageOffsetY, QString name,
    int verticalPosition, int horizontalPosition, int xToRead, int yToRead, int fileResolution, int skip,
    int positionHorizontalOffset, int positionVerticalOffset, int textureBegginingLon, int textureBegginingLat,
    int movementCase)
{
    fstream filePixel;
    quint8 pixel[3];
    uchar* bits;

    long offset;
    int n = image->height() - 1;

    offset = (positionVerticalOffset * fileResolution + positionHorizontalOffset) * skip * 3;
  
   

    // load HGT file to memory
    filePixel.open(name.toUtf8(), fstream::in | fstream::binary);

    bool opened = filePixel.is_open();

    int z = 0;
    for (int y = imageOffsetY; y < imageOffsetY + yToRead; y++) {

        filePixel.seekg(offset + 3 * skip * z * fileResolution);
        long tmp = offset + 3 * skip * z * fileResolution;

        bits = image->scanLine(n - y);

        for (int x = imageOffsetX; x < imageOffsetX + xToRead; x++) {

            filePixel.read((char*)&pixel[0], 1);
            filePixel.read((char*)&pixel[1], 1);
            filePixel.read((char*)&pixel[2], 1);


            filePixel.seekg(skip * 3 - 3, filePixel.cur);

            *(bits + 3 * x) = pixel[0];
            *(bits + 3 * x + 1) = pixel[1];
            *(bits + 3 * x + 2) = pixel[2];

        }

        z++;
    }

    filePixel.close();
}

void CRawFile::sphericalToFilePath(QString* filePath, float lon, float lat, int LOD) {

    QString filePathTmp;

    if (LOD < 6) {
        filePathTmp = "E:\\HgtReader_data\\Textures\\L00_L02";
    }
    else if (LOD < 9) {
        filePathTmp = "E:\\HgtReader_data\\Textures\\L03_L05";
    }
    else if (LOD < 12) {
        filePathTmp = "E:\\HgtReader_data\\Textures\\L06_L08";
    }
    else {
        filePathTmp = "E:\\HgtReader_data\\Textures\\L09_L10";
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
        filePathTmp = filePathTmp + "_W" + lonString + ".raw";
    }
    else {
        filePathTmp = filePathTmp + "_E" + lonString + ".raw";
    }

    *filePath = filePathTmp;
}