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

#include "InterfaceImplementation.h"
#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

namespace dbus {

class Connection;
class MediaEndpoint;
class PlaybackThread;
class AudioSource : public ObjectBase {
public:
	AudioSource(Connection* connection, const MediaEndpoint& media_end_point);
	virtual ~AudioSource();

protected:
	virtual const InterfaceImplementation* matchInterface(
			const StringWithHash& interface) const;

private:
	static Message handle_propertyChanged(Message& msg, ObjectBase* ctx);

	// possible values "disconnected", "connecting", "connected", "playing"
	virtual void onStateChange(const char* value);

	Connection* connection_;
	const MediaEndpoint& media_end_point_;
	PlaybackThread* playback_thread_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash PROPERTYCHANGED_SIGNAL;
	static const StringWithHash STATE_PROPERTY;

	static const MethodDescriptor audioSourceMethods_[];
	static const MethodDescriptor audioSourceSignals_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(AudioSource);
};

} /* namespace dbus */

#endif /* AUDIOSOURCE_H_ */
