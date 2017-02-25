#include <climits>
#include <cstdlib>
#include "gtest/gtest.h"
#include "CrossCorrelation.hpp"
#include "Configuration.hpp"

#define FILE_NAME "rightA.wav"

class CrossCorrelationTest : public ::testing::Test {
 protected:
  virtual void SetUp() { }
  virtual void TearDown() { }
};

TEST_F(CrossCorrelationTest,XCorrElem_VerifyingMemoryAllocation){
  int     i,lv_size;
  FILE*   lp_file;
  int     lv_begin = 12423;
  short*  lp_speechOneMinute      = new short[SAMPLING_RATE*60];
  short*  lp_speechOneMinuteCopy  = new short[SAMPLING_RATE*60];

  lp_file = fopen ( FILE_NAME , "rb" );
  fseek (lp_file , 0 , SEEK_END);
  lv_size = ftell (lp_file);
  rewind (lp_file);

  fread (lp_speechOneMinute,2,SAMPLING_RATE*60,lp_file); 

  XCorrElem lv_xCorrElem(0);

  for(i = lv_begin;i < lv_begin + SAMPLING_RATE*5; i+=SAMPLING_RATE)
    lv_xCorrElem.addRefSignal(&lp_speechOneMinute[i],SAMPLING_RATE);

  std::memcpy(lp_speechOneMinuteCopy,lp_speechOneMinute,SAMPLING_RATE * 60 * sizeof(short));
  delete[] lp_speechOneMinute;
  std::vector<short*>* lp_refSignals = lv_xCorrElem.getRefSignals();
  fclose(lp_file);

  for(i=0;i<5;i++)
    EXPECT_EQ(0,std::memcmp((*lp_refSignals)[i], &lp_speechOneMinuteCopy[lv_begin+i*SAMPLING_RATE], SAMPLING_RATE * sizeof(short)));
  fclose(lp_file);
  //delete[] lp_speechOneMinuteCopy;
}

TEST_F(CrossCorrelationTest,XCorrElem_VerifyingEnergy2Calculation){
  int     i,j,lv_size;
  FILE*   lp_file;
  int     lv_begin = 12423;
  long    lv_nrg2;
  short*  lp_speechOneMinute = new short[SAMPLING_RATE*60];

  lp_file = fopen ( "rightA.wav" , "rb" );
  fseek (lp_file , 0 , SEEK_END);
  lv_size = ftell (lp_file);
  rewind (lp_file);

  fread (lp_speechOneMinute,2,SAMPLING_RATE*60,lp_file); 

  XCorrElem lv_xCorrElem(0);

  for(i = lv_begin;i < lv_begin + SAMPLING_RATE*5; i+=SAMPLING_RATE)
    lv_xCorrElem.addRefSignal(&lp_speechOneMinute[i],SAMPLING_RATE);

  std::vector<long>* lp_nrg2RefSignals = lv_xCorrElem.getNrg2RefSignals();

  for(i=0;i<5;i++){
    lv_nrg2 = 0;
    for(j = lv_begin + i * SAMPLING_RATE ; j < lv_begin + ( i + 1) * SAMPLING_RATE; j++){
      lv_nrg2 += lp_speechOneMinute[j] * lp_speechOneMinute[j];
    }
    EXPECT_EQ(lv_nrg2,(*lp_nrg2RefSignals)[i]);
  }
  fclose(lp_file);
  delete[] lp_speechOneMinute;
}

