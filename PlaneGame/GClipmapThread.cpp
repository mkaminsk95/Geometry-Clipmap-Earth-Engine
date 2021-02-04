#include "GClipmapThread.h"
#include "CCommons.h"
#include <chrono>
#include <math.h>
#include <GLevel.h>
#include <vector>
#include "CPerformance.h"

GClipmapThread::GClipmapThread(COpenGl* openGlPointer) : QThread(openGlPointer), openGl(openGlPointer)
{
    camera = openGl->drawingState.getCamera();
    clipmap = openGl->clipmap;
}

void GClipmapThread::run() {
    
    CPerformance* performance = CPerformance::getInstance();

    vector<GLevel*> level;

    for (int i = 0; i < 13; i++) {
        level.push_back(&clipmap->level[i]);
        levelsOffsets[i] = level[i]->horizontalOffset;
        levelsDegree[i] =  level[i]->readDegree;
    }
    highestLvlOfDetail = 12;
   

  
    while (true) {
        
        
        if (!clipmap->clipmapReady) {
            
    
            performance->trianglesRead = 0;



            findPosition();

            treshold = 8840;

            getActiveLvlOfDetail();
            getHighestLvlOfDetail();


            for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
                level[x]->refreshTextures(tlon, tlat);         
        
            for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
                level[x]->refreshPositions(tlon, tlat);

            clipmap->activeLvlOfDetail =  activeLvlOfDetail;
            clipmap->highestLvlOfDetail = highestLvlOfDetail;

            clipmap->clipmapReady = true;


        }

    }
    
};

void GClipmapThread::getActiveLvlOfDetail() {


    if (distanceFromEarth < 2 * treshold)    activeLvlOfDetail = 0;  else   //~18km
    if (distanceFromEarth < 4 * treshold)    activeLvlOfDetail = 1;  else   //~36km
    if (distanceFromEarth < 8 * treshold)    activeLvlOfDetail = 2;  else   //~72km
    if (distanceFromEarth < 16 * treshold)   activeLvlOfDetail = 3;  else   //~144km
    if (distanceFromEarth < 32 * treshold)   activeLvlOfDetail = 4;  else   //~288km
    if (distanceFromEarth < 64 * treshold)   activeLvlOfDetail = 5;  else   //~576km
    if (distanceFromEarth < 128 * treshold)  activeLvlOfDetail = 6;  else   //~1152km
    if (distanceFromEarth < 256 * treshold)  activeLvlOfDetail = 7;  else   //~2304km 
    if (distanceFromEarth < 512 * treshold)  activeLvlOfDetail = 8;  else   //~4608km
    if (distanceFromEarth < 1024 * treshold) activeLvlOfDetail = 9;  else   //~9216km
    if (distanceFromEarth < 2048 * treshold) activeLvlOfDetail = 10; else   //~18432km
    if (distanceFromEarth < 4096 * treshold) activeLvlOfDetail = 11; else   //~36864km
                                             activeLvlOfDetail = 12;
}

void GClipmapThread::getHighestLvlOfDetail() {



        double camX, camY, camZ;
        double posX, posY, posZ;
        double edgeLon, edgeLat;
        float distanceToHighestLvlEdge, distanceToPreviousEdge, distanceToHorizon;
        
        //computing distance to horizon
        distanceToHorizon = sqrt(2 * 6378100 * distanceFromEarth + distanceFromEarth * distanceFromEarth);
        
        //computing distance to next level edge
        edgeLon = tlon - levelsOffsets[highestLvlOfDetail] * levelsDegree[highestLvlOfDetail];
        CCommons::getCartesianFromSpherical(edgeLon, tlat, 6378100,&posX,&posY,&posZ);
        camera->getCamPosition(&camX, &camY, &camZ);
        distanceToHighestLvlEdge = sqrt(pow(camX - posX, 2) + pow(camY - posY, 2) + pow(camZ - posZ, 2));

        //computing distance to previous edge
        edgeLon = tlon - levelsOffsets[highestLvlOfDetail-1] * levelsDegree[highestLvlOfDetail-1];
        CCommons::getCartesianFromSpherical(edgeLon, tlat, 6378100, &posX, &posY, &posZ);
        distanceToPreviousEdge = sqrt(pow(camX - posX, 2) + pow(camY - posY, 2) + pow(camZ - posZ, 2));


        //if horizon is more far away than level edge, change highest edge
        if (distanceToHorizon > distanceToHighestLvlEdge && highestLvlOfDetail < 12)
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