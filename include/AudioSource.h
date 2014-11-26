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

#include <googleapis/base/callback.h>

namespace dbus {

class Connection;
class MediaEndpoint;
class AudioSource : public SimpleObjectBase {
public:
	// possible values "disconnected", "connecting", "connected", "playing"
	typedef googleapis::Callback2<const char*, const AudioSource*> OnStateChangeCallback;

	AudioSource(Connection* connection, const ObjectPath& path);
	virtual ~AudioSource();

	bool connect();
	void connectAsync(int timeout, googleapis::Callback1<Message*>* cb);
	void disconnect();
	void disconnectAsync(int timeout, googleapis::Callback1<Message*>* cb);

	void setOnStateChangeCallback(OnStateChangeCallback* cb) {
	   		delete on_state_change_cb_;
	   		on_state_change_cb_ = cb;
	}
private:
	static void handle_stateChanged(const char* new_state, ObjectBase* ctx);

	Connection* connection_;
	OnStateChangeCallback* on_state_change_cb_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash CONNECT_METHOD;
	static const StringWithHash DISCONNECT_METHOD;
	static const StringWithHash GETPROPERTIES_METHOD;

	static const StringWithHash PROPERTYCHANGED_SIGNAL;
	static const StringWithHash STATE_PROPERTY;

	static const MethodDescriptor interfaceMethods_[];
	static const MethodDescriptor interfaceSignals_[];
	static const PropertyDescriptor interfaceProperties_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(AudioSource);
};

} /* namespace dbus */

#endif /* AUDIOSOURCE_H_ */
