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

const char* AudioSourceInterface::INTERFACE = "org.bluez.AudioSource";
const char* AudioSourceInterface::PROPERTYCHANGED_SIGNAL = "PropertyChanged";
const char* AudioSourceInterface::STATE_PROPERTY = "State";

class AudioSourcePropertyChanged : public MethodLocator {
public:
	AudioSourcePropertyChanged() : MethodLocator(Type::E_SIGNAL,
    		AudioSourceInterface::INTERFACE,
			AudioSourceInterface::PROPERTYCHANGED_SIGNAL) {
	}

	virtual Message handle(Message& msg, void* ctx) {
		MessageArgumentIterator it = msg.argIterator();
		if (it.hasArgs()) {
			const char* property_name = it.getString();
			if (it.next()) {
				BaseMessageIterator itv = it.recurse();
	            const char* value = itv.getString();
	            if (strcmp(property_name, AudioSourceInterface::STATE_PROPERTY) == 0) {
	            	AudioSourceInterface* pImpl = reinterpret_cast<AudioSourceInterface*>(ctx);
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
};

void AudioSourceInterface::registerMethods(Connection& connection, AudioSourceInterface* impl) {
	connection.addMethodHandler(new AudioSourcePropertyChanged(), impl);
}

void AudioSourceInterface::unregisterMethods(Connection& connection) {
	connection.removeMethodHandler(AudioSourcePropertyChanged());
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

} /* namespace dbus */
