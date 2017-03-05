#include "CrossCorrelation.hpp"

#ifdef CORETEST
std::vector<short*>* XCorrElem::getRefSignals(){
	return &ov_refSignals;
}
std::vector<long>* XCorrElem::getNrg2RefSignals(){
	return &ov_nrg2RefSignals;
}
int XCorrElem::getMaxIndex(){
	return ov_maxIndex;
}
long XCorrElem::getfirstSecTS(){
	return ov_firstSecTS;
}
std::vector<XCorrElem>* XCorrThread::getXCorrElemTable(){
	return &ov_xCorrElements;
}
void XCorrThread::runCondition(){
	//pthread_cond_signal(&sov_capturedSigCond);
}
#endif

int CrossCorrelation::init(){
	FILE*   lp_file;
	int     i,lv_size;
	
	lp_file = std::fopen (ov_fileName.c_str() , "rb" );
	fseek (lp_file , 0 , SEEK_END);
	lv_size = ftell (lp_file);
	rewind (lp_file);
	op_capturedSig 			= new short[SAMPLING_RATE << 1];
	op_capturedSignalNrg2 	= new long[SAMPLING_RATE];
	op_refFile				= new short[lv_size >> 1];
	#ifdef __arm__
	sov_cpuOfTheMainThread	= sched_getcpu();
	#endif

	//LOG_INFO<<"SAMPLING_RATE << 1 = "<<(SAMPLING_RATE << 1);
	//LOG_INFO<<"SAMPLING_RATE = "<<SAMPLING_RATE ;
	//LOG_INFO<<"lv_size >> 1 = "<<(lv_size >> 1);
	int n =fread (op_refFile, sizeof(short), lv_size >> 1, lp_file);
	//LOG_INFO<<"sizeof(short) * (lv_size >> 1) = "<<(sizeof(short) * (lv_size >> 1))<<" n = "<<n;

	for(i = 0 ; i < NB_OF_THREADS ; i++){
		ov_xcorrThreads.emplace_back(XCorrThread(op_capturedSig,op_capturedSignalNrg2,i+1));
	}

	i = 0;
	for (auto it = ov_xcorrThreads.begin(); it != ov_xcorrThreads.end(); ++it){
		//LOG_INFO<<"addRefSignal (thread num "<< i <<") : begin = "<<5 * 60 * SAMPLING_RATE * i<<" end = "<<(5 * 60 * SAMPLING_RATE * i+5 * 60 * SAMPLING_RATE);
		(*it).addRefSignal(&op_refFile[5 * 60 * SAMPLING_RATE * i], 5 * 60 * SAMPLING_RATE, 5 * 60 * 1000 * i);
		(*it).start();
		i++;
	}
	fclose(lp_file);
	return 0;
}

int CrossCorrelation::process(){
	int i;
	ov_unixTSOfLastSample = time(0);
	LOG_INFO << "op_capturedSig = "<<op_capturedSig;
	LOG_INFO << "op_capturedSig + SAMPLING_RATE = "<<op_capturedSig + SAMPLING_RATE;
	if(ov_isTheFirstCapturedSec){
		LOG_INFO << "ov_isTheFirstCapturedSec";
		ov_isTheFirstCapturedSec = false;
		memmove(op_capturedSig, op_capturedSig + SAMPLING_RATE, SAMPLING_RATE << 1);
	}
	else{
		LOG_INFO << "op_capturedSig[0] = " <<op_capturedSig[0] << " op_capturedSig[1] = " <<op_capturedSig[1] ;
		op_capturedSignalNrg2[0] = 0;
    	for(i = 0; i < SAMPLING_RATE; i++)
    		op_capturedSignalNrg2[0] += op_capturedSig[i] * op_capturedSig[i];
    	for(i = 1; i < SAMPLING_RATE; i++)
    		op_capturedSignalNrg2[i] =    op_capturedSignalNrg2[i - 1] 
                            			- op_capturedSig[i - 1] * op_capturedSig[i - 1]
                            			+ op_capturedSig[i + SAMPLING_RATE - 1] * op_capturedSig[i + SAMPLING_RATE - 1];
        //pthread_cond_broadcast(&sov_capturedSigCond);  
        pthread_cond_signal(&sop_capturedSigCond[0]);               			
        for(int j=0;j<NB_OF_THREADS;j++)
        	pthread_cond_wait( &sov_calcFinishedSigCond,&sov_calcFinishedSigMutex);
        memmove(op_capturedSig, op_capturedSig + SAMPLING_RATE, SAMPLING_RATE << 1);          			
	}
	
	return 0;
}

int CrossCorrelation::getTimeStamp(long &av_timeStamp){

	for (auto it = ov_xcorrThreads.begin(); it != ov_xcorrThreads.end(); ++it)
		if((*it).getTimeStamp(av_timeStamp)){
			std::cout<<"hello timstamp"<<std::endl;
			av_timeStamp += (time(0) - ov_unixTSOfLastSample);
			return 0;
		}
	return -1;
}

int CrossCorrelation::getCapturedSignalPointer(short* &ap_capturedSigPtr){
	ap_capturedSigPtr = op_capturedSig + SAMPLING_RATE;
	return 0;
}

