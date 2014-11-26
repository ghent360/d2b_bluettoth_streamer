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
#include "RemoteMethod.h"

#include <glog/logging.h>

namespace dbus {
AudioSource::AudioSource(Connection* connection, const ObjectPath& path)
    : SimpleObjectBase(path),
	  connection_(connection),
	  on_state_change_cb_(NULL) {
	interface_ = &implementation_;
}

AudioSource::~AudioSource() {
	delete on_state_change_cb_;
}

bool AudioSource::connect() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return reply.msg() != NULL && reply.getType() == DBUS_MESSAGE_TYPE_METHOD_RETURN;
}

void AudioSource::connectAsync(int timeout, googleapis::Callback1<Message*>* cb) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	connection_->send(rpc, timeout, cb);
}

void AudioSource::disconnect() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, DISCONNECT_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void AudioSource::disconnectAsync(int timeout, googleapis::Callback1<Message*>* cb) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, DISCONNECT_METHOD);
	rpc.prepareCall();
	connection_->send(rpc, timeout, cb);
}

const StringWithHash AudioSource::INTERFACE("org.bluez.AudioSource");
const StringWithHash AudioSource::CONNECT_METHOD("Connect");
const StringWithHash AudioSource::DISCONNECT_METHOD("Disconnect");
const StringWithHash AudioSource::GETPROPERTIES_METHOD("GetProperties");

const StringWithHash AudioSource::PROPERTYCHANGED_SIGNAL("PropertyChanged");
const StringWithHash AudioSource::STATE_PROPERTY("State");

void AudioSource::handle_stateChanged(const char* new_state, ObjectBase* ctx) {
	AudioSource* pThis = reinterpret_cast<AudioSource*>(ctx);
	if (pThis->on_state_change_cb_) {
		bool once = !pThis->on_state_change_cb_->IsRepeatable();
		pThis->on_state_change_cb_->Run(new_state, pThis);
		if (once) {
			pThis->on_state_change_cb_ = NULL;
		}
	}
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
