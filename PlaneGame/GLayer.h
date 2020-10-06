
#ifndef GLAYER_h
#define GLAYER_h

#include "CDrawingStateSnapshot.h"
#include "CCommons.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>


//#include "GClipmap.h"


class GClipmap;

class GLayer {
public:

	GLayer() {

	}
	GLayer(GClipmap* clipmapPointer, float degree, float inHgtFiledegree, float inScale, int inLOD, int inLayerIndex, 
			int inHgtSkipping, int inFileResolution, int inRawSkipping,int inRawFileResolution, int n);

	struct point {
		int x;
		int y;
	};

	void buildLayer(double tlon, double tlat);
	
	void setFillerPosition(int inFillerPositionHorizontal, int inFillerPositionVertical) {
		
		fillerPositionHorizontal = inFillerPositionHorizontal;
		fillerPositionVertical = inFillerPositionVertical;
	}

	void setPosition(int inPositionHorizontal, int inPositionVertical) {

		positionHorizontal = inPositionHorizontal;
		positionVertical = inPositionVertical;
	}

	void setActive() {
		isActive = true;
	}

	void setInactive() {
		isActive = false;
	}
		

	GClipmap* clipmap;
	double horizontalOffset, verticalOffset;
	bool isActive;

private: 

	bool firstGothrough;

	point rTexBegHor, rTexBegVer, hTexBegHor, hTexBegVer;
	int hgtTextureBegginingY, hgtTextureBegginingX, rawTextureBegginingY, rawTextureBegginingX;
	int textureBegginingYtmp;

	float oldLon, oldLat;
	double oldLonLeft, oldLonRight, oldLatTop, oldLatDown;

	float readDegree;	//How big is angle between two neighbours points in layer
	float scale;				
	float HgtFiledegree;    //How big is the angle that file covers

	int	HgtSkipping;			//We read every n-th point in HgtFile
	int HgtFileResolution;		//How many point there are in one scanline in corresponding Hgt file 
	int rawSkipping;			//We read every n-th point in rawFile
	int rawFileResolution;		//How many point there are in one scanline in corresponding Raw file 

	int* drawMode;		//Are we drawing lines or triangles
	int LOD;			//Level Of Detail
	int layerIndex;
	int n;

	int fillerPositionHorizontal;
	int fillerPositionVertical;
	int positionHorizontal;
	int positionVertical;

	QOpenGLShaderProgram* program;

	QOpenGLVertexArrayObject* vaoA;
	QOpenGLVertexArrayObject* vaoB;
	QOpenGLVertexArrayObject* vaoC;
	QOpenGLVertexArrayObject* vaoD;
	QOpenGLVertexArrayObject* vaoE;
	QOpenGLVertexArrayObject* vaoF;

	QOpenGLBuffer* indexA_Buffer;
	QOpenGLBuffer* indexB_Buffer;
	QOpenGLBuffer* indexC_Buffer;
	QOpenGLBuffer* indexD_Buffer;
	QOpenGLBuffer* indexE_Buffer;
	QOpenGLBuffer* indexF_Buffer;

	QOpenGLBuffer* vertexA_Buffer;
	QOpenGLBuffer* vertexB_Buffer;
	QOpenGLBuffer* vertexC_Buffer;
	QOpenGLBuffer* vertexD_Buffer;
	QOpenGLBuffer* vertexE_Buffer;
	QOpenGLBuffer* vertexF_Buffer;

	QImage* heightMap;
	QImage* pixelMap;
	QOpenGLTexture* heightTexture;
	QOpenGLTexture* pixelTexture;

	void mapPixelDataIntoTexture(double tlon, double tlat);
	void mapHeightDataIntoTexture(double tlon, double tlat);
	
	//full reading
	void fullHgtTextureReading(double lonLeft, double lonRight, double latTop, double latDown);
	void fullRawTextureReading(double lonLeft, double lonRight, double latTop, double latDown);

	//vertical reading
	void verticalBlockRawTextureReading(int lonDifference, int latDifference, 
		double lonLeft, double lonRight, double latTop, double latDown, point texBegHVer);
	void verticalBlockHgtTextureReading(int lonDifference, int latDifference, 
		double lonLeft, double lonRight, double latTop, double latDown, point texBegHVer);

	//horizontal reading
	void horizontalBlockRawTextureReading(int lonDifference, int latDifference, 
		double lonLeft, double lonRight, double latTop, double latDown, point texBegHor);
	void horizontalBlockHgtTextureReading(int lonDifference, int latDifference,
		double lonLeft, double lonRight, double latTop, double latDown, point texBegHor);
	
	void checkHowManyPixelsToReadFromRaw_X(int* howManyToReadX, int horizontalPosition, double lonLeftHor, double lonRightHor, float i, float fileDegree, bool rawReading);
	void checkHowManyPixelsToReadFromRaw_Y(int* howManyToReadY, int verticalPosition, double latTopHor, double latDownHor, float j, float fileDegree, bool rawReading);
	void checkRawFileOffset_X(int* filePositionHorizontalOffset, int horizontalPosition, double latTopHor, float maxTilesLat);
	void checkRawFileOffset_Y(int* filePositionVerticalOffset, int verticalPosition, double latTopHor, float maxTilesLat);

	void findingTopLeftFileToRead(float *maxTilesLon, float *maxTilesLat, double lonLeft, double latTop, double degree);

	void computeTextureOffsets(int latDifference, int lonDifference, point* texBegHor, point* texBegVer, bool rawReading);
	void computeNewLonAndLat(double tlon, double tlat, double *lonLeft, double *lonRight, double *latTop, double *latDown);


	void cumputeOffsets();
	void drawA(float originX, float originY);
	void drawB(float originX, float originY);
	void drawC(float originX, float originY);
	void drawD(float originX, float originY);
	void drawE(float originX, float originY);
	void drawF(float originX, float originY);



};

#endif