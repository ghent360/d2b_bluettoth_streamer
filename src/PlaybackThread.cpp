/*
 * PlaybackThread.cpp
 *
 *  Created on: Nov 19, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "PlaybackThread.h"
#include "AudioMixer.h"

#include <glog/logging.h>
#include <sys/select.h>
#include <unistd.h>

namespace dbus {

PlaybackThread::PlaybackThread(Connection* connection,
    const ObjectPath& transport_path,
	iqurius::AudioChannel* audio_channel,
	int sampling_rate)
      : running_(false),
        signal_stop_(false),
        decoding_ok_(false),
        transport_(connection, transport_path),
        thread_(),
        fd_(0),
        read_mtu_(0),
        write_mtu_(0),
		sampling_rate_(sampling_rate),
		audio_channel_(audio_channel),
		audo_buffer_len_(0),
		resampler_(nullptr),
		preroll_filled_(0),
		in_preroll_(false) {
  audo_channel_buffer_ = new uint8_t[iqurius::AudioMixer::AUDIO_BUFFER_SIZE];
  if (sampling_rate_ != 44100) {
      soxr_error_t error;
      soxr_io_spec_t io_spec = soxr_io_spec(SOXR_INT16_I, SOXR_INT16_I);
      soxr_quality_spec_t q_spec = soxr_quality_spec(SOXR_HQ, 0);
      resampler_ = soxr_create(sampling_rate_,
                               44100,
							   2,  // Number of channels
							   &error,
							   &io_spec,
							   &q_spec,
							   NULL);

  }
  for (int idx = 0; idx < preroll_size_; idx++) {
	  preroll_[idx] = nullptr;
  }
}

void PlaybackThread::freeTransport() {
  if (fd_) {
    transport_.release("rw");
    close(fd_);
    fd_ = 0;
    read_mtu_ = 0;
    write_mtu_ = 0;
  }
}

bool PlaybackThread::acqureTransport() {
  freeTransport();
  return transport_.acquire("rw", &fd_, &read_mtu_, &write_mtu_);
}

void PlaybackThread::stop() {
  if (running_) {
    signal_stop_ = true;
    pthread_join(thread_, NULL);
    flush();
    running_ = false;
  }
  freeTransport();
}

void PlaybackThread::start() {
  if (ok()) {
    LOG(WARNING) << "Playback thread already running.";
    return;
  }
  stop();
  if (acqureTransport()) {
    signal_stop_ = false;
    decoding_ok_ = true;
    preroll_filled_ = 0;
    for (int idx = 0; idx < preroll_size_; idx++) {
  	  preroll_[idx] = nullptr;
    }
    in_preroll_ = true;
    pthread_create(&thread_, NULL, threadProc, this);
    running_ = true;
  }
}

void* PlaybackThread::threadProc(void *ctx) {
  PlaybackThread* pThis = reinterpret_cast<PlaybackThread*>(ctx);
  pThis->run();
  return NULL;
}

void PlaybackThread::run() {
  uint8_t* read_buffer = new uint8_t[read_mtu_];
  audo_buffer_len_ = 0;
  while(!signal_stop_) {
    int len;
    fd_set readset;
    struct timeval timeout;

    FD_ZERO(&readset);
    FD_SET(fd_, &readset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;  // 100ms
    len = select(fd_ + 1, &readset, NULL, NULL, &timeout);
    if (len < 0) {
      LOG(ERROR) << "select FD " << fd_ << " error = " << errno;
      break;
    }
    if (len == 0) {
      continue;
    }

    len = read(fd_, read_buffer, read_mtu_);
    if (len > 0) {
      decode(read_buffer, len);
    } else if (errno != EAGAIN) {
      LOG(ERROR) << "read FD " << fd_ << " error = " << errno;
      break;
    }
  }
  delete [] read_buffer;
  decoding_ok_ = false;
}

iqurius::AudioBuffer* PlaybackThread::waitForFreeBuffer() {
  iqurius::AudioBuffer* audio_buffer;
  do {
	audio_buffer = audio_channel_->getFreeBuffer();
	if (audio_buffer) break;
	usleep(1000);
  } while (!signal_stop_);
  return audio_buffer;
}

void PlaybackThread::playPcm(const uint8_t* buffer, size_t size) {
  while ((size / 4) > 0) {
	if (!resampler_) {
		if (audo_buffer_len_ + size > iqurius::AudioMixer::AUDIO_BUFFER_SIZE) {
		  iqurius::AudioBuffer* audio_buffer = waitForFreeBuffer();
		  if (!audio_buffer) return;
		  if (audo_buffer_len_) {
			audio_buffer->write(audo_channel_buffer_, audo_buffer_len_);
			audo_buffer_len_ = 0;
		  }
		  size_t written = audio_buffer->write(buffer, size);
		  postBuffer(audio_buffer);
		  buffer += written;
		  size -= written;
		} else {
		  memcpy(audo_channel_buffer_ + audo_buffer_len_, buffer, size);
		  audo_buffer_len_ += size;
		  size = 0;
		}
	} else {
		soxr_error_t error;
		size_t input_consumed;
		size_t output_written;
		error = soxr_process(resampler_,
				             buffer,
							 size / 4,
							 &input_consumed,
							 audo_channel_buffer_ + audo_buffer_len_,
							 (iqurius::AudioMixer::AUDIO_BUFFER_SIZE - audo_buffer_len_) / 4,
							 &output_written);
		audo_buffer_len_ += output_written * 4;
		if (audo_buffer_len_ == iqurius::AudioMixer::AUDIO_BUFFER_SIZE) {
			iqurius::AudioBuffer* audio_buffer = waitForFreeBuffer();
			if (!audio_buffer) return;
			audio_buffer->write(audo_channel_buffer_, audo_buffer_len_);
			postBuffer(audio_buffer);
			audo_buffer_len_ = 0;
		}
		buffer += input_consumed * 4;
		size -= input_consumed * 4;
	}
  }
}

void PlaybackThread::postBuffer(iqurius::AudioBuffer* audio_buffer) {
  if (!in_preroll_) {
	audio_channel_->postBuffer(audio_buffer);
	return;
  }
  if (preroll_filled_ < preroll_size_) {
	preroll_[preroll_filled_++] = audio_buffer;
	return;
  }
  in_preroll_ = false;
  for (int idx = 0; idx < preroll_size_; idx++) {
	audio_channel_->postBuffer(preroll_[idx]);
	preroll_[idx] = nullptr;
  }
  preroll_filled_ = 0;
  audio_channel_->postBuffer(audio_buffer);
}

void PlaybackThread::flush() {
  if (in_preroll_) {
    for (int idx = 0; idx < preroll_filled_; idx++) {
	  if (preroll_[idx]) {
	    audio_channel_->postBuffer(preroll_[idx]);
	    preroll_[idx] = nullptr;
	  }
    }
    preroll_filled_ = 0;
  }

  if (!resampler_) {
	iqurius::AudioBuffer* audio_buffer = waitForFreeBuffer();
	if (!audio_buffer) return;
	if (audo_buffer_len_) {
	  audio_buffer->write(audo_channel_buffer_, audo_buffer_len_);
	  audo_buffer_len_ = 0;
	}
    audio_channel_->postBuffer(audio_buffer);
  } else {
	soxr_error_t error;
	size_t input_consumed;
	size_t output_written;
	error = soxr_process(resampler_,
						 nullptr,
						 0,
						 &input_consumed,
						 audo_channel_buffer_ + audo_buffer_len_,
						 (iqurius::AudioMixer::AUDIO_BUFFER_SIZE - audo_buffer_len_) / 4,
						 &output_written);
	audo_buffer_len_ += output_written * 4;
	iqurius::AudioBuffer* audio_buffer = waitForFreeBuffer();
	if (!audio_buffer) return;
	audio_buffer->write(audo_channel_buffer_, audo_buffer_len_);
	audio_channel_->postBuffer(audio_buffer);
	audo_buffer_len_ = 0;
  }
}
} /* namespace dbus */
