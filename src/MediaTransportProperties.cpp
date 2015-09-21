/*
 * MediaTransportProperties.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MediaTransportProperties.h"
#include "MessageArgumentIterator.h"

#include <glog/logging.h>
#include <string.h>

namespace dbus {

const char* PROP_DEVICE = "Device";
const char* PROP_UUID = "UUID";
const char* PROP_CODEC = "Codec";
const char* PROP_CONFIGURATION = "Configuration";
const char* PROP_DELAY = "Delay";
const char* PROP_NREC = "NREC";
const char* PROP_INBOUND_RINGTONE = "InbandRingtone";
const char* PROP_ROUTING = "Routing";
const char* PROP_VOLUME = "Volume";

MediaTransportProperties::MediaTransportProperties()
    : codec_id_(0),
      configuration_(NULL),
      configuration_len_(0),
      delay_(0),
      nrec_(false),
      inbound_ringtones_(false),
      volume_(0) {
}

MediaTransportProperties::~MediaTransportProperties() {
	if (configuration_) {
		delete [] configuration_;
	}
}

void MediaTransportProperties::assign(const MediaTransportProperties& other) {
	if (configuration_) {
		delete [] configuration_;
		configuration_ = NULL;
	}
	device_ = other.device_;
    uuid_ = other.uuid_;
    codec_id_ = other.codec_id_;
    configuration_ = new uint8_t[other.configuration_len_];
    memcpy(configuration_, other.configuration_, other.configuration_len_);
    configuration_len_ = other.configuration_len_;
    delay_ = other.delay_;
    nrec_ = other.nrec_;
    inbound_ringtones_ = other.inbound_ringtones_;
    routing_ = other.routing_;
    volume_ = other.volume_;
}

void MediaTransportProperties::parseDictionary(BaseMessageIterator* itprop) {
    while (itprop->hasNext() &&
    		itprop->getArgumentType() == DBUS_TYPE_DICT_ENTRY) {
    	BaseMessageIterator itentry = itprop->recurse();
    	const char* key = itentry.getString();
    	itentry.next();
    	BaseMessageIterator itvalue = itentry.recurse();
    	if (strcmp(key, PROP_DEVICE) == 0) {
    		device_ = itvalue.getObjectPath();
    	} else if (strcmp(key, PROP_CODEC) == 0) {
    		codec_id_ = itvalue.getByte();
    	} else if (strcmp(key, PROP_UUID) == 0) {
    		uuid_ = itvalue.getString();
    	} else if (strcmp(key, PROP_CONFIGURATION) == 0) {
            itvalue.getByteArray(&configuration_, &configuration_len_);
    	} else if (strcmp(key, PROP_DELAY) == 0) {
    		delay_ = itvalue.getWord();
    	} else if (strcmp(key, PROP_NREC) == 0) {
    		nrec_ = itvalue.getBool();
    	} else if (strcmp(key, PROP_INBOUND_RINGTONE) == 0) {
    		inbound_ringtones_ = itvalue.getBool();
    	} else if (strcmp(key, PROP_ROUTING) == 0) {
    		routing_ = itvalue.getString();
    	} else if (strcmp(key, PROP_VOLUME) == 0) {
    		volume_ = itvalue.getWord();
    	}
    	itprop->next();
    }
}

void MediaTransportProperties::dump() const {
    LOG(INFO) << "device_:" << device_.path();
    LOG(INFO) << "uuid_:" << uuid_;
    LOG(INFO) << "codec_id_:" << codec_id_;
    LOG(INFO) << "delay_:" << delay_;
    LOG(INFO) << "nrec_:" << nrec_;
    LOG(INFO) << "inbound_ringtones_:" << inbound_ringtones_;
    LOG(INFO) << "routing_:" << routing_;
    LOG(INFO) << "volume_:" << volume_;
    LOG(INFO) << "configuration_len_:" << configuration_len_;
}

} /* namespace dbus */
