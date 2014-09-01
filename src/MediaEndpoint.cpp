/*
 * MediaEndpoint.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MediaEndpoint.h"

#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "MethodLocator.h"

#include <glog/logging.h>

namespace dbus {

const char* MEDIA_ENDPOINT_INTERFACE = "org.bluez.MediaEndpoint";
const char* PROP_DEVICE = "Device";
const char* PROP_UUID = "UUID";
const char* PROP_CODEC = "Codec";
const char* PROP_CONFIGURATION = "Configuration";
const char* PROP_DELAY = "Delay";
const char* PROP_NREC = "NREC";
const char* PROP_INBOUND_RINGTONE = "InbandRingtone";
const char* PROP_ROUTING = "Routing";
const char* PROP_VOLUME = "Volume";

class MediaEndpointSelectConfiguration : public MethodLocator {
public:
	MediaEndpointSelectConfiguration(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"SelectConfiguration") {}
	virtual Message handle(Message&, void* ctx) {
		//MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		return Message();
	}
};

class MediaEndpointSetConfiguration : public MethodLocator {
public:
	MediaEndpointSetConfiguration(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"SetConfiguration") {}
	virtual Message handle(Message& msg, void* ctx) {
		MessageArgumentIterator it = msg.argIterator();
        ObjectPath transport = it.getObjectPath();
        LOG(INFO) << "SetConfiguration transport:" << transport.path();
        it.next();
        MediaEndpoint::TransportProperties transport_properties;
        BaseMessageIterator itprop = it.recurse();
        while (itprop.hasNext() &&
        		itprop.getArgumentType() == DBUS_TYPE_DICT_ENTRY) {
        	BaseMessageIterator itentry = itprop.recurse();
        	const char* key = itentry.getString();
        	itentry.next();
        	BaseMessageIterator itvalue = itentry.recurse();
        	if (strcmp(key, PROP_DEVICE) == 0) {
        		transport_properties.device = itvalue.getObjectPath();
        	} else if (strcmp(key, PROP_CODEC) == 0) {
        		transport_properties.codec_id = itvalue.getByte();
        	} else if (strcmp(key, PROP_UUID) == 0) {
        		transport_properties.uuid = itvalue.getString();
        	} else if (strcmp(key, PROP_CONFIGURATION) == 0) {
                itvalue.getByteArray(&transport_properties.configuration,
                		&transport_properties.configuration_len);
        	} else if (strcmp(key, PROP_DELAY) == 0) {
        		transport_properties.delay = itvalue.getWord();
        	} else if (strcmp(key, PROP_NREC) == 0) {
        		transport_properties.nrec = itvalue.getBool();
        	} else if (strcmp(key, PROP_INBOUND_RINGTONE) == 0) {
        		transport_properties.inbound_ringtones = itvalue.getBool();
        	} else if (strcmp(key, PROP_ROUTING) == 0) {
        		transport_properties.routing = itvalue.getString();
        	} else if (strcmp(key, PROP_VOLUME) == 0) {
        		transport_properties.volume = itvalue.getWord();
        	}
            LOG(INFO) << "SetConfiguration: " << key;
        	itprop.next();
        }
		MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		pThis->setConfiguration(transport, transport_properties);
        Message reply;
        Message::forMethodReturn(msg, &reply);
		return reply;
	}
};

class MediaEndpointClearConfiguration : public MethodLocator {
public:
	MediaEndpointClearConfiguration(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"ClearConfiguration") {}
	virtual Message handle(Message& msg, void* ctx) {
		MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		pThis->clearConfiguration(msg.argIterator().getObjectPath());
		return Message();
	}
};

class MediaEndpointRelease : public MethodLocator {
public:
	MediaEndpointRelease(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"Release") {}
	virtual Message handle(Message&, void* ctx) {
		MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		pThis->release();
		return Message();
	}
};

void MediaEndpoint::registerMethods(Connection& connection) {
    connection.addMethodHandler(new MediaEndpointSelectConfiguration(), this);
    connection.addMethodHandler(new MediaEndpointSetConfiguration(), this);
    connection.addMethodHandler(new MediaEndpointClearConfiguration(), this);
    connection.addMethodHandler(new MediaEndpointRelease(), this);
}

void MediaEndpoint::unregisterMethods(Connection& connection) {
    connection.removeMethodHandler(MediaEndpointSelectConfiguration());
    connection.removeMethodHandler(MediaEndpointSetConfiguration());
    connection.removeMethodHandler(MediaEndpointClearConfiguration());
    connection.removeMethodHandler(MediaEndpointRelease());
}

void MediaEndpoint::selectConfiguration(void* capabilities,
		size_t capabilities_len,
		void** selected_capabilities,
		size_t* selected_capabilities_len) {

}

void MediaEndpoint::setConfiguration(ObjectPath transport,
		TransportProperties& properties) {

}

void MediaEndpoint::clearConfiguration(ObjectPath transport) {

}

void MediaEndpoint::release() {

}

} /* namespace dbus */
