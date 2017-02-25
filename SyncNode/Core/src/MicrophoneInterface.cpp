#include "MicrophoneInterface.hpp"


int MicInterface::init(){
	#ifdef __arm__
	int lv_retValue;
	char* lp_pcmName = (char *)PCM_NAME;
	snd_pcm_info_t* lp_info;

	snd_output_stdio_attach(&op_log, stderr, 0);

	lv_retValue = snd_pcm_open(&op_handle, lp_pcmName, ov_stream, 0);
	if (lv_retValue < 0) {
		LOG_ERROR << "Error in opening audio device";
		return MICINT_ERROR;
	}

	snd_pcm_info_alloca(&lp_info);
	lv_retValue = snd_pcm_info(op_handle, lp_info);
	if(lv_retValue <0){
		LOG_ERROR <<"Info error";
		return MICINT_ERROR;
	}

    ov_hardParams.ov_format 	= SND_PCM_FORMAT_S16_LE;
    ov_hardParams.ov_rate 		= SAMPLING_RATE;
    ov_hardParams.ov_channels 	= CHANNELS_NUM;

    setParams();
	#endif
	return MICINT_OK;
}

#ifdef __arm__
int MicInterface::setParams(){
	int lv_retValue, lv_rate;
	snd_pcm_hw_params_t* lp_hardwareParams;
	snd_pcm_sw_params_t* lp_softwareParams;

	// Allocate params structures
	snd_pcm_hw_params_alloca(&lp_hardwareParams);
	snd_pcm_sw_params_alloca(&lp_softwareParams);

	// Check device
	lv_retValue = snd_pcm_hw_params_any(op_handle, lp_hardwareParams);
	if (lv_retValue < 0) {
		LOG_ERROR << "Broken configuration for this PCM: no configurations available";
		return MICINT_ERROR;
	}
	else{
		lv_retValue = snd_pcm_hw_params_set_access(op_handle, lp_hardwareParams, SND_PCM_ACCESS_RW_INTERLEAVED);
		if (lv_retValue < 0) {
			LOG_ERROR << "Access type not available";
			return MICINT_ERROR;
		}
	}
	
	// Check format
	lv_retValue = snd_pcm_hw_params_set_format(op_handle, lp_hardwareParams, ov_hardParams.ov_format);
	if (lv_retValue < 0) {
		LOG_ERROR << "Sample format non available";
		return MICINT_ERROR;
	}

	// Check channel count
	lv_retValue = snd_pcm_hw_params_set_channels(op_handle, lp_hardwareParams, ov_hardParams.ov_channels);
	if (lv_retValue < 0) {
		LOG_ERROR << "Channels count non available";
		return MICINT_ERROR;
	}

	// Check rate
	lv_rate = ov_hardParams.ov_rate;
	lv_retValue = snd_pcm_hw_params_set_rate_near(op_handle, lp_hardwareParams, &ov_hardParams.ov_rate, 0);
	if ((float)lv_rate * 1.05 < ov_hardParams.ov_rate || (float)lv_rate * 0.95 > ov_hardParams.ov_rate) {
		LOG_ERROR << "rate is not accurate";
		return MICINT_ERROR;
	}

	// Define buffer time and frames number
	if (ov_bufferTime == 0 && ov_bufferFrames == 0) {
		lv_retValue = snd_pcm_hw_params_get_buffer_time_max(lp_hardwareParams, &ov_bufferTime, 0);
		if (lv_retValue < 0) {
			LOG_ERROR << "Error in getting buffer time max";
			return MICINT_ERROR;
		}
	if (ov_bufferTime > FOUR_SEC_BUFF_TIME_IN_MICRO_SEC)
		ov_bufferTime = FOUR_SEC_BUFF_TIME_IN_MICRO_SEC;
	}

	// Define period time and frames number
	if (ov_periodTime == 0 && ov_periodFrames == 0) {
		if (ov_bufferTime > 0)
			ov_periodTime = ov_bufferTime / 4;
		else
			ov_periodFrames = ov_periodFrames / 4;
	}
	
	// Validateperiod and buffer, time and frames number
	if (ov_periodTime > 0)
		lv_retValue = snd_pcm_hw_params_set_period_time_near(op_handle, lp_hardwareParams, &ov_periodTime, 0);
	else
		lv_retValue = snd_pcm_hw_params_set_period_size_near(op_handle, lp_hardwareParams, &ov_periodFrames, 0);
	
	if (ov_bufferTime > 0) 
		lv_retValue = snd_pcm_hw_params_set_buffer_time_near(op_handle, lp_hardwareParams, &ov_bufferTime, 0);
	else
		lv_retValue = snd_pcm_hw_params_set_buffer_size_near(op_handle, lp_hardwareParams, &ov_bufferFrames);

	// Validate hardware parameters
	lv_retValue = snd_pcm_hw_params(op_handle, lp_hardwareParams);
	if (lv_retValue < 0) {
		LOG_ERROR << "Unable to install hw params";
		return MICINT_ERROR;
	}

	// Get buffer and chunck size
	snd_pcm_hw_params_get_period_size(lp_hardwareParams, &ov_chunkSize, 0);
	snd_pcm_hw_params_get_buffer_size(lp_hardwareParams, &ov_bufferSize);

	// Get software parameters
	snd_pcm_sw_params_current(op_handle, lp_softwareParams);

	// Set minimum availibility
	snd_pcm_sw_params_set_avail_min(op_handle, lp_softwareParams, ov_chunkSize);

	// Set thresholds
	snd_pcm_sw_params_set_start_threshold(op_handle, lp_softwareParams, 1);
	snd_pcm_sw_params_set_stop_threshold(op_handle, lp_softwareParams, ov_bufferSize);

	// Validate software parameters
	lv_retValue = snd_pcm_sw_params(op_handle, lp_softwareParams);
	if (lv_retValue < 0) {
		LOG_ERROR << "Unable to install sw params";
		return MICINT_ERROR;
	}

	//Allcate audio buffer
	ov_bitsPerSample = snd_pcm_format_physical_width(ov_hardParams.ov_format);
	ov_bitsPerFrame = ov_bitsPerSample * ov_hardParams.ov_channels;
	ov_chunkBytes = ov_chunkSize * ov_bitsPerFrame / 8;
	op_audioBuffer = (u_char*)malloc(ov_chunkBytes);

	if (op_audioBuffer == nullptr) {
		LOG_ERROR << "not enough memory";
		return MICINT_ERROR;
	}

	ov_bufferFrames = ov_bufferSize;

	return MICINT_OK;
}

size_t MicInterface::pcmRead(u_char* ap_data, size_t av_count)
{
	size_t lv_readBytesCount;
	size_t lv_count = av_count;

	if (lv_count != ov_chunkSize) {
		lv_count = ov_chunkSize;
	}

	while (lv_count > 0) {
		lv_readBytesCount = snd_pcm_readi(op_handle, ap_data, lv_count);
		if (lv_readBytesCount == -EAGAIN || (lv_readBytesCount >= 0 && (size_t)lv_readBytesCount < lv_count)) {
			snd_pcm_wait(op_handle, 1000);
		} else if (lv_readBytesCount == -EPIPE) {
			LOG_ERROR << "Overrun error occured";
		} else if (lv_readBytesCount == -ESTRPIPE) {
			LOG_ERROR << "Device is suspended";
		} else if (lv_readBytesCount < 0) {
			LOG_ERROR << "Problem occure";
			exit(0);
		}
		if (lv_readBytesCount > 0) {
			lv_count -= lv_readBytesCount;
			av_count += lv_readBytesCount * ov_bitsPerFrame / 8;
		}
	}
	return lv_count;
}
#endif