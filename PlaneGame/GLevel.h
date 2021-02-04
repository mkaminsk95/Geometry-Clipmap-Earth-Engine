
#ifndef GLevel_h
#define GLevel_h

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

class GLevel {
public:

	GLevel() {}
	GLevel(GClipmap* clipmapPointer, float degree, float inHgtFiledegree, float inScale, int inlevelIndex, 
			int inHgtSkipping, int inFileResolution, int inRawSkipping,int inRawFileResolution, int n);

	

	GClipmap* clipmap;
	//GPixel pixel;
	GPixel* pixelManager;
	GHeight* heightManager;
	QOpenGLShaderProgram* program;

	int n;

	float horizontalOffset, verticalOffset;
	float allFinerHorizontalSum, allFinerVerticalSum;
	bool isActive;

	double levelPositionLon, levelPositionLat;
	double oldLon, oldLat;
	double oldLonLeft, oldLonRight, oldLatTop, oldLatDown;

	double readDegree;
	bool firstGothrough;

	void computeOffsets();

	void refreshTextures(double tlon, double tlat);
	void refreshPositions(double tlon, double tlat);
	
	void updatePosition();
	void updateTextures();
	void updateOriginPoints();
	void buildLevel();

	
	void setPosition(int inPositionHorizontal, int inPositionVertical) {
		positionHorizontal = inPositionHorizontal;
		positionVertical = inPositionVertical;
	}

	bool imageReleased;
	void releaseImage();
	void initializeImage();

private: 

	CPerformance* performance;

	point rTexBegHor, rTexBegVer, hTexBegHor, hTexBegVer;
	int hgttextureOriginYRdy, hgttextureOriginXRdy, rawtextureOriginYRdy, rawtextureOriginXRdy;
	int hgttextureOriginY, hgttextureOriginX, rawtextureOriginY, rawtextureOriginX;


	int scale;			//How big is the clipmap
	int* drawMode;		//Are we drawing lines or triangles
	int levelIndex;		//Index of the level
	
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

	
	void drawA(float originX, float originY);
	void drawB(float originX, float originY);
	void drawC(float originX, float originY);
	void drawD(float originX, float originY);
	void drawE(float originX, float originY);
	void drawF(float originX, float originY);

};



#endif