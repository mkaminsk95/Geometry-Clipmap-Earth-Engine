
#ifndef GLAYER_h
#define GLAYER_h

#include "CDrawingStateSnapshot.h"
#include "CCommons.h"
#include "GStructures.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMutex>


//#include "GClipmap.h"

class GPixel;
class GHeight;
class GClipmap;

class GLayer {
public:

	GLayer() {

	}
	GLayer(GClipmap* clipmapPointer, float degree, float inHgtFiledegree, float inScale, int inLayerIndex, 
			int inHgtSkipping, int inFileResolution, int inRawSkipping,int inRawFileResolution, int n);

	

	void buildLayer();
	
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
	GPixel* pixelManager;
	GHeight* heightManager;
	QOpenGLShaderProgram* program;

	int n;

	float horizontalOffset, verticalOffset;
	float allFinerHorizontalSum, allFinerVerticalSum;
	bool isActive;

	double oldLon, oldLat;
	double oldLonLeft, oldLonRight, oldLatTop, oldLatDown;

	double readDegree;
	bool firstGothrough;

	void updateLayer(double tlon, double tlat);
	void computeLayerPosition(double tlon, double tlat);
	void updateTextures();
	int tmp;


private: 


	point rTexBegHor, rTexBegVer, hTexBegHor, hTexBegVer;
	int hgtTextureBegginingY, hgtTextureBegginingX, rawTextureBegginingY, rawTextureBegginingX;
	int hgtTextureBegginingY2, hgtTextureBegginingX2, rawTextureBegginingY2, rawTextureBegginingX2;


	int scale;				
	float HgtFiledegree;    //How big is the angle that file covers

	int	HgtSkipping;			//We read every n-th point in HgtFile
	int HgtFileResolution;		//How many point there are in one scanline in corresponding Hgt file 
	int* drawMode;		//Are we drawing lines or triangles
	int layerIndex;
	

	QVector2D offsets;
	int fillerPositionHorizontal;
	int fillerPositionVertical;
	int positionHorizontal;
	int positionVertical;

	
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

	//QImage* heightMap;
	//QImage* pixelMap;
	QOpenGLTexture* heightTexture;
	QOpenGLTexture* pixelTexture;


	void mapDataIntoImages(double tlon, double tlat, int lonDifference, int latDifference);
	
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