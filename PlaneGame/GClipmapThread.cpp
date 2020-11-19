#include "GClipmapThread.h"
#include "CCommons.h"
#include <chrono>
#include <math.h>

GClipmapThread::GClipmapThread(COpenGl* openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    camera = openGl->drawingState.getCamera();
    clipmap = openGl->clipmap;
}

void GClipmapThread::run() {
    //clipmap.findPosition();
    
    openGl->drawingState.getDrawingStateSnapshot(&dss);      // get current scene state
    
    for (int i = 0; i < 13; i++) {
        layersOffsets[i] = clipmap->layer[i].horizontalOffset;
        layersDegree[i] = clipmap->layer[i].readDegree;
    }
    highestLvlOfDetail = 9;
   
    while (true) {
        

        if (!clipmap->clipmapReady) {

            


            findPosition();
            
            computeDistanceFromLayerEdge();

            int treshold = 20000;

            if (distanceFromEarth < treshold)
                activeLvlOfDetail = 0;
            else if (distanceFromEarth < 2 * treshold)
                activeLvlOfDetail = 1;
            else if (distanceFromEarth < 4 * treshold)
                activeLvlOfDetail = 2;
            else if (distanceFromEarth < 8 * treshold)
                activeLvlOfDetail = 3;
            else if (distanceFromEarth < 16 * treshold)
                activeLvlOfDetail = 4;
            else if (distanceFromEarth < 32 * treshold)
                activeLvlOfDetail = 5;
            else if (distanceFromEarth < 64 * treshold)
                activeLvlOfDetail = 6;
            else if (distanceFromEarth < 128 * treshold)
                activeLvlOfDetail = 7;
            else if (distanceFromEarth < 256 * treshold)
                activeLvlOfDetail = 8;
            else
                activeLvlOfDetail = 9;

            /*if (distanceFromEarth < 5000)
                highestLvlOfDetail = 0;
            else if (distanceFromEarth < 10000)
                highestLvlOfDetail = 2;
            else if (distanceFromEarth < 20000)
                highestLvlOfDetail = 3;
            else if (distanceFromEarth < 40000)
                highestLvlOfDetail = 4;
            else if (distanceFromEarth < 80000)
                highestLvlOfDetail = 5;
            else if (distanceFromEarth < 160000)
                highestLvlOfDetail = 6;
            else if (distanceFromEarth < 320000)
                highestLvlOfDetail = 7;
            else if (distanceFromEarth < 640000)
                highestLvlOfDetail = 8;
            else
                highestLvlOfDetail = 9;*/

            //activeLvlOfDetail = 7;
            //highestLvlOfDetail = 8;

            for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
                clipmap->layer[x].updateLayer(tlon, tlat);

            for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
                clipmap->layer[x].computeLayerPosition(tlon, tlat);
            

            clipmap->activeLvlOfDetail =  activeLvlOfDetail;
            clipmap->highestLvlOfDetail = highestLvlOfDetail;

            clipmap->clipmapReady = true;
        
        }

    }
    
};

void GClipmapThread::computeDistanceFromLayerEdge() {


    double camX, camY, camZ;
    double posX, posY, posZ;
    double edgeLon, edgeLat;
    float distanceToNextEdge, distanceToPreviousEdge, distanceToHorizon;
    
    //computing distance to horizon
    distanceToHorizon = sqrt(2 * 6378100 * distanceFromEarth + distanceFromEarth * distanceFromEarth);
    
    //computing distance to next layer edge
    edgeLon = tlon - layersOffsets[highestLvlOfDetail] * layersDegree[highestLvlOfDetail];
    CCommons::getCartesianFromSpherical(edgeLon, tlat, 6378100,&posX,&posY,&posZ);
    camera->getCamPosition(&camX, &camY, &camZ);
    distanceToNextEdge = sqrt(pow(camX - posX, 2) + pow(camY - posY, 2) + pow(camZ - posZ, 2));

    //computing distance to previous edge
    edgeLon = tlon - layersOffsets[highestLvlOfDetail-1] * layersDegree[highestLvlOfDetail-1];
    CCommons::getCartesianFromSpherical(edgeLon, tlat, 6378100, &posX, &posY, &posZ);
    distanceToPreviousEdge = sqrt(pow(camX - posX, 2) + pow(camY - posY, 2) + pow(camZ - posZ, 2));

    CCommons::doubleIntoVSConsole(activeLvlOfDetail);
    CCommons::stringIntoVSConsole("    ");
    CCommons::doubleIntoVSConsole(highestLvlOfDetail);
    CCommons::stringIntoVSConsole("\n");

    //if horizon is more far away than layer edge, change highest edge
    if (distanceToHorizon > distanceToNextEdge && activeLvlOfDetail != highestLvlOfDetail && highestLvlOfDetail < 9)
        highestLvlOfDetail = highestLvlOfDetail + 1;
    else if (distanceToHorizon < distanceToPreviousEdge && activeLvlOfDetail != highestLvlOfDetail)
        highestLvlOfDetail = highestLvlOfDetail - 1;
}

void GClipmapThread::findPosition() {


    double camX, camY, camZ;
    double lon, lat, rad;


    double earthRadius = CONST_EARTH_RADIUS;

    //finding current cameraToEarth point
    camera->getCamPosition(&camX, &camY, &camZ);

    distanceFromEarth = sqrt(camX * camX + camY * camY + camZ * camZ) - 6378100;

    //finding right tile
    CCommons::getSphericalFromCartesian(camX, camY, camZ, &lon, &lat, &rad);
    CCommons::findTopLeftCorner(lon, lat, 0.0018311, &tlon, &tlat);

}