bool XCorrThread::getTimeStamp(long &av_timeStamp){
	av_timeStamp = ov_timeStamp;
	return ov_isTSFound;
}

void XCorrThread::run(){
	#ifdef __arm__
	cpu_set_t cpuset;
	sov_cpuOfTheMainThread = (sov_cpuOfTheMainThread+1)%4;
	CPU_ZERO(&cpuset);
	CPU_SET( sov_cpuOfTheMainThread , &cpuset);
	sched_setaffinity(0, sizeof(cpuset), &cpuset);
	std::cout << "Thread #" << "  : on CPU " << sched_getcpu() << std::endl;
	#endif
	while(1){
		ov_isThreadReady = true;
		pthread_cond_wait( &sop_capturedSigCond[ov_threadNum-1],&sop_capturedSigMutex[ov_threadNum-1]);
		if(ov_threadNum < 3)
			pthread_cond_signal(&sop_capturedSigCond[ov_threadNum]);
		printf(" thread num : %d T1: %.2f ms\n",ov_threadNum, (double)(clock())/CLOCKS_PER_MSEC);
		for (auto it = ov_xCorrElements.begin(); it != ov_xCorrElements.end(); ++it){
			(*it).processCapturedSignal(op_capturedSig,op_capturedSignalNrg2);
			if((*it).ov_validationLevel == NB_OF_SECONS_REQUIRED_FOR_VALIDATION){
				ov_isTSFound = true;
				ov_timeStamp = (*it).ov_estimatedTS;
			}
		}
		printf("thread num : %d T2: %.2f ms\n",ov_threadNum, (double)(clock())/CLOCKS_PER_MSEC);
		pthread_cond_signal(&sov_calcFinishedSigCond);
	}
}

int XCorrThread::addRefSignal(short* ap_refSignal,int av_signalLen, int av_initialTS){
	int i,j;
	int lv_initialTSInSamplesUnit = av_initialTS * 8;
	for(i = 1 ; i < NB_OF_MINUTE_HANDLED_IN_A_THREAD * 2 ; i += 2){
		ov_xCorrElements.emplace_back(XCorrElem(av_initialTS + 30000 * i));
	}

	j=1;
	for (auto it = ov_xCorrElements.begin(); it != ov_xCorrElements.end(); ++it){
		for(i = 0 ; i < 5 ; i++){
			//LOG_INFO<<"addRefSignal : begin = "<< (30 * j + i+av_initialTS/1000)  <<" end = "<<( (30 * j + i)+1+av_initialTS/1000);
			(*it).addRefSignal(&ap_refSignal[SAMPLING_RATE * (30 * j + i)] , SAMPLING_RATE);
		}
		j+=2;
	}

	return 0;
}

int XCorrElem::addRefSignal(short* ap_refSignal,int av_signalLen){
	int i;
	short* lp_audioFileSignal = new short[av_signalLen];
	long   lp_nrg2 = 0;

	memcpy(lp_audioFileSignal,ap_refSignal,av_signalLen*sizeof(short));
	

	for(i = 0 ; i < av_signalLen ; i++){
		lp_nrg2 += ap_refSignal[i] * ap_refSignal[i];
	}

	ov_refSignals.emplace_back(lp_audioFileSignal);
	ov_nrg2RefSignals.emplace_back(lp_nrg2);

	return XCORRELEM_OK;
}

int XCorrElem::processCapturedSignal(short* ap_capturedSignal,long* ap_capturedSignalNrg2){
	int 	i,j;
	long 	lv_xCorr;
	long 	lv_oldXCorr;
	short*	lp_refSignal 			= ov_refSignals[ov_validationLevel];
	long 	lv_nrg2Signal 			= ov_nrg2RefSignals[ov_validationLevel];
	float 	lv_normalizedXCorr;
	float	lv_normalizedXCorrMax 	= 0.0;
	float   lv_captNorm, lv_refNorm;

	for (i = 0; i < SAMPLING_RATE; i++) {
		lv_xCorr = 0;

		for (j = 0; j < SAMPLING_RATE; j++) {
			lv_oldXCorr = lv_xCorr;
			lv_xCorr += lp_refSignal[j] * ap_capturedSignal[i+j];
		}

		lv_captNorm = (float)(lv_xCorr) / (float)(ap_capturedSignalNrg2[i]);
		lv_refNorm 	= (float)(lv_xCorr) / (float)(lv_nrg2Signal);
		lv_normalizedXCorr = lv_captNorm * lv_refNorm;
		if(lv_normalizedXCorr > lv_normalizedXCorrMax){
			lv_normalizedXCorrMax = lv_normalizedXCorr;
			ov_maxIndex = i;
		}
	}

	if(lv_normalizedXCorrMax > IDENTIFICATION_THRESHOLD){
		ov_estimatedTS = ov_firstSecTS + 1000 * ov_validationLevel + (SAMPLING_RATE - ov_maxIndex) / 8;
		ov_validationLevel++;
		std::cout<<"ov_validationLevel = "<<ov_validationLevel<<std::endl;
	}
	else
		ov_validationLevel = 0;

	return XCORRELEM_OK;
}