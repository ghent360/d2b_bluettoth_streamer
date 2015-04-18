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

#include <glog/logging.h>
#include <googleapis/base/mutex.h>
#include <list>
#include <pthread.h>

namespace iqurius {

class AudioChannel;
class SoundFragment;
class SoundQueue {
public:
	SoundQueue(AudioChannel* audio_channel)
      : current_fragment_(nullptr),
		audio_channel_(audio_channel),
		replay_(false),
		running_(false),
		signal_stop_(false),
		thread_() {
	}
	virtual ~SoundQueue();

	void scheduleFragment(SoundFragment* fragment);
	void replay() { replay_ = true; }
	void start();
	void stop();
private:
	static void* threadProc(void *);
	void run();

	std::list<SoundFragment*> scheduled_fragments_;
	SoundFragment* current_fragment_;
	AudioChannel* audio_channel_;
	bool replay_;
	bool running_;
	bool signal_stop_;
	pthread_t thread_;
	googleapis::Mutex mutex_;

	DISALLOW_COPY_AND_ASSIGN(SoundQueue);
};

} /* namespace iqurius */

#endif /* SOUNDQUEUE_H_ */
