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

static constexpr size_t MAX_QUEUED_MESSAGES = 10;

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
  FragmentInfo* next_fragment = nullptr;
  fragment_playback_ = false;
  while (!signal_stop_) {
	bool got_new_fragment = false;
	{
	  googleapis::MutexLock lock(&mutex_);
	  if (scheduled_fragments_.size() > 0) {
		delete next_fragment;
		next_fragment = *scheduled_fragments_.begin();
		scheduled_fragments_.pop_front();
		got_new_fragment = true;
	  }
	}
	if (!got_new_fragment) {
      usleep(100000);
	}
    if (!next_fragment) {
      fragment_playback_ = false;
      continue;
    }
    while (replay_ || next_fragment->repeatLeft()) {
      fragment_playback_ = true;
      int16_t old_music_volume = music_audio_channel_->getVolume();
      int16_t old_effects_volume = effect_audio_channel_->getVolume();
      music_audio_channel_->setVolume(0.3f);
      effect_audio_channel_->setVolume(0.7f);
      next_fragment->playFragment(effect_audio_channel_);
      effect_audio_channel_->setVolume(old_effects_volume);
      music_audio_channel_->setVolume(old_music_volume);
      usleep(next_fragment->delay());
      if (!auto_replay_) {
        replay_ = false;
      }
    }
    if (fragment_playback_) {
	  fragment_playback_ = false;
      sleep(1);  // pause after each message
    }
  }
  delete next_fragment;
}

void SoundQueue::FragmentInfo::playFragment(AudioChannel* channel) {
  if (!fragment_) {
	fragment_ = SoundFragment::fromVorbisFile(fragment_path_.c_str());
  }
  if (!fragment_) {
	LOG(ERROR) << "Unable to load audio fragment " << fragment_path_;
	repeat_num_ = 0;
	return;
  }
  fragment_->playFragment(channel);
  if (repeat_num_)
	repeat_num_--;
}

SoundQueue::FragmentInfo::~FragmentInfo() {
	delete fragment_;
}

void SoundQueue::waitQueueEmpty() {
  autoReplay(false);
  while (true) {
	{
      googleapis::MutexLock lock(&mutex_);
      if (scheduled_fragments_.size() == 0 && !fragment_playback_) {
        return;
      }
	}
    usleep(250000);
  }
}

void SoundQueue::scheduleFragment(const char* path,
		uint32_t repeat,
		uint32_t delay) {
  googleapis::MutexLock lock(&mutex_);
  if (!path) {
	LOG(ERROR) << "Trying to schedule a null pointer fragment";
  }
  if (scheduled_fragments_.size() < MAX_QUEUED_MESSAGES) {
	scheduled_fragments_.push_back(new FragmentInfo(path, repeat, delay));
  } else {
	LOG(ERROR) << "Sound queue is full.";
  }
}

} /* namespace dbus */
