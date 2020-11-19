
#ifndef GCLIPMAPTHREAD_H
#define GCLIPMAPTHREAD_H

#include <QThread>
#include "COpenGl.h"
#include "GLayer.h"
#include "GClipmap.h"
#include "CCommons.h"



class COpenGl;

class GClipmapThread : public QThread
{
    //Q_OBJECT


public:
    GClipmapThread(COpenGl* openGl);

    COpenGl* openGl;
    GClipmap* clipmap;
    //void stop();

    int activeLvlOfDetail, highestLvlOfDetail;
    double distanceFromEarth;
    double tlon, tlat;
    
    float layersOffsets[13];
    double layersDegree[13];
    int n;

protected:
    void run();

	void computeDistanceFromLayerEdge();

    GLayer layer;

private:
	CDrawingStateSnapshot dss;
    CCamera *camera;
    QVector3D* cameraPosition;

	//methods
    void findPosition();
};

#endif