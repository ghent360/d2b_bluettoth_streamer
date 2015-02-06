/*
 * AudioTargetControl.cpp
 *
 *  Created on: Jan 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "AudioTargetControl.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "MediaEndpoint.h"
#include "MediaTransport.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "RemoteMethod.h"

#include <dbus/dbus.h>
#include <glog/logging.h>

namespace dbus {
AudioTargetControl::AudioTargetControl(Connection* connection, const ObjectPath& path)
    : SimpleObjectBase(path),
	  state_(false),
	  connection_(connection),
	  on_state_change_cb_(NULL),
	  song_pos_(0),
	  song_len_(0),
	  status_(STATUS_STOPPED) {
	interface_ = &implementation_;
}

AudioTargetControl::~AudioTargetControl() {
	delete on_state_change_cb_;
}

void AudioTargetControl::volumeUp() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, VOLUME_UP_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void AudioTargetControl::sendButton(uint8_t button_id) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SEND_BUTTON_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(button_id);
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void AudioTargetControl::updatePlayStatus() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, GET_PLAY_STATUS_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg() != NULL) {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			DictionaryHelper dict(&iter);
			song_pos_ = dict.getUint32("SongPos");
			song_len_ = dict.getUint32("SongLength");
			status_ = (EPlayerStatus)dict.getByte("Status");
		}
	}
}

void AudioTargetControl::volumeDown() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, VOLUME_DOWN_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void AudioTargetControl::getEventCapabilities() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, GET_EVENT_CAPABILITIES_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg()) {
		auto iter = reply.argIterator().recurse();
		do {
			LOG(INFO) << "Company Id: " << (int)iter.getByte();
		} while (iter.next());
	}
}

void AudioTargetControl::onStateChanged(bool new_state) {
	if (on_state_change_cb_) {
		bool once = !on_state_change_cb_->IsRepeatable();
		on_state_change_cb_->Run(new_state, this);
		if (once) {
			on_state_change_cb_ = NULL;
		}
	}
	state_ = new_state;
}

void AudioTargetControl::refreshProperties() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf().str(), INTERFACE.str(), GETPROPERTIES_METHOD.str());
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg()) {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			DictionaryHelper dict(&iter);
			state_ = dict.getBool(CONNECTED_PROPERTY);
		}
	} else {
		LOG(ERROR) << "Error calling " << INTERFACE.str() << "." << GETPROPERTIES_METHOD.str();
	}
}
const StringWithHash AudioTargetControl::INTERFACE("org.bluez.Control");
const StringWithHash AudioTargetControl::VOLUME_UP_METHOD("VolumeUp");
const StringWithHash AudioTargetControl::SEND_BUTTON_METHOD("SendButton");
const StringWithHash AudioTargetControl::VOLUME_DOWN_METHOD("VolumeDown");
const StringWithHash AudioTargetControl::GET_PLAY_STATUS_METHOD("GetPlayStatus");
const StringWithHash AudioTargetControl::GET_EVENT_CAPABILITIES_METHOD("GetEventCapabilities");
const StringWithHash AudioTargetControl::GETPROPERTIES_METHOD("GetProperties");

const StringWithHash AudioTargetControl::PROPERTYCHANGED_SIGNAL("PropertyChanged");
const StringWithHash AudioTargetControl::CONNECTED_PROPERTY("Connected");

void AudioTargetControl::handle_stateChanged(bool new_state, ObjectBase* ctx) {
	AudioTargetControl* pThis = reinterpret_cast<AudioTargetControl*>(ctx);
	pThis->onStateChanged(new_state);
}

const MethodDescriptor AudioTargetControl::interfaceMethods_[] = {
};

const MethodDescriptor AudioTargetControl::interfaceSignals_[] = {
	MethodDescriptor(PROPERTYCHANGED_SIGNAL, default_PropertyChange_handler)
};

const PropertyDescriptor AudioTargetControl::interfaceProperties_[] = {
    PropertyDescriptor(CONNECTED_PROPERTY, handle_stateChanged)
};

const InterfaceImplementation AudioTargetControl::implementation_(INTERFACE,
		interfaceMethods_, interfaceSignals_, interfaceProperties_);

} /* namespace dbus */
