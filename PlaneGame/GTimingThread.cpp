#include "GTimingThread.h"
#include "CCommons.h"


GTimingThread::GTimingThread() {

	performance = CPerformance::getInstance();
	
	seconds = 0;
};

void GTimingThread::start() {

	while (true) {

		//sleep(5);

		seconds++;
		trianglesRead = performance->trianglesRead;
		CCommons::stringIntoVSConsole("sekunda ");
		CCommons::doubleIntoVSConsole(seconds);
	
		CCommons::stringIntoVSConsole("trojkaty wczytane ");
		CCommons::doubleIntoVSConsole(trianglesRead);
		CCommons::stringIntoVSConsole(" \n");


	}

}