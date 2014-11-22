/*
 * ObjectBase.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */
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

} /* namepsace dbus */
