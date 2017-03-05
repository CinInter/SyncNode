#include <climits>
#include <cstdlib>
#include "gtest/gtest.h"
#include "Configuration.hpp"
#include "Tools.hpp"

#include "MicrophoneInterface.hpp"

class MicrophoneInterfaceTest : public ::testing::Test {
 protected:
  virtual void SetUp() { 
  	
  }
  virtual void TearDown() { }
};

TEST_F(MicrophoneInterfaceTest,MicrophoneInterfaceTestRecording){
	FILE* lp_file;

	lp_file = fopen("file.binary", "wb");
	MicInterface lv_micInterface;
	lv_micInterface.init();
	usleep(10000);
	#ifdef __arm__
	while(1){
		lv_micInterface.getSamples();
		fwrite(lv_micInterface.getAudioBuffer(), 2, lv_micInterface.getAudioBuffSize(), lp_file);
	}
	#endif
	fclose(lp_file);
}

TEST_F(MicrophoneInterfaceTest,MicrophoneSimulationInterfaceTestRecording){
	FILE* lp_file;

	lp_file = fopen("FifteenMinutesRandFile.binary", "wb");
	MicSimInterface lv_micSimInterface;
	lv_micSimInterface.init("FifteenMinutesRandFile.raw");
	usleep(10000);
	#ifdef __arm__
	while(1){
		std::cout << "time = " <<std::time(0) << std::endl;
		lv_micSimInterface.getSamples();
		fwrite(lv_micSimInterface.getAudioBuffer(), 2, lv_micSimInterface.getAudioBuffSize(), lp_file);
	}
	#endif
	fclose(lp_file);
}