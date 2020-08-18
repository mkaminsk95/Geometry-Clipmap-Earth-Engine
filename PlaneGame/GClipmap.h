
#ifndef GCLIPMAP_H
#define GCLIPMAP_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QImage>
#include <QVector>

#include "CDrawingStateSnapshot.h"
#include "COpenGl.h"

//#include "ogl.h"


class COpenGl;
class GLayer;


class GClipmap  {
public:

	GClipmap(COpenGl *openGl);
	~GClipmap();

	//public variables
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

	int howManyToRenderA;
	int howManyToRenderB;
	int howManyToRenderC;
	int howManyToRenderD;
	int howManyToRenderE;
	int howManyToRenderF;

	int HgtTextureLocation;
	int pixelTextureLocation[13];
	int drawingMode;
	int activeLevelOfDetail;
	QVector<GLayer> layer;
	

	//public methods
	void draw();

	void setDrawingStateSnapshot(CDrawingStateSnapshot* dss) {
		drawingStateSnapshot = dss;
	}
	void setCamInfo(double windowAspectRatio, double zNear, double zFar) {
		this->windowAspectRatio = windowAspectRatio;
		this->zNear = zNear;
		this->zFar = zFar;
	}

private:

	//private variables
	COpenGl*	openGl;
	CCamera*	camera;
	QVector3D* cameraPosition;
	QVector3D* grid;
	

	
	bool initialized;
	CDrawingStateSnapshot* drawingStateSnapshot;

	int positionInputLocation;

	//data for MVP matrix
	double xCenter;
	double yCenter;
	double zCenter;
	double windowAspectRatio;
	double zNear;
	double zFar;

	//
	double distanceFromEarth;
	double tlon, tlat;

	//methods
	void findPosition();

	void initializeA_Buffer();
	void initializeB_Buffer();
	void initializeC_Buffer();
	void initializeD_Buffer();
	void initializeE_Buffer();
	void initializeF_Buffer();

	//void buildLayer();

	QMatrix4x4 generateModelViewMatrix();

};

#endif
