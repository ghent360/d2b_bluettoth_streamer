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
#include "Connection.h"
#include "MediaEndpoint.h"
#include "MediaTransport.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "PlaybackThread.h"

#include <glog/logging.h>

namespace dbus {

AudioSource::AudioSource(Connection* connection, const ObjectPath& path,
		const MediaEndpoint& media_end_point)
    : SimpleObjectBase(path),
      connection_(connection),
	  media_end_point_(media_end_point),
	  playback_thread_(0) {
	interface_ = &implementation_;
}

AudioSource::~AudioSource() {
    if (playback_thread_) {
    	playback_thread_->stop();
    	delete playback_thread_;
    }
}

void AudioSource::onStateChange(const char* value) {
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
}

const StringWithHash AudioSource::INTERFACE("org.bluez.AudioSource");
const StringWithHash AudioSource::PROPERTYCHANGED_SIGNAL("PropertyChanged");
const StringWithHash AudioSource::STATE_PROPERTY("State");

Message AudioSource::handle_propertyChanged(Message& msg, ObjectBase* ctx) {
	MessageArgumentIterator it = msg.argIterator();
	if (it.hasArgs()) {
		const char* property_name = it.getString();
		if (it.next()) {
			BaseMessageIterator itv = it.recurse();
			const char* value = itv.getString();
			if (strcmp(property_name, AudioSource::STATE_PROPERTY.str()) == 0) {
				AudioSource* pImpl = reinterpret_cast<AudioSource*>(ctx);
				if (pImpl != NULL) {
					pImpl->onStateChange(value);
				}
			}
			LOG(INFO) << "AudioSourcePropertyChanged: " << property_name
					<< " " << value;
		}
	}
	return Message();
}

const MethodDescriptor AudioSource::interfaceMethods_[] = {
};

const MethodDescriptor AudioSource::interfaceSignals_[] = {
	MethodDescriptor(PROPERTYCHANGED_SIGNAL, handle_propertyChanged),
};

const InterfaceImplementation AudioSource::implementation_(INTERFACE,
		interfaceMethods_, interfaceSignals_);

} /* namespace dbus */
