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

SoundQueue::~SoundQueue() {
  stop();
  for (auto* fragment : scheduled_fragments_) {
	delete fragment;
  }
}

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
  while (!signal_stop_) {
	SoundFragment* next_fragment = nullptr;
	{
	  googleapis::MutexLock lock(&mutex_);
	  if (scheduled_fragments_.size() > 0) {
		next_fragment = *scheduled_fragments_.begin();
		scheduled_fragments_.pop_front();
	  }
	}
    if (next_fragment) {
	  delete current_fragment_;
	  current_fragment_ = next_fragment;
	  replay_ = true;
    } else {
      usleep(100000);
    }
    if (replay_) {
      if (current_fragment_) {
    	current_fragment_->playFragment(audio_channel_);
    	sleep(2);  // pause after each message
      }
      replay_ = false;
    }
  }
}

void SoundQueue::scheduleFragment(SoundFragment* fragment) {
  googleapis::MutexLock lock(&mutex_);
  if (fragment) {
	scheduled_fragments_.push_back(fragment);
  } else {
	LOG(ERROR) << "Trying to schedule a null pointer fragment";
  }
}

} /* namespace dbus */
