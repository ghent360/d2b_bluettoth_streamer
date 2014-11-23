/*
 * AudioSource.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "AudioSource.h"
#include "BluezNames.h"
#include "Connection.h"
#include "MediaEndpoint.h"
#include "MediaTransport.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "PlaybackThread.h"
#include "RemoteMethod.h"

#include <glog/logging.h>

namespace dbus {
/*
AudioSource::AudioSource(Connection* connection, const ObjectPath& path,
		const MediaEndpoint& media_end_point)
    : SimpleObjectBase(path),
      connection_(connection),
	  media_end_point_(media_end_point),
	  playback_thread_(0) {
	interface_ = &implementation_;
}
*/
AudioSource::AudioSource(Connection* connection, const ObjectPath& path)
    : SimpleObjectBase(path),
	  connection_(connection) {
	interface_ = &implementation_;
}

AudioSource::~AudioSource() {
    /*if (playback_thread_) {
    	playback_thread_->stop();
    	delete playback_thread_;
    }*/
}

bool AudioSource::connect() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return reply.msg() != NULL && reply.getType() == DBUS_MESSAGE_TYPE_METHOD_RETURN;
}

void AudioSource::connectAsync(googleapis::Callback1<Message*>* cb) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	connection_->send(rpc, -1, cb);
}

void AudioSource::disconnect() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, DISCONNECT_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void AudioSource::onStateChange(const char* value) {
/*
    if (strcmp(value, "playing") == 0) {
    	if (media_end_point_.isTransportConfigValid()) {
    	    if (playback_thread_) {
    	    	playback_thread_->stop();
    	    	delete playback_thread_;
    	    	playback_thread_ = 0;
    	    }
    	    playback_thread_ = new PlaybackThread(connection_, media_end_point_.getTransportPath());
    	    playback_thread_->start();
    	}
    } else if (strcmp(value, "connected") == 0 ||
    		   strcmp(value, "disconnected") == 0) {
	    if (playback_thread_) {
	    	playback_thread_->stop();
	    	delete playback_thread_;
	    	playback_thread_ = 0;
	    }
    }
*/
}

const StringWithHash AudioSource::INTERFACE("org.bluez.AudioSource");
const StringWithHash AudioSource::CONNECT_METHOD("Connect");
const StringWithHash AudioSource::DISCONNECT_METHOD("Disconnect");
const StringWithHash AudioSource::GETPROPERTIES_METHOD("GetProperties");

const StringWithHash AudioSource::PROPERTYCHANGED_SIGNAL("PropertyChanged");
const StringWithHash AudioSource::STATE_PROPERTY("State");

void AudioSource::handle_stateChanged(const char* new_state, ObjectBase* ctx) {
	AudioSource* pImpl = reinterpret_cast<AudioSource*>(ctx);
	pImpl->onStateChange(new_state);
}

const MethodDescriptor AudioSource::interfaceMethods_[] = {
};

const MethodDescriptor AudioSource::interfaceSignals_[] = {
	MethodDescriptor(PROPERTYCHANGED_SIGNAL, default_PropertyChange_handler)
};

const PropertyDescriptor AudioSource::interfaceProperties_[] = {
    PropertyDescriptor(STATE_PROPERTY, handle_stateChanged)
};

const InterfaceImplementation AudioSource::implementation_(INTERFACE,
		interfaceMethods_, interfaceSignals_, interfaceProperties_);

} /* namespace dbus */
