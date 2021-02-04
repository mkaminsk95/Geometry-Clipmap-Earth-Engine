
#ifndef GPIXEL_h
#define GPIXEL_h

#include <CRawFile.h>
#include <QOpenGLTexture>
#include <qopenglshaderprogram.h>
#include "GStructures.h"
#include "QImage"

class GLevel;
class GClipmap;



class GPixel
{
public:


	GPixel(GLevel* levelPointer, int inlevelIndex, double inReadDegree, int inRawSkipping, int inRawFileResolution);
	void initializeImage();
	void releaseImage();
	QImage pixell;

	GLevel* level;
	GClipmap* clipmap;
	QOpenGLShaderProgram* program;

	int levelIndex;
	int n;
	
	QOpenGLTexture* pixelTexture;
	QImage* pixelMap;
	bool imageReleased;

	double readDegree;		//How big is angle between two neighbours points in level
	int rawSkipping;		//We read every n-th point in rawFile
	int rawFileResolution;	//How many point there are in one scanline in corresponding Raw file 

	double pointDegree;     //Distance between points in grid (may differ from readDegree)

	void fullRawTextureReading(double lonLeft, double lonRight, double latDown, double latTop);
	void horizontalBlockRawTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop, 
								double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegHor);
	void verticalBlockRawTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop, 
								double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegHor);

	void findingTopLeftFileToRead(double* maxTilesLon, double* maxTilesLat, double lonLeft, double latTop, double degree);
	void checkHowManyPointsToRead_X(int* howManyToReadX, int horizontalPosition, double lonLeftHor, double lonRightHor, float i, float fileDegree);
	void checkHowManyPointsToRead_Y(int* howManyToReadY, int verticalPosition, double latTopHor, double latDownHor, float j, float fileDegree);
	void checkFileOffset_X(int* filePositionHorizontalOffset, int horizontalPosition, double latTopHor, float maxTilesLat);
	void checkFileOffset_Y(int* filePositionVerticalOffset, int verticalPosition, double latTopHor, float maxTilesLat);

};

#endif