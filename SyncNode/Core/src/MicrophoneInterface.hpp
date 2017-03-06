#ifndef __MICROPHONEINTERFACE_HPP__
#define __MICROPHONEINTERFACE_HPP__
#ifdef __arm__
#include <alsa/asoundlib.h>
#endif
#include <iostream>
#include <unistd.h>

#include "Configuration.hpp"
#include "Tools.hpp"
#include "Thread.hpp"

enum MicIntEnum 		{MICINT_OK, MICINT_ERROR};

#ifdef __arm__
struct HardParams{
	snd_pcm_format_t 	ov_format;
	unsigned int 		ov_channels;
	unsigned int 		ov_rate;
} ;
#endif

class MicInterface{
#ifdef __arm__
private:
	snd_pcm_t* 			op_handle;
	struct HardParams 	ov_hardParams;
	unsigned 			ov_bufferTime; // in micro seconds
	unsigned 			ov_periodTime; // in micro seconds
	snd_pcm_uframes_t 	ov_bufferFrames;
	snd_pcm_uframes_t 	ov_periodFrames;
	snd_pcm_uframes_t	ov_bufferSize;
	size_t 				ov_bitsPerSample, ov_bitsPerFrame;
	size_t 				ov_chunkBytes;
	snd_output_t*		op_log;
	snd_pcm_stream_t 	ov_stream;
	
	int 				setParams();
	size_t 				pcmRead(u_char* ap_data, size_t av_rcount);

protected:
	snd_pcm_uframes_t 	ov_chunkSize;
	u_char*				op_audioBuffer;

public:
	virtual int 		init();
	bool 				isMicReady();
	virtual int 		getSamples();
	int					destroy();
	virtual u_char* 	getAudioBuffer();
	virtual int 		getAudioBuffSize();

	virtual~ MicInterface(){}

	MicInterface():
		ov_bufferTime(0),
		ov_periodTime(0),
		ov_periodFrames(0),
		ov_bufferFrames(0),
		ov_chunkSize(0),
		ov_bufferSize(0),
		op_audioBuffer(nullptr),
		ov_stream(SND_PCM_STREAM_CAPTURE){}
#endif
};
#ifndef __arm__
class MicSimInterface : public MicInterface, public Thread{
private:
	std::string 		ov_fileName;
	pthread_cond_t  	ov_dataReadyCond 		= PTHREAD_COND_INITIALIZER;
	pthread_mutex_t 	ov_dataReadyMutex		= PTHREAD_MUTEX_INITIALIZER;
	int 				ov_chunkSize;
	u_char*				op_audioBuffer;
	virtual void 		run();
public:
	virtual int 		init(std::string av_fileName);
	virtual int 		getSamples();
	virtual u_char* 	getAudioBuffer();
	virtual int 		getAudioBuffSize();
};
#endif
#endif