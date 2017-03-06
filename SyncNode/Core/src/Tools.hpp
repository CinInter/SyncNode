#ifndef __TOOLS_H__
#define __TOOLS_H__
#include "Log.hpp"
#include "Configuration.hpp"

#include <vector>
#include <sys/stat.h>
#include <string>

#define LOG_ERROR 	FILE_LOG(logERROR)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_WARNING FILE_LOG(logWARNING)<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_INFO 	FILE_LOG(logINFO)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_DEBUG 	FILE_LOG(logDEBUG)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_DEBUG1 	FILE_LOG(logDEBUG1)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_DEBUG2 	FILE_LOG(logDEBUG2)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_DEBUG3 	FILE_LOG(logDEBUG3)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "
#define LOG_DEBUG4 	FILE_LOG(logDEBUG4)	<< "("<<__FILE__  << ":"<<__LINE__<<") - "

enum State      {SYNC_ENDED,FILE_CHOSEN,SYNC_STARTED};
enum ReqType    {LOAD_FILE,PLAY_FILE,GET_TIMST,EXIT_PROG};

static void  getMacAddress(std::string &av_macAddr){
    FILE* 	lp_cmd;
    char 	lp_tmpBuff[25];
    #ifdef __arm__
    lp_cmd = popen("/sbin/ifconfig | grep eth0 -m1 | cut -d' ' -f 11", "r");
    #else
    lp_cmd = popen("/sbin/ifconfig | grep ether -m1 | cut -d' ' -f 2", "r");
    #endif
    if (lp_cmd == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    while(fgets(lp_tmpBuff, sizeof(lp_tmpBuff), lp_cmd)){ }
   	av_macAddr.assign(lp_tmpBuff,17);
    pclose(lp_cmd);
}

static bool verifyDevice(){
    std::string lv_macAddr;
    getMacAddress(lv_macAddr);
    if(lv_macAddr.compare(DEVICE_MAC_ADDRESS)==0)
        return true;
    else
        return false;
}

static int isFileExist (const std::string &av_fileName) {
  struct stat lp_buffer;   
  return (stat (av_fileName.c_str(), &lp_buffer) == 0);
}

static int parseRequest(std::string av_request, std::vector<std::string> &av_results){
    std::string lv_buffer;
    std::stringstream lv_stringStream (av_request);

    while (lv_stringStream >> lv_buffer)
        av_results.push_back(lv_buffer);

    if(av_results[0].compare("LOAD_FILE")==0)
        return LOAD_FILE;
    else if(av_results[0].compare("PLAY_FILE")==0)
        return PLAY_FILE;
    else if(av_results[0].compare("GET_TIMST")==0)
        return GET_TIMST;
    return -1;
}
#endif