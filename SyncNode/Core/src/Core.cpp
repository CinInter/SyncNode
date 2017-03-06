#include "Thread.hpp"
#include "Configuration.hpp"
#include "Tools.hpp"
#include "Log.hpp"

#include "CrossCorrelation.hpp"
#include "MicrophoneInterface.hpp"
#include "NodeJSInterface.hpp"

static pthread_cond_t  sov_launchSynchroCond 	= PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sov_launchSynchroMutex	= PTHREAD_MUTEX_INITIALIZER;

class MediaThread : public Thread{
private:
	std::string ov_fileName;
    #ifdef __arm__
	MicInterface ov_micInterface;
    #else
    MicSimInterface ov_micInterface;
    #endif
	CrossCorrelation* op_crossCorrelation;
protected:
	virtual void run(){
		op_crossCorrelation = new CrossCorrelation(ov_fileName);
		op_crossCorrelation->init();
        #ifndef __arm__
		ov_micInterface.init("lol");
        #else
        ov_micInterface.init();
        #endif
		pthread_cond_wait(&sov_launchSynchroCond, &sov_launchSynchroMutex);
		while(1){
			ov_micInterface.getSamples();
			op_crossCorrelation->process();
		}
	}
public:
	int setFileName(std::string av_fileName){
		ov_fileName = av_fileName;
		return 0;
	}
	int getTimeStamp(long &av_timeStamp){
		return op_crossCorrelation->getTimeStamp(av_timeStamp);
	}
};

int main(int argc, char* argv[]){
	NodeJSInterface 			lv_nodeJSInterface;
	MediaThread 				lv_mediaThread;
	std::vector<std::string> 	lv_reqParsing;
	std::string 				lv_readString;
	bool 						lv_isDeviceRegistered;
	enum State 					lv_state;
    int                         lv_request;

    lv_state = SYNC_ENDED;
	lv_isDeviceRegistered = verifyDevice();

    FILELog::ReportingLevel() = FILELog::FromString("DEBUG4");
    FILE* log_fd = fopen( "CProcess_logfile.txt", "w" );
    Output2FILE::Stream() = log_fd;
	while(1){
		lv_nodeJSInterface.listen();
		lv_nodeJSInterface.read(lv_readString);
        lv_reqParsing.clear();
		lv_request = parseRequest(lv_readString,lv_reqParsing);
		if(!lv_isDeviceRegistered){
            lv_nodeJSInterface.write("ERROR_DEVICE_NOT_REGISTERED");
            LOG_ERROR << "Device is not registered";
        }
        else{
            switch(lv_state){
                case SYNC_ENDED:
                	switch(lv_request){
                		LOG_INFO << "Current state: SYNC_ENDED  Current event: "<<lv_reqParsing[0];
                		case LOAD_FILE:
                            LOG_INFO << "Current state: SYNC_ENDED  Current event: "<<lv_reqParsing[0];
                			LOG_INFO << "File name : " << lv_reqParsing[1];
                			if(isFileExist(lv_reqParsing[1])){
                				lv_mediaThread.setFileName(lv_reqParsing[1]);
                				lv_nodeJSInterface.write("OK FILE_FOUND_AND_LOADED");
                				//returnValue=pthread_create( &synchronizationThread, NULL, synchronizationThreadHandle, (void*) (buffer+10));
                				//if(returnValue){
                					//logProcessP("MAIN THREAD  - LOG ERROR - main: pthread_create() return code: ",returnValue);
                					//exit(EXIT_FAILURE);
                				//}
                				LOG_INFO << "File : "<<lv_reqParsing[1]<< " is found and loaded";
                				lv_state = FILE_CHOSEN;
                			}
                			else{
                				lv_nodeJSInterface.write("ERROR FILE_NOT_FOUND");
                				LOG_ERROR << "File : "<<lv_reqParsing[1]<< " isn't found";
                			}
                		break;
                		default:
                            LOG_INFO << "Current state: SYNC_ENDED  Current event: "<<lv_reqParsing[0];
                			lv_nodeJSInterface.write("ERROR FILE_NOT_CHOSEN_YET");
                			LOG_ERROR << "File isn't chosen yet";
                		break;
                	}
                break;
				case FILE_CHOSEN:
                    switch(lv_request){
                		case LOAD_FILE:
                            LOG_INFO << "Current state: FILE_CHOSEN  Current event: "<<lv_reqParsing[0];
                			LOG_INFO << "File name : " << lv_reqParsing[1];
                			if(isFileExist(lv_reqParsing[1])){
                				//pthread_cancel(synchronizationThread);
                            	//logProcess("MAIN THREAD  - LOG WARN - main: ynchronization thread is killed");
                				lv_mediaThread.setFileName(lv_reqParsing[1]);
                				lv_nodeJSInterface.write("OK FILE_FOUND_AND_LOADED");
                				//returnValue=pthread_create( &synchronizationThread, NULL, synchronizationThreadHandle, (void*) (buffer+10));
                				//if(returnValue){
                					//logProcessP("MAIN THREAD  - LOG ERROR - main: pthread_create() return code: ",returnValue);
                					//exit(EXIT_FAILURE);
                				//}
                				LOG_INFO << "File : "<<lv_reqParsing[1]<< " is found and loaded";
                				lv_state = FILE_CHOSEN;
                			}
                			else{
                				lv_nodeJSInterface.write("ERROR FILE_NOT_FOUND");
                				LOG_ERROR << "File : "<<lv_reqParsing[1]<< " isn't found";
                			}
                		break;
                		case PLAY_FILE:
                			lv_nodeJSInterface.write("OK SYNCHRONIZATION_LAUNCHED");
                        	pthread_cond_signal(&sov_launchSynchroCond);
                        	LOG_INFO << "Starting synchronization ...";
                        	lv_state = SYNC_STARTED;
                		break;
                		case GET_TIMST:
                			lv_nodeJSInterface.write("OK 0");
                			LOG_INFO << "Handling timestamp request TS = 0";
                		break;
                		default:
                		break;
                	}
                break;
                case SYNC_STARTED:
                	LOG_INFO << "Current state: SYNC_STARTED";
                	switch(lv_request){
                		case LOAD_FILE:
                            LOG_INFO << "Current state: SYNC_ENDED  Current event: "<<lv_reqParsing[0];
                            LOG_INFO << "File name : " << lv_reqParsing[1];
                            if(isFileExist(lv_reqParsing[1])){
                                lv_mediaThread.setFileName(lv_reqParsing[1]);
                                lv_nodeJSInterface.write("OK FILE_FOUND_AND_LOADED");
                                //returnValue=pthread_create( &synchronizationThread, NULL, synchronizationThreadHandle, (void*) (buffer+10));
                                //if(returnValue){
                                    //logProcessP("MAIN THREAD  - LOG ERROR - main: pthread_create() return code: ",returnValue);
                                    //exit(EXIT_FAILURE);
                                //}
                                LOG_INFO << "File : "<<lv_reqParsing[1]<< " is found and loaded";
                                lv_state = FILE_CHOSEN;
                            }
                            else{
                                lv_nodeJSInterface.write("ERROR FILE_NOT_FOUND");
                                LOG_ERROR << "File : "<<lv_reqParsing[1]<< " isn't found";
                            }
                		break;
                		case PLAY_FILE:
                            lv_nodeJSInterface.write("ERROR THIS_REQUEST_CANNOT_BE_HANDLED_AS_FILE_IS_PLAYING");
                            LOG_ERROR << "MAIN THREAD  - LOG ERROR - main: Request not handled because the file is being play";
                		break;
                		case GET_TIMST:
                            //lv_mediaThread.getTimeStamp();
                            lv_nodeJSInterface.write("OK 2134245332");
                		break;
                		default:
                		break;
                	}
                break;
                default:
                	LOG_INFO << "Current state: UNKNOWN_STATE";
                	lv_nodeJSInterface.write("ERROR UNKNOWN_STATE");
                break;
            }
        }
        lv_nodeJSInterface.closeClient();
	}
	lv_nodeJSInterface.closeServer();
	return 0;
}