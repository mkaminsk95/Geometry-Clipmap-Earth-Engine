#include "GClipmapThread.h"
#include "CCommons.h"
#include <chrono>


GClipmapThread::GClipmapThread(COpenGl* openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    camera = openGl->drawingState.getCamera();
   // clipmap = openGl->clipmapBuffer;
}

void GClipmapThread::run() {
    //clipmap.findPosition();
    
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
   
    while (true) {
        
        
       // findPosition();
        //openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
        //Sleep(2000);
    }
    
};

void GClipmapThread::findPosition() {

    double camX, camY, camZ;
    double lon, lat, rad;
    double tlon, tlat;
    
    //finding current cameraToEarth point
    camera->getCamPosition(&camX, &camY, &camZ);

    CCommons::getSphericalFromCartesian(camX, camY, camZ, &lon, &lat, &rad);
 
    CCommons::findTopLeftCorner(lon, lat, 15, &tlon, &tlat);

    CCommons::doubleIntoVSConsole(tlon);
    CCommons::stringIntoVSConsole(" ");
    CCommons::doubleIntoVSConsole(tlat);
    CCommons::stringIntoVSConsole("\n");
}