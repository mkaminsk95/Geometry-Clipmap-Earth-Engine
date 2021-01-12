
#ifndef GLAYER_h
#define GLAYER_h

#include "CDrawingStateSnapshot.h"
#include "CCommons.h"
#include "CPerformance.h"
#include "GStructures.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMutex>


class GPixel;
class GHeight;
class GClipmap;

class GLayer {
public:

	GLayer() {}
	GLayer(GClipmap* clipmapPointer, float degree, float inHgtFiledegree, float inScale, int inLayerIndex, 
			int inHgtSkipping, int inFileResolution, int inRawSkipping,int inRawFileResolution, int n);

	

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

	void buildLayer();
	void computeLayerPosition(double tlon, double tlat);
	void updateLayer(double tlon, double tlat);
	void updateTextures();
	
	void setPosition(int inPositionHorizontal, int inPositionVertical) {
		positionHorizontal = inPositionHorizontal;
		positionVertical = inPositionVertical;
	}


private: 

	CPerformance* performance;

	point rTexBegHor, rTexBegVer, hTexBegHor, hTexBegVer;
	int hgtTextureBegginingYBuff, hgtTextureBegginingXBuff, rawTextureBegginingYBuff, rawTextureBegginingXBuff;
	int hgtTextureBegginingY, hgtTextureBegginingX, rawTextureBegginingY, rawTextureBegginingX;


	int scale;			//How big is the clipmap
	int* drawMode;		//Are we drawing lines or triangles
	int layerIndex;		//Index of the layer
	
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