#ifndef __CROSSCORRELATION_H__
#define __CROSSCORRELATION_H__

#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <sched.h> 
#include "Thread.hpp"
#include "Configuration.hpp"
#include "Tools.hpp"
#include "Log.hpp"

static pthread_cond_t  sov_calcFinishedSigCond 		= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sov_calcFinishedSigMutex		= PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t 	sop_capturedSigCond  [3] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};
static pthread_mutex_t  sop_capturedSigMutex [3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

static int sov_cpuOfTheMainThread;
enum XCorrErrors 		{XCORR_OK};
enum XCorrThreadErrors 	{XCORRTHREAD_VALUE_RERADY, XCORRTHREAD_STILL_SYNCHRONIZING};
enum XCorrElemErrors 	{XCORRELEM_OK, XCORRELEM_INDEX_ERROR};

enum XCorrStates 		{THREAD_INIT,FAST_TS_SEEKING,NORMAL_TS_SEEKING};
enum XCorrElemStates 	{SEEKING,TS_FOUND};

class XCorrThread;
class XCorrElem;
class CrossCorrelation{
private:
	std::vector<XCorrThread> 	ov_xcorrThreads;
	std::string 				ov_fileName;
	short*						op_capturedSig;
	short*						op_refFile;
	long*						op_capturedSignalNrg2;
	XCorrStates 				ov_xCorrState;
	long 						ov_unixTSOfLastSample;
	bool						ov_isTheFirstCapturedSec;
	
public:
	int init();
	int process();
	int	getCapturedSignalPointer(short* &ap_capturedSigPtr);
	int getTimeStamp(long &av_timeStamp);

	virtual~ CrossCorrelation(){
		delete[] op_capturedSig;
		delete[] op_refFile;
		delete[] op_capturedSignalNrg2;
	}
	CrossCorrelation(const std::string &av_fileName):
		ov_fileName(av_fileName),
		ov_xCorrState(THREAD_INIT),
		ov_isTheFirstCapturedSec(true){}
};

class XCorrThread : public Thread{
private:
	std::vector<XCorrElem> 		ov_xCorrElements;
	short*						op_capturedSig;
	long*						op_capturedSignalNrg2;
	long 						ov_timeStamp;
	bool						ov_isTSFound;
	bool						ov_isThreadReady;
	int 						ov_threadNum;

protected:
	virtual void run();

public:
	XCorrThread(short* ap_capturedSig, long* ap_capturedSignalNrg2, int av_threadNumber):
		op_capturedSig(ap_capturedSig),
		op_capturedSignalNrg2(ap_capturedSignalNrg2),
		ov_timeStamp(-1),
		ov_isTSFound(false),
		ov_isThreadReady(false),
		ov_threadNum(av_threadNumber)
	{
		LOG_INFO<<"new XCorrThread";
	}
	int 	addRefSignal(short* ap_refSignal,int av_signalLen, int av_initialTS);
	bool 	getTimeStamp(long &av_timeStamp);
#ifdef CORETEST
	std::vector<XCorrElem>* 	getXCorrElemTable();
	void						runCondition();
#endif
};

class XCorrElem{
private:
	std::vector<short*> 		ov_refSignals;
	std::vector<long>			ov_nrg2RefSignals;
	long 						ov_firstSecTS;
	int 						ov_maxIndex;
	
public:
	XCorrElemStates				ov_xCorrElemState;
	int 						ov_validationLevel;
	long						ov_estimatedTS;

	XCorrElem(long av_firstSecTS):
		ov_firstSecTS(av_firstSecTS),
		ov_xCorrElemState(SEEKING),
		ov_validationLevel(0)
	{
	}

	virtual ~XCorrElem(){
		for (auto it = ov_refSignals.begin(); it != ov_refSignals.end(); ++it)
			delete[] *it;
	}

	int addRefSignal(short* ap_refSignal,int av_signalLen);
	int processCapturedSignal(short* ap_capturedSignal,long* ap_capturedSignalNrg2);

#ifdef CORETEST
	std::vector<short*>* 	getRefSignals();
	std::vector<long>* 		getNrg2RefSignals();
	int 					getMaxIndex();
	long 					getfirstSecTS();
#endif
};
#endif