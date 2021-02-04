
#ifndef GCLIPMAPTHREAD_H
#define GCLIPMAPTHREAD_H

#include <QThread>
#include <QTime>
#include "COpenGl.h"
#include "GLevel.h"
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
    
    float levelsOffsets[13];
    double levelsDegree[13];
    int n;


    GLevel level;

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