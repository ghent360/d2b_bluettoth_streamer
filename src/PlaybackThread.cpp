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

#include <glog/logging.h>
#include <sys/select.h>
#include <unistd.h>

namespace dbus {

PlaybackThread::PlaybackThread(Connection* connection,
		const ObjectPath& transport_path)
    : running_(false),
	  signal_stop_(false),
	  transport_(connection, transport_path),
	  thread_(),
	  fd_(0),
	  read_mtu_(0),
	  write_mtu_(0),
	  pcm_handle_(NULL) {
}

void PlaybackThread::stop() {
	if (running_) {
		signal_stop_ = true;
		pthread_join(thread_, NULL);
		transport_.release("rw");
		close(fd_);
		fd_ = 0;
		running_ = false;
	}
	if (pcm_handle_) {
		snd_pcm_close(pcm_handle_);
		pcm_handle_ = NULL;
	}
}

void PlaybackThread::setPcmParams() {
}

void PlaybackThread::start() {
    if (!running_) {
  	    fd_ = 0;
  	    read_mtu_ = 0;
  	    write_mtu_ = 0;
    	if (transport_.acquire("rw", &fd_, &read_mtu_, &write_mtu_)) {
    		signal_stop_ = false;
            pthread_create(&thread_, NULL, threadProc, this);
            running_ = true;
    	}
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
    } else {
    	LOG(WARNING) << "Playback thread already running.";
    }
}

void* PlaybackThread::threadProc(void *ctx) {
	PlaybackThread* pThis = reinterpret_cast<PlaybackThread*>(ctx);
	pThis->run();
	return NULL;
}

void PlaybackThread::run() {
	uint8_t* read_buffer = new uint8_t[read_mtu_];
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
    if (read_buffer) {
    	delete [] read_buffer;
    }
}

void PlaybackThread::playPcm(const uint8_t* buffer, size_t size) {
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
