#pragma once

#include <QThread>
#include "CPerformance.h"

class GTimingThread : public QThread
{
public:
    GTimingThread();

    CPerformance* performance;
    void stop(); 
    void start();


private:

    int trianglesRead;
    int seconds;
    

};

