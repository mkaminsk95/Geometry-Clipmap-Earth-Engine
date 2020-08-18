
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

protected:
    void run();

    GLayer layer;

private:
	CDrawingStateSnapshot dss;
    CCamera *camera;
    QVector3D* cameraPosition;

	//methods
    void findPosition();
};

#endif