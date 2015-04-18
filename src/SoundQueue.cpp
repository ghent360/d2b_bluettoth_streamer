/*
 * SoundQueue.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "SoundQueue.h"

#include "AudioMixer.h"
#include "SoundFragment.h"

#include <glog/logging.h>

namespace iqurius {

void SoundQueue::stop() {
  if (running_) {
    signal_stop_ = true;
    pthread_join(thread_, nullptr);
    running_ = false;
  }
}

void SoundQueue::start() {
  if (running_) {
    LOG(WARNING) << "SoundQueue thread already running.";
    return;
  }
  replay_ = false;
  signal_stop_ = false;
  pthread_create(&thread_, NULL, threadProc, this);
  running_ = true;
}

void* SoundQueue::threadProc(void *ctx) {
  SoundQueue* pThis = reinterpret_cast<SoundQueue*>(ctx);
  pThis->run();
  return NULL;
}

void SoundQueue::run() {
  SoundFragment* fragment = nullptr;
  while (!signal_stop_) {
	std::string next_fragment;
	{
	  googleapis::MutexLock lock(&mutex_);
	  if (scheduled_fragments_.size() > 0) {
		next_fragment = *scheduled_fragments_.begin();
		scheduled_fragments_.pop_front();
	  }
	}
    if (next_fragment.empty()) {
        usleep(100000);
    } else {
	  delete fragment;
	  fragment = SoundFragment::fromVorbisFile(next_fragment.c_str());
	  replay_ = true;
    }
    if (replay_) {
      if (fragment) {
    	fragment->playFragment(audio_channel_);
    	sleep(2);  // pause after each message
      }
      replay_ = false;
    }
  }
  delete fragment;
}

void SoundQueue::scheduleFragment(const char* path) {
  googleapis::MutexLock lock(&mutex_);
  if (path) {
	scheduled_fragments_.push_back(std::string(path));
  } else {
	LOG(ERROR) << "Trying to schedule a null pointer fragment";
  }
}

} /* namespace dbus */
