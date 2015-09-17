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

#include <glog/logging.h>

namespace dbus {

MediaEndpoint::MediaEndpoint(const ObjectPath& path)
    : SimpleObjectBase(path),
	  transport_config_valid_(false) {
	interface_ = &implementation_;
}

void MediaEndpoint::setConfiguration(const ObjectPath& transport,
		const MediaTransportProperties& properties) {
	LOG(INFO) << "MediaEndpoint::setConfiguration transport=" << transport.path();
	properties.dump();
	transport_path_ = transport;
	transport_properties_ = properties;
	transport_config_valid_ = true;
}

void MediaEndpoint::clearConfiguration(const ObjectPath& transport) {
	LOG(INFO) << "MediaEndpoint::clearConfiguration";
	transport_config_valid_ = false;
}

void MediaEndpoint::release() {
	LOG(INFO) << "MediaEndpoint::release";
	transport_config_valid_ = false;
}

const StringWithHash MediaEndpoint::INTERFACE("org.bluez.MediaEndpoint");
const StringWithHash MediaEndpoint::SELECTCONFIGURATION_METHOD("SelectConfiguration");
const StringWithHash MediaEndpoint::SETCONFIGURATION_METHOD("SetConfiguration");
const StringWithHash MediaEndpoint::CLEARCONFIGURATION_METHOD("ClearConfiguration");
const StringWithHash MediaEndpoint::RELEASE_METHOD("Release");

Message MediaEndpoint::handle_selectConfiguration(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation*) {
	MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
	uint8_t *capabilities_in;
    uint8_t *capabilities_out;
    size_t size_in, size_out;
    Message reply;
    DBusError err;
    bool success;

    dbus_error_init(&err);
    auto arguments = msg.argIterator();
    if (arguments.getArgumentType() != DBUS_TYPE_ARRAY) {
    	Connection::handleError(&err, __FUNCTION__, __LINE__);
    	Message::forError(msg, "org.bluez.MediaEndpoint.Error.InvalidArguments",
    			"Unable to select configuration", &reply);
        return reply;
    }
    arguments.getByteArray(&capabilities_in, &size_in);
    success = pThis->selectConfiguration(capabilities_in, size_in,
    		&capabilities_out, &size_out);
    if (success) {
    	Message::forMethodReturn(msg, &reply);
    	reply.argBuilder()
    			.openContainer(DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE)
				.append(capabilities_out, size_out);
    	delete [] capabilities_out;
    } else {
    	Message::forError(msg, "org.bluez.MediaEndpoint.Error.InvalidArguments",
    			"Unable to select configuration", &reply);
    }
	return reply;
}

Message MediaEndpoint::handle_setConfiguration(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation*) {
	MessageArgumentIterator it = msg.argIterator();
	ObjectPath transport = it.getObjectPath();
	it.next();
	MediaTransportProperties transport_properties;
	BaseMessageIterator itprop = it.recurse();
	transport_properties.parseDictionary(&itprop);
	MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
	pThis->setConfiguration(transport, transport_properties);
	return Message();
}

Message MediaEndpoint::handle_clearConfiguration(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation*) {
	MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
	pThis->clearConfiguration(msg.argIterator().getObjectPath());
	return Message();
}

Message MediaEndpoint::handle_release(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation*) {
	MediaEndpoint* pThis = reinterpret_cast<MediaEndpoint*>(ctx);
	pThis->release();
	return Message();
}

const MethodDescriptor MediaEndpoint::interfaceMethods_[] = {
	MethodDescriptor(SELECTCONFIGURATION_METHOD, handle_selectConfiguration),
	MethodDescriptor(SETCONFIGURATION_METHOD, handle_setConfiguration),
	MethodDescriptor(CLEARCONFIGURATION_METHOD, handle_clearConfiguration),
	MethodDescriptor(RELEASE_METHOD, handle_release),
};

const InterfaceImplementation MediaEndpoint::implementation_(INTERFACE, interfaceMethods_);

} /* namespace dbus */
