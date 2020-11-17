
#ifndef GHEIGHT_h
#define GHEIGHT_h

#include <CHgtFile.h>
#include <qopengltexture.h>
#include <qopenglshaderprogram.h>
#include "GStructures.h"

class GLayer;
class GClipmap;


class GHeight
{
public: 

	GHeight(GLayer* layerPointer, int inLayerIndex, double inReadDegree, int inHgtSkipping, int inHgtFileResolution, int inHgtFileDegree);

	GLayer* layer;
	GClipmap* clipmap;
	QOpenGLShaderProgram* program;

	int layerIndex;
	int n;

	QOpenGLTexture* heightTexture;
	QImage* heightMap;

	double readDegree;		//How big is angle between two neighbours points in layer
	int hgtSkipping;		//We read every n-th point in rawFile
	int hgtFileResolution;	//How many point there are in one scanline in corresponding Raw file 
	int hgtFileDegree;

	void fullHgtTextureReading(double lonLeft, double lonRight, double latDown, double latTop);
	void horizontalBlockHgtTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop,
		double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegHor);
	void verticalBlockHgtTextureReading(int lonDifference, int latDifference, double lonLeft, double lonRight, double latDown, double latTop,
		double oldLonLeft, double oldLonRight, double oldLatDown, double oldLatTop, point texBegVer);

	void findingTopLeftFileToRead(double* maxTilesLon, double* maxTilesLat, double lonLeft, double latTop, double degree);
	void checkHowManyPointsToRead_X(int* howManyToReadX, int horizontalPosition, double lonLeftHor, double lonRightHor, float i, float fileDegree);
	void checkHowManyPointsToRead_Y(int* howManyToReadY, int verticalPosition, double latTopHor, double latDownHor, float j, float fileDegree);
	void checkFileOffset_X(int* filePositionHorizontalOffset, int horizontalPosition, double latTopHor, float maxTilesLat);
	void checkFileOffset_Y(int* filePositionVerticalOffset, int verticalPosition, double latTopHor, float maxTilesLat);
};

#endif