/*
 * AudioSource.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef AUDIOSOURCE_H_
#define AUDIOSOURCE_H_

#include "MethodLocator.h"
#include "util.h"

namespace dbus {

class Connection;
class AudioSourceInterface {
public:
	virtual ~AudioSourceInterface() {}

protected:
	virtual void onStateChange(const char* value) = 0;

	friend class AudioSourcePropertyChanged;
	static const char* INTERFACE;
	static const char* PROPERTYCHANGED_SIGNAL;
	static const char* STATE_PROPERTY;

public:
	static void registerMethods(Connection&, AudioSourceInterface*);
	static void unregisterMethods(Connection&);
};

class MediaEndpoint;
class PlaybackThread;
class AudioSource : public AudioSourceInterface {
public:
	AudioSource(Connection* connection, const MediaEndpoint& media_end_point)
        : connection_(connection),
		  media_end_point_(media_end_point),
		  playback_thread_(0) {
	}

	virtual ~AudioSource();

private:
	// possible values "disconnected", "connecting", "connected", "playing"
	virtual void onStateChange(const char* value);

	Connection* connection_;
	const MediaEndpoint& media_end_point_;
	PlaybackThread* playback_thread_;
	DISALLOW_COPY_AND_ASSIGN(AudioSource);
};

} /* namespace dbus */

#endif /* AUDIOSOURCE_H_ */
