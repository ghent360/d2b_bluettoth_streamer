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
#include "MediaTransportProperties.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "MethodLocator.h"

#include <glog/logging.h>

namespace dbus {

const char* MEDIA_ENDPOINT_INTERFACE = "org.bluez.MediaEndpoint";

class MediaEndpointSelectConfiguration : public MethodLocator {
public:
	MediaEndpointSelectConfiguration(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"SelectConfiguration") {}
	virtual Message handle(Message& msg, void* ctx) {
		//MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		Message reply;
		Message::forMethodReturn(msg, &reply);
		return reply;
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
        MediaTransportProperties transport_properties;
        BaseMessageIterator itprop = it.recurse();
        transport_properties.parseDictionary(&itprop);
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
		Message reply;
		Message::forMethodReturn(msg, &reply);
		return reply;
	}
};

class MediaEndpointRelease : public MethodLocator {
public:
	MediaEndpointRelease(): MethodLocator(Type::E_METHOD,
			MEDIA_ENDPOINT_INTERFACE,
    		"Release") {}
	virtual Message handle(Message& msg, void* ctx) {
		MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
		pThis->release();
		Message reply;
		Message::forMethodReturn(msg, &reply);
		return reply;
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

void MediaEndpoint::setConfiguration(const ObjectPath& transport,
		const MediaTransportProperties& properties) {

}

void MediaEndpoint::clearConfiguration(const ObjectPath& transport) {

}

void MediaEndpoint::release() {

}

} /* namespace dbus */
