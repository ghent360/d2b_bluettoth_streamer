/*
 * PlaybackThread.h
 *
 *  Created on: Nov 19, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef PLAYBACKTHREAD_H_
#define PLAYBACKTHREAD_H_

#include "MediaTransport.h"
#include "util.h"

#include <alsa/asoundlib.h>
#include <pthread.h>

namespace dbus {

class ObjectPath;
class PlaybackThread {
public:
	PlaybackThread(Connection* connection, const ObjectPath&);
	virtual ~PlaybackThread() {
		stop();
	};

	virtual void decode(const uint8_t* buffer, size_t size) = 0;
	virtual void playPcm(const uint8_t* buffer, size_t size);

	void start();
	void stop();
private:
	void setPcmParams();

	bool running_;
	bool signal_stop_;
	MediaTransport transport_;
	pthread_t thread_;

	int fd_;
	int read_mtu_;
	int write_mtu_;

	snd_pcm_t *pcm_handle_;

	static void* threadProc(void *);
	void run();

	DISALLOW_COPY_AND_ASSIGN(PlaybackThread);
};

} /* namespace dbus */

#endif /* PLAYBACKTHREAD_H_ */
