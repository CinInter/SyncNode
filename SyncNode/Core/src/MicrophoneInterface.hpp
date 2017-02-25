#ifndef __MICROPHONEINTERFACE_HPP__
#define __MICROPHONEINTERFACE_HPP__
#include <alsa/asoundlib.h>
#include "Configuration.hpp"
#include "Tools.hpp"

enum MicIntEnum 		{MICINT_OK, MICINT_ERROR};

#ifdef __arm__
struct HardParams{
	snd_pcm_format_t 	ov_format;
	unsigned int 		ov_channels;
	unsigned int 		ov_rate;
} ;
#endif

class MicInterface{
private:
	#ifdef __arm__
	snd_pcm_t* 			op_handle;
	struct HardParams 	ov_hardParams;
	unsigned int 		ov_bufferTime; // in micro seconds
	unsigned int 		ov_periodTime; // in micro seconds
	snd_pcm_uframes_t 	ov_bufferFrames;
	snd_pcm_uframes_t 	ov_periodFrames;
	snd_pcm_uframes_t 	ov_chunkSize;
	snd_pcm_uframes_t	ov_bufferSize;
	size_t 				ov_bitsPerSample, ov_bitsPerFrame;
	size_t 				ov_chunkBytes;
	u_char*				op_audioBuffer;
	snd_output_t*		op_log;
	snd_pcm_stream_t 	ov_stream ;
	int setParams();
	size_t pcmRead(u_char* ap_data, size_t av_rcount);
	#endif
public:
	int 	init();
	bool 	isMicReady();
	int 	getSamples();
	int		destroy();

	virtual~ MicInterface(){}
	#ifdef __arm__
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
#endif