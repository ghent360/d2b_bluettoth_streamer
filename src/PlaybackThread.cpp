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
	  write_mtu_(0) {
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

void PlaybackThread::play_pcm(const uint8_t* buffer, size_t size) {

}

} /* namespace dbus */
