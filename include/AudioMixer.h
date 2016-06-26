/*
 * AudioMixer.h
 *
 *  Created on: Apr 14, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef AUDIOMIXER_H_
#define AUDIOMIXER_H_

#include "util.h"

#include <alsa/asoundlib.h>
#include <glog/logging.h>
#include <pthread.h>

namespace iqurius {

template<class T, std::size_t sz>
class FFRingBuffer {
public:
	FFRingBuffer() : head_(0), tail_(0) {
	  for(std::size_t idx = 0; idx < sz; ++idx)
	  {
		  buffer_[idx] = nullptr;
	  }
	}

	bool enqueue(T* value) {
	  if (nullptr != buffer_[head_]) {
		 return false;
	  }
	  buffer_[head_] = value;
	  head_ = NEXT(head_);
	  return true;
	}

	bool dequeue(T** value) {
      *value = buffer_[tail_];
      if (nullptr == *value) {
    	  return false;
      }
      buffer_[tail_] = nullptr;
      tail_ = NEXT(tail_);
	  return true;
	}
private:
	static std::size_t NEXT(std::size_t i) {
		return (i + 1) % sz;
	}

	std::size_t head_;
	std::size_t tail_;
	T* buffer_[sz];
	DISALLOW_COPY_AND_ASSIGN(FFRingBuffer);
};

class AudioBuffer {
public:
	AudioBuffer(size_t size) : size_(size), data_len_(0) {
		buffer_ = new uint8_t[size];
	}
	virtual ~AudioBuffer() { delete [] buffer_; }

	void reset() { data_len_ = 0; }

	size_t getSize() const { return size_; }
	size_t getDataLen() const { return data_len_; }

	uint8_t* getData() { return buffer_; }
	const uint8_t* getData() const { return buffer_; }
	void setDataSize(size_t data_len) { data_len_ = data_len; }

	size_t write(const uint8_t* data, size_t len) {
		if (len > size_ - data_len_) {
			len = size_ - data_len_;
		}
		if (len) {
		  memcpy(buffer_ + data_len_, data, len);
		  data_len_ += len;
		}
		return len;
	}

private:
	const size_t size_;
	size_t data_len_;
	uint8_t* buffer_;

	DISALLOW_COPY_AND_ASSIGN(AudioBuffer);
};

class AudioChannel {
public:
	static constexpr size_t NUM_AUDIO_BUFFERS = 10;

	AudioChannel(size_t audio_buffer_size) : volume_(0x100), idle_(true) {
	  for (size_t idx = 0; idx < NUM_AUDIO_BUFFERS; ++idx) {
		AudioBuffer* audio_buffer = new AudioBuffer(audio_buffer_size);
		free_audio_buffers_.enqueue(audio_buffer);
	  }
	}

	~AudioChannel() {
	  size_t num_buffers = 0;
	  AudioBuffer* audio_buffer = nullptr;
	  while (free_audio_buffers_.dequeue(&audio_buffer)) {
		  delete audio_buffer;
		  num_buffers++;
	  }
	  while (audio_buffers_.dequeue(&audio_buffer)) {
		  delete audio_buffer;
		  num_buffers++;
	  }
	  if (num_buffers != NUM_AUDIO_BUFFERS) {
		LOG(ERROR) << (NUM_AUDIO_BUFFERS - num_buffers)
				<< " audio buffers leaked.";
	  }
	}

	AudioBuffer* getFreeBuffer() {
	  AudioBuffer* audio_buffer = nullptr;
	  free_audio_buffers_.dequeue(&audio_buffer);
	  return audio_buffer;
	}

	void releaseBuffer(AudioBuffer* audio_buffer) {
	  if (audio_buffer == nullptr) {
		LOG(ERROR) << "Attempting to return a null buffer.";
		return;
	  }
	  audio_buffer->reset();
	  if (!free_audio_buffers_.enqueue(audio_buffer)) {
		LOG(ERROR) << "Unable to return buffer (channel mismatch?).";
	  }
	}

	AudioBuffer* pullBuffer() {
	  AudioBuffer* audio_buffer = nullptr;
	  audio_buffers_.dequeue(&audio_buffer);
	  idle_ = audio_buffer == nullptr;
      return audio_buffer;
	}

	int16_t getVolume() const { return volume_; }
	int16_t setVolume(int16_t value);
	float setVolume(float value);

	bool postBuffer(AudioBuffer* audio_buffer) {
	  if (audio_buffer == nullptr) {
		LOG(ERROR) << "Attempting to post a null buffer.";
		return false;
	  }
	  idle_ = false;
	  return audio_buffers_.enqueue(audio_buffer);
	}

	void waitForIdle() {
	  int max_wait_cnt = 0;
	  while (!idle_ && max_wait_cnt < 10) {  // Max wait 1s
		usleep(100000);  // 100ms
		max_wait_cnt++;
	  }
	}

private:
	FFRingBuffer<AudioBuffer, NUM_AUDIO_BUFFERS> free_audio_buffers_;
	FFRingBuffer<AudioBuffer, NUM_AUDIO_BUFFERS> audio_buffers_;
	int16_t volume_;
	volatile bool idle_;
	DISALLOW_COPY_AND_ASSIGN(AudioChannel);
};

class AudioMixer {
public:
	AudioMixer(size_t num_channels);
	~AudioMixer();

	void start();
	void stop();
	void waitForIdle();

	AudioChannel* getAudioChannel(size_t channel_no);

	static constexpr size_t AUDIO_BUFFER_SIZE = 4*4410;  // 100ms
protected:
	void playPcm(const uint8_t* buffer, size_t size);

private:
	static const AudioBuffer* SILENCE;

	static void* threadProc(void *);
	void run();

	bool running_;
	bool signal_stop_;
	volatile bool idle_;
	pthread_t thread_;

	size_t num_channels_;
	AudioChannel** channels_;
	snd_pcm_t *pcm_handle_;
	int dbg_handle_;
	DISALLOW_COPY_AND_ASSIGN(AudioMixer);
};

} /* namespace iqurius */

#endif /* AUDIOMIXER_H_ */
