/*
 * ObjectBase.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */
#include "MessageArgumentIterator.h"
#include "ObjectBase.h"

namespace dbus {

Message ObjectBase::handleMessage(Message& msg) {
	StringWithHash interfaceName = msg.getInterface();
	const InterfaceImplementation* implementation = matchInterface(interfaceName);
	if (NULL == implementation) {
		Message error;
		Message::forError(msg, DBUS_ERROR_UNKNOWN_INTERFACE, interfaceName.str(), &error);
		return error;
	}
	return implementation->handleMessage(msg, this);
}

ObjectPath ObjectBase::makeObjectPath(const char* prefix, const void* object) {
	std::string pathStr;
	char buffer[17];
	snprintf(buffer, 17, "%p", object);
	pathStr.append(prefix);
	pathStr.append("/");
	pathStr.append(buffer);
	ObjectPath result(pathStr);
	return result;
}

const InterfaceImplementation* SimpleObjectBase::matchInterface(
		const StringWithHash& interface) const {
	if (interface_->matchesInterface(interface)) {
		return interface_;
	}
	return NULL;
}

void SimpleObjectBase::registerSignals(Connection* conn) const {
	interface_->registerSignals(conn, this);
}

void SimpleObjectBase::unregisterSignals(Connection* conn) const {
	interface_->unregisterSignals(conn, this);
}

Message SimpleObjectBase::default_PropertyChange_handler(Message& msg,
		ObjectBase* ctx, const InterfaceImplementation* interface) {
	MessageArgumentIterator it = msg.argIterator();
	Message reply;
	if (it.hasArgs()) {
		StringWithHash property_name = it.getString();
		if (it.next()) {
		    if (!interface->handlePropertyChanged(property_name, &it, ctx)) {
		    	Message::forError(msg, DBUS_ERROR_UNKNOWN_PROPERTY,
		    			property_name.str(), &reply);
		    }
		} else {
			Message::forError(msg, DBUS_ERROR_INVALID_ARGS,
					"Missing property value", &reply);
		}
	} else {
		Message::forError(msg, DBUS_ERROR_INVALID_ARGS,
				"Missing property name", &reply);
	}
	return reply;
}

} /* namepsace dbus */
