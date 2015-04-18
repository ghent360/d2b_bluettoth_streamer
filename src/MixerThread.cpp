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

int16_t AudioChannel::setVolume(int16_t value) {
  int16_t old_volume = volume_;
  if (value >= 0) {
	volume_ = value;
  }
  return old_volume;
}

float AudioChannel::setVolume(float value) {
  int16_t old_volume = setVolume((int16_t)(value * 0x7fff));
  return ((double)old_volume) / (double)0x7fff;
}

MixerThread::MixerThread(size_t num_channels)
    : running_(false),
	  signal_stop_(false),
	  thread_(),
	  num_channels_(num_channels),
	  channels_(nullptr),
	  pcm_handle_(nullptr) {
  CHECK(num_channels > 0);
  channels_ = new AudioChannel*[num_channels_];
  for (size_t idx = 0; idx < num_channels_; ++idx) {
	channels_[idx] = new AudioChannel(AUDIO_BUFFER_SIZE);
  }
}

MixerThread::~MixerThread() {
  stop();
  if (channels_) {
	for (size_t idx = 0; idx < num_channels_; ++idx) {
	  delete channels_[idx];
	  channels_[idx] = nullptr;
	}
  }
  delete [] channels_;
}

AudioChannel* MixerThread::getAudioChannel(size_t channel_no) {
  if (channel_no < num_channels_) {
	return channels_[channel_no];
  }
  return nullptr;
}

void MixerThread::stop() {
  if (running_) {
	signal_stop_ = true;
	pthread_join(thread_, nullptr);
	running_ = false;
  }
  if (pcm_handle_) {
	snd_pcm_close(pcm_handle_);
	pcm_handle_ = nullptr;
  }
}

void MixerThread::start() {
  if (running_) {
 	LOG(WARNING) << "Playback thread already running.";
 	return;
  }
  if (pcm_handle_) {
    snd_pcm_close(pcm_handle_);
    pcm_handle_ = nullptr;
  }
  int err = snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0) {
    LOG(ERROR) << "Error opening pcm stream: " << snd_strerror(err);
    pcm_handle_ = nullptr;
  } else {
    err = snd_pcm_set_params(pcm_handle_,
  		    SND_PCM_FORMAT_S16_LE,
	        SND_PCM_ACCESS_RW_INTERLEAVED,
			2,
			44100,
			0,
			250000);
	if (err < 0) {
	  LOG(ERROR) << "Error configuring pcm stream: " << snd_strerror(err);
	  snd_pcm_close(pcm_handle_);
	  pcm_handle_ = nullptr;
	} else {
	  signal_stop_ = false;
	  pthread_create(&thread_, NULL, threadProc, this);
	  running_ = true;
	}
  }
}

void* MixerThread::threadProc(void *ctx) {
  MixerThread* pThis = reinterpret_cast<MixerThread*>(ctx);
  pThis->run();
  return NULL;
}

void MixerThread::run() {
  AudioBuffer** mix_list = new AudioBuffer*[num_channels_];
  AudioChannel** mix_channel_owner = new AudioChannel*[num_channels_];
  AudioBuffer mix_buffer(AUDIO_BUFFER_SIZE);
  constexpr size_t mix_buffer_len = AUDIO_BUFFER_SIZE / 2;
  mix_buffer.setDataSize(mix_buffer_len * 2);

  while(!signal_stop_) {
	size_t num_mix_channels = 0;
	for (size_t idx = 0; idx < num_channels_; ++idx) {
   	  mix_channel_owner[num_mix_channels] = channels_[idx];
      mix_list[num_mix_channels] = channels_[idx]->pullBuffer();
      if (mix_list[num_mix_channels]) {
    	num_mix_channels++;
      }
	}
	const AudioBuffer* play_buffer;
	if (num_mix_channels == 0) {
	  play_buffer = SILENCE;
	} else if (num_mix_channels == 1) {
	  int16_t* mixed_samples = (int16_t *)mix_buffer.getData();
      int16_t* buffer_samples = (int16_t *)mix_list[0]->getData();
	  size_t buffer_len = mix_list[0]->getDataLen() / 2;
	  int16_t channel_volume = mix_channel_owner[0]->getVolume();
	  int sample_no;

	  for (sample_no = 0; sample_no < buffer_len; ++sample_no) {
		int32_t sample = ((int32_t)buffer_samples[sample_no] * channel_volume)
			/ 0x7fff;
		mixed_samples[sample_no] = (int16_t)sample;
	  }
	  for (; sample_no < mix_buffer_len; ++sample_no) {
		mixed_samples[sample_no] = 0;
	  }
	  play_buffer = &mix_buffer;
	} else {
	  int16_t* mixed_samples = (int16_t *)mix_buffer.getData();
	  for (int sample_no = 0; sample_no < mix_buffer_len; ++sample_no) {
        int32_t sample = 0;
        for (int buffer_idx = 0; buffer_idx < num_mix_channels; ++buffer_idx) {
          int16_t* buffer_samples = (int16_t *)mix_list[buffer_idx]->getData();
          size_t buffer_len = mix_list[buffer_idx]->getDataLen() / 2;
          int16_t channel_volume = mix_channel_owner[buffer_idx]->getVolume();

          if (sample_no < buffer_len) {
        	sample += ((int32_t)buffer_samples[sample_no] * channel_volume)
        			/ 0x7fff;
          }
        }
        if (sample > 0x7fff) {
          sample = 0x7fff;
        } else if (sample < -0x7fff) {
          sample = -0x7fff;
        }
        mixed_samples[sample_no] = (int16_t)sample;
	  }
	  play_buffer = &mix_buffer;
	}
   	playPcm(play_buffer->getData(), play_buffer->getDataLen());
	for (size_t idx = 0; idx < num_mix_channels; ++idx) {
	  mix_channel_owner[idx]->releaseBuffer(mix_list[idx]);
   	}
  }
  delete [] mix_channel_owner;
  delete [] mix_list;
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
	size -= frames;
  }
}

} /* namespace dbus */