TEST_F(CrossCorrelationTest,XCorrElem_VerifyingProcessCapturedSignal){
  int     i,j,lv_size;
  FILE*   lp_file;
  int     lv_begin, lv_delay;
  short*  lp_speechOneMinute        = new short[SAMPLING_RATE*60];
  short*  lp_capturedOneSecWithPast = new short[SAMPLING_RATE*2];
  long*   lp_capturedNrg2           = new long[SAMPLING_RATE];
  int     lv_prevMaxInd;

  std::srand(std::time(0));
  lv_begin = 20000 + std::rand() % 10000;
  lv_delay = std::rand() % 500;
  lp_file = fopen ( "rightA.wav" , "rb" );
  fseek (lp_file , 0 , SEEK_END);
  lv_size = ftell (lp_file);
  rewind (lp_file);

  fread (lp_speechOneMinute,2,SAMPLING_RATE*60,lp_file); 

  XCorrElem lv_xCorrElem(lv_begin/8);

  for(i = lv_begin;i < lv_begin + SAMPLING_RATE*5; i+=SAMPLING_RATE)
    lv_xCorrElem.addRefSignal(&lp_speechOneMinute[i],SAMPLING_RATE);

  for(j=0;j<5;j++){
    std::memcpy(lp_capturedOneSecWithPast, &lp_speechOneMinute[lv_begin-lv_delay+SAMPLING_RATE*j], SAMPLING_RATE * 2 * sizeof(short));

    lp_capturedNrg2[0] = 0;
    for(i=0;i<SAMPLING_RATE;i++)
      lp_capturedNrg2[0] += lp_capturedOneSecWithPast[i] * lp_capturedOneSecWithPast[i];

    for(i=1;i<SAMPLING_RATE;i++)
      lp_capturedNrg2[i] =    lp_capturedNrg2[i-1] 
                            - lp_capturedOneSecWithPast[i-1] * lp_capturedOneSecWithPast[i-1]
                            + lp_capturedOneSecWithPast[i+SAMPLING_RATE-1] * lp_capturedOneSecWithPast[i+SAMPLING_RATE-1];
                      
    lv_xCorrElem.processCapturedSignal(lp_capturedOneSecWithPast,lp_capturedNrg2);
    EXPECT_EQ(lv_xCorrElem.ov_validationLevel, j+1);
    if(j)
      EXPECT_EQ(lv_prevMaxInd, lv_xCorrElem.getMaxIndex());
    else
      lv_prevMaxInd = lv_xCorrElem.getMaxIndex();
    if(j==0)
      ASSERT_TRUE(((lv_begin/8-lv_delay/8+1000)<=lv_xCorrElem.ov_estimatedTS+1)&&((lv_begin/8-lv_delay/8+1000)>lv_xCorrElem.ov_estimatedTS-1));
  }
  fclose(lp_file);
  delete[] lp_speechOneMinute;
  delete[] lp_capturedOneSecWithPast;
  delete[] lp_capturedNrg2;
}

TEST_F(CrossCorrelationTest,XCorrThread_getTimeStampInNonFoundCase){
  long lv_timeStamp;
  XCorrThread lv_xCorrThread(nullptr,nullptr,0);

  EXPECT_EQ(lv_xCorrThread.getTimeStamp(lv_timeStamp), false);
  EXPECT_EQ(lv_timeStamp, -1);
}

TEST_F(CrossCorrelationTest,XCorrThread_addRefSignalMemoryVerification){
  int     i,j,k,lv_size;
  FILE*   lp_file;
  short*  lp_speechOneMinute  = new short[SAMPLING_RATE*60*15];
  std::vector<XCorrElem>* lp_xCorrElemTable;
  std::vector<short*>*    lp_refSignalsFirstCorrElem;

  std::srand(std::time(0));

  for (i = 0; i < 8000*60*15; i++)
      lp_speechOneMinute[i] = std::rand() % 255 -127;

  XCorrThread lv_xCorrThread(nullptr,nullptr,0);
  lv_xCorrThread.addRefSignal(lp_speechOneMinute,SAMPLING_RATE*60*15,0);
  lp_xCorrElemTable = lv_xCorrThread.getXCorrElemTable();
  
  lp_refSignalsFirstCorrElem = (*lp_xCorrElemTable)[0].getRefSignals();

  k = 1;
  for(i=0;i<5;i++){
    lp_refSignalsFirstCorrElem = (*lp_xCorrElemTable)[i].getRefSignals();
    for(j=0;j<5;j++)
      EXPECT_EQ(0,std::memcmp((*lp_refSignalsFirstCorrElem)[j], &lp_speechOneMinute[SAMPLING_RATE*(30*k+j)], SAMPLING_RATE * sizeof(short)));
    k += 2;
  }
  delete[] lp_speechOneMinute;
}

/*TEST_F(CrossCorrelationTest,XCorrThread_threadingCheck){
  int     i,j,k,lv_size;
  FILE*   lp_file;
  short*  lp_capturedOneSecWithPast = new short[SAMPLING_RATE*2];
  long*   lp_capturedNrg2           = new long [SAMPLING_RATE];
  short*  lp_speechOneMinute        = new short[SAMPLING_RATE*60*5];
  long    lv_timeStamp;
  bool    lv_timeStampReady;
  std::vector<XCorrElem>* lp_xCorrElemTable;
  std::vector<short*>*    lp_refSignalsFirstCorrElem;

  std::srand(std::time(0));

  for (i = 0; i < 8000*60*5; i++)
      lp_speechOneMinute[i] = std::rand() % 255 - 127;

  XCorrThread lv_xCorrThread(lp_capturedOneSecWithPast,lp_capturedNrg2,0);
  lv_xCorrThread.addRefSignal(lp_speechOneMinute,SAMPLING_RATE*60*5,0);
  lv_xCorrThread.start();

  for(j = 0; j < 5; j++){
    std::memcpy(lp_capturedOneSecWithPast, &lp_speechOneMinute[(90+j)*8000], SAMPLING_RATE * 2 * sizeof(short));

    lp_capturedNrg2[0] = 0;
    for(i=0;i<SAMPLING_RATE;i++)
      lp_capturedNrg2[0] += lp_capturedOneSecWithPast[i] * lp_capturedOneSecWithPast[i];
    for(i=1;i<SAMPLING_RATE;i++)
      lp_capturedNrg2[i] =    lp_capturedNrg2[i-1] 
                            - lp_capturedOneSecWithPast[i-1] * lp_capturedOneSecWithPast[i-1]
                            + lp_capturedOneSecWithPast[i+SAMPLING_RATE-1] * lp_capturedOneSecWithPast[i+SAMPLING_RATE-1];

    lv_xCorrThread.runCondition();
    lv_xCorrThread.getCondition();
  }

  lv_timeStampReady = lv_xCorrThread.getTimeStamp(lv_timeStamp);
  EXPECT_EQ(lv_timeStampReady,true);
  EXPECT_EQ(lv_timeStamp,95000); 
  delete[] lp_speechOneMinute;
  delete[] lp_capturedNrg2;
  delete[] lp_capturedOneSecWithPast;
}*/

/*TEST_F(CrossCorrelationTest,CrossCorrelation){
  int     i,j,lv_size;
  FILE*   lp_file;
  long value;
  short*  lp_speechFifteenMinutes = new short[SAMPLING_RATE * 60 * 15];
  short* test;
  bool    lv_timeStampReady;
  CrossCorrelation xCorr("FifteenMinutesRandFile.raw");
  xCorr.init();
  xCorr.getCapturedSignalPointer(test);
  
  lp_file = fopen("FifteenMinutesRandFile.raw","rb");
  fread(lp_speechFifteenMinutes,2 ,SAMPLING_RATE * 60 * 15, lp_file);

  //for(j = 0; j < 5; j++){
    std::memcpy(test, &lp_speechFifteenMinutes[(90+0)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;
  //} 
    std::memcpy(test, &lp_speechFifteenMinutes[(90+1)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;

    std::memcpy(test, &lp_speechFifteenMinutes[(90+2)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;

    std::memcpy(test, &lp_speechFifteenMinutes[(90+3)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;

    std::memcpy(test, &lp_speechFifteenMinutes[(90+4)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;

    std::memcpy(test, &lp_speechFifteenMinutes[(90+5)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    lv_timeStampReady = xCorr.getTimeStamp(value);
    std::cout<<"Time Stamp = "<<value<<std::endl;

    EXPECT_EQ(lv_timeStampReady,true);
  EXPECT_EQ((value<96002)&&(value>95999),true); 
    std::cout<<"Time Stamp = "<<value<<std::endl;

  fclose(lp_file);

  delete[] lp_speechFifteenMinutes;
}*/
TEST_F(CrossCorrelationTest,CrossCorrelation){
  int     i,j,lv_size;
  FILE*   lp_file;
  long value;
  short*  lp_speechFifteenMinutes = new short[SAMPLING_RATE * 60 * 15];
  short* test;
  std::cout << "Thread # main" << "  : on CPU " << sched_getcpu() << std::endl;
  CrossCorrelation xCorr("FifteenMinutesRandFile.raw");
  xCorr.init();
  xCorr.getCapturedSignalPointer(test);
  
  lp_file = fopen("FifteenMinutesRandFile.raw","rb");
  fread(lp_speechFifteenMinutes,2 ,SAMPLING_RATE * 60 * 15, lp_file);

  //for(j = 0; j < 5; j++){
    memcpy(test, &lp_speechFifteenMinutes[(90+0)*8000], SAMPLING_RATE * sizeof(short));
    xCorr.process();
    xCorr.getTimeStamp(value);
  LOG_INFO << "op_capturedSig[90*8000] = " <<lp_speechFifteenMinutes[90*8000] << " op_capturedSig[90*8000+1] = " <<lp_speechFifteenMinutes[90*8000+1] ;
    memcpy(test, &lp_speechFifteenMinutes[(90+1)*8000], SAMPLING_RATE * sizeof(short));

    clock_t correlationStart=clock();
    xCorr.process();
    clock_t correlationEnd =clock();
    printf("Time taken: %.2f ms\n", (double)(correlationEnd-correlationStart)/CLOCKS_PER_MSEC);
    xCorr.getTimeStamp(value);
  fclose(lp_file);

  delete[] lp_speechFifteenMinutes;
}