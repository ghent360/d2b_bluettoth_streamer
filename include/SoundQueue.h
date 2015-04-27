/*
 * SoundQueue.h
 *
 *  Created on: Apr 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef SOUNDQUEUE_H_
#define SOUNDQUEUE_H_

#include "util.h"

#include <googleapis/base/mutex.h>
#include <list>
#include <pthread.h>

namespace iqurius {

class AudioChannel;
class SoundFragment;
class SoundQueue {
public:
	SoundQueue(AudioChannel* effect_audio_channel,
			AudioChannel* music_audio_channel)
      : effect_audio_channel_(effect_audio_channel),
		music_audio_channel_(music_audio_channel),
		replay_(false),
		auto_replay_(false),
		running_(false),
		signal_stop_(false),
		thread_() {
	}
	~SoundQueue() {
	  stop();
	}

	void scheduleFragment(const char* path, uint32_t repeat = 1, uint32_t delay = 0);
	void replay() { replay_ = true; }
	void autoReplay(bool auto_replay) { auto_replay_ = auto_replay; replay_ = auto_replay;}
	void waitQueueEmpty();
	void start();
	void stop();
private:
	static void* threadProc(void *);
	void run();

	class FragmentInfo
	{
	public:
		FragmentInfo(const char* path, uint32_t repeat = 1, uint32_t delay = 0)
		    : fragment_path_(path),
			  fragment_(nullptr),
			  repeat_num_(repeat),
			  repeat_delay_(delay) {
		}
		~FragmentInfo();

		void playFragment(AudioChannel*);

		uint32_t repeatLeft() const { return repeat_num_; }
		uint32_t delay() const { return repeat_delay_; }
	private:
		std::string fragment_path_;
		SoundFragment* fragment_;
		uint32_t repeat_num_;
		uint32_t repeat_delay_;

		DISALLOW_COPY_AND_ASSIGN(FragmentInfo);
	};
	std::list<FragmentInfo*> scheduled_fragments_;
	AudioChannel* effect_audio_channel_;
	AudioChannel* music_audio_channel_;
	bool replay_;
	bool auto_replay_;
	bool running_;
	bool signal_stop_;
	volatile bool fragment_playback_;
	pthread_t thread_;
	googleapis::Mutex mutex_;

	DISALLOW_COPY_AND_ASSIGN(SoundQueue);
};

} /* namespace iqurius */

#endif /* SOUNDQUEUE_H_ */
