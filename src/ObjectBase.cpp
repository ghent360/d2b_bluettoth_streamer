/*
 * ObjectBase.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */
#include "Connection.h"
#include "ObjectBase.h"

namespace dbus {

ObjectBase::ObjectBase(Connection* connection)
    : self_(connection->makeObjectPath(this)) {
}

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

} /* namepsace dbus */
