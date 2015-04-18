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
	iqurius::AudioChannel* audio_channel)
      : running_(false),
        signal_stop_(false),
        transport_(connection, transport_path),
        thread_(),
        fd_(0),
        read_mtu_(0),
        write_mtu_(0),
		audio_channel_(audio_channel),
		audo_leftover_len_(0) {
  audo_leftover_buffer_ = new uint8_t[iqurius::AudioMixer::AUDIO_BUFFER_SIZE];
}

void PlaybackThread::stop() {
  if (running_) {
    signal_stop_ = true;
    pthread_join(thread_, NULL);
    running_ = false;
  }
  if (fd_) {
    transport_.release("rw");
    close(fd_);
    fd_ = 0;
    read_mtu_ = 0;
    write_mtu_ = 0;
  }
}

void PlaybackThread::start() {
  if (running_) {
    LOG(WARNING) << "Playback thread already running.";
    return;
  }
  if (fd_) {
    transport_.release("rw");
    close(fd_);
    fd_ = 0;
    read_mtu_ = 0;
    write_mtu_ = 0;
  }
  if (transport_.acquire("rw", &fd_, &read_mtu_, &write_mtu_)) {
    signal_stop_ = false;
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
  audo_leftover_len_ = 0;
  while(!signal_stop_) {
    int len;
    fd_set readset;
    struct timeval timeout;

    FD_ZERO(&readset);
    FD_SET(fd_, &readset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;  // 100ms
    len = select(fd_ + 1, &readset, NULL, NULL, &timeout);
    if (len <= 0) continue;

    len = read(fd_, read_buffer, read_mtu_);
    if (len > 0) {
      decode(read_buffer, len);
    } else if (errno != EAGAIN) {
      LOG(ERROR) << "FD " << fd_ << " error = " << errno;
    }
  }
  delete [] read_buffer;
}

iqurius::AudioBuffer* PlaybackThread::waitForFreeBuffer() {
	iqurius::AudioBuffer* audio_buffer;

  do {
	audio_buffer = audio_channel_->getFreeBuffer();
	if (audio_buffer) {
		return audio_buffer;
	}
    usleep(1000);
  } while (!signal_stop_);
  return nullptr;
}

void PlaybackThread::playPcm(const uint8_t* buffer, size_t size) {
  while (size > 0) {
    if (audo_leftover_len_ + size > iqurius::AudioMixer::AUDIO_BUFFER_SIZE) {
      iqurius::AudioBuffer* audio_buffer = waitForFreeBuffer();
      if (!audio_buffer) return;
      if (audo_leftover_len_) {
        audio_buffer->write(audo_leftover_buffer_, audo_leftover_len_);
        audo_leftover_len_ = 0;
      }
      size_t written = audio_buffer->write(buffer, size);
      audio_channel_->postBuffer(audio_buffer);
      buffer += written;
      size -= written;
    } else {
      memcpy(audo_leftover_buffer_ + audo_leftover_len_, buffer, size);
      audo_leftover_len_ += size;
      size = 0;
    }
  }
}

} /* namespace dbus */
