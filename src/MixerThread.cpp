/*
 * MixerThread.cpp
 *
 *  Created on: Apr 14, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "MixerThread.h"

#include <glog/logging.h>
#include <sys/select.h>
#include <unistd.h>

namespace iqurius {

class SilenceAudioBuffer : public AudioBuffer {
public:
	SilenceAudioBuffer(size_t size) : AudioBuffer(size) {
		memset(getData(), 0, size);
		setDataSize(size);
	}
private:
	DISALLOW_COPY_AND_ASSIGN(SilenceAudioBuffer);
};

static const SilenceAudioBuffer SILENCE_BUFFER(MixerThread::AUDIO_BUFFER_SIZE);
const AudioBuffer* MixerThread::SILENCE = &SILENCE_BUFFER;

MixerThread::MixerThread()
    : running_(false),
	  signal_stop_(false),
	  thread_(),
	  channel_(AUDIO_BUFFER_SIZE),
	  pcm_handle_(NULL) {
}

void MixerThread::stop() {
	if (running_) {
		signal_stop_ = true;
		pthread_join(thread_, NULL);
		running_ = false;
	}
	if (pcm_handle_) {
		snd_pcm_close(pcm_handle_);
		pcm_handle_ = NULL;
	}
}

void MixerThread::start() {
    if (!running_) {
    	int err = snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0);
    	if (err < 0) {
    		LOG(ERROR) << "Error opening pcm stream: " << snd_strerror(err);
    		pcm_handle_ = NULL;
    	} else {
    		err = snd_pcm_set_params(pcm_handle_, SND_PCM_FORMAT_S16_LE,
    				SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 0, 250000);
			if (err < 0) {
				LOG(ERROR) << "Error configuring pcm stream: " << snd_strerror(err);
				snd_pcm_close(pcm_handle_);
				pcm_handle_ = NULL;
			};
    	}
		signal_stop_ = false;
		pthread_create(&thread_, NULL, threadProc, this);
		running_ = true;
    } else {
    	LOG(WARNING) << "Playback thread already running.";
    }
}

void* MixerThread::threadProc(void *ctx) {
	MixerThread* pThis = reinterpret_cast<MixerThread*>(ctx);
	pThis->run();
	return NULL;
}

void MixerThread::run() {
    while(!signal_stop_) {
    	AudioBuffer* buffer = channel_.pullBuffer();
    	const AudioBuffer* play_buffer = buffer ? buffer : SILENCE;
    	playPcm(play_buffer->getData(), play_buffer->getDataLen());
    	if (buffer) {
			channel_.releaseBuffer(buffer);
    	}
    }
}

void MixerThread::playPcm(const uint8_t* buffer, size_t size) {
	snd_pcm_sframes_t frames;

	if (NULL == pcm_handle_) {
		return;
	}
	size /= 4;
	while (size > 0) {
		frames = snd_pcm_writei(pcm_handle_, buffer, size);
		if (frames < 0) {
			frames = snd_pcm_recover(pcm_handle_, frames, 0);
		}
		if (frames < 0) {
		    LOG (ERROR) << "snd_pcm_writei failed: " << snd_strerror(frames);
		    break;
		}
		if (frames > 0 && frames < (long)sizeof(buffer)) {
			LOG(INFO) << "Short write (expected " << size << ", wrote " << frames << ")";
		}
		size -= frames;
	}
}

} /* namespace dbus */
