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
    : codec_id(0),
      configuration(NULL),
      configuration_len(0),
      delay(0),
      nrec(false),
      inbound_ringtones(false),
      volume(0) {
}

MediaTransportProperties::~MediaTransportProperties() {
	if (configuration) {
		delete [] configuration;
	}
}

void MediaTransportProperties::parseDictionary(BaseMessageIterator* itprop) {
    while (itprop->hasNext() &&
    		itprop->getArgumentType() == DBUS_TYPE_DICT_ENTRY) {
    	BaseMessageIterator itentry = itprop->recurse();
    	const char* key = itentry.getString();
    	itentry.next();
    	BaseMessageIterator itvalue = itentry.recurse();
    	if (strcmp(key, PROP_DEVICE) == 0) {
    		device = itvalue.getObjectPath();
    	} else if (strcmp(key, PROP_CODEC) == 0) {
    		codec_id = itvalue.getByte();
    	} else if (strcmp(key, PROP_UUID) == 0) {
    		uuid = itvalue.getString();
    	} else if (strcmp(key, PROP_CONFIGURATION) == 0) {
            itvalue.getByteArray(&configuration, &configuration_len);
    	} else if (strcmp(key, PROP_DELAY) == 0) {
    		delay = itvalue.getWord();
    	} else if (strcmp(key, PROP_NREC) == 0) {
    		nrec = itvalue.getBool();
    	} else if (strcmp(key, PROP_INBOUND_RINGTONE) == 0) {
    		inbound_ringtones = itvalue.getBool();
    	} else if (strcmp(key, PROP_ROUTING) == 0) {
    		routing = itvalue.getString();
    	} else if (strcmp(key, PROP_VOLUME) == 0) {
    		volume = itvalue.getWord();
    	}
    	itprop->next();
    }
}

} /* namespace dbus */
