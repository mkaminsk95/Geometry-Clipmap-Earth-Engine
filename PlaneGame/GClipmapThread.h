
#ifndef GCLIPMAPTHREAD_H
#define GCLIPMAPTHREAD_H

#include <QThread>
#include <QTime>
#include "COpenGl.h"
#include "GLayer.h"
#include "GClipmap.h"
#include "CCommons.h"



class COpenGl;

class GClipmapThread : public QThread
{
public:

    GClipmapThread(COpenGl* openGl);

    COpenGl* openGl;
    GClipmap* clipmap;

    int activeLvlOfDetail, highestLvlOfDetail;
    double distanceFromEarth;
    double tlon, tlat;
    
    float layersOffsets[13];
    double layersDegree[13];
    int n;


    GLayer layer;

private:

    int treshold;

    QTime time;
    CCamera *camera;
	CDrawingStateSnapshot dss;
    QVector3D* cameraPosition;

	//methods
    void run();
    void findPosition();
	void getHighestLvlOfDetail();
    void getActiveLvlOfDetail();

};

#endif