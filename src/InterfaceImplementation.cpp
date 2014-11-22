/*
 * InterfaceImplementation.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */
#include "Connection.h"
#include "InterfaceImplementation.h"

#include <glog/logging.h>

namespace dbus {

const MethodDescriptor* InterfaceImplementation::findMethod(const StringWithHash& name,
		const std::list<MethodDescriptor>& list) const {
	for (const MethodDescriptor& d : list) {
		if (d.methodName_ == name) {
			return &d;
		}
	}
	return NULL;
}

Message InterfaceImplementation::handleMessage(Message& msg, ObjectBase* ctx) const {
	int type = msg.getType();
	StringWithHash methodName = msg.getMethod();
	Message error;
	const MethodDescriptor* d;

	switch (type) {
	case DBUS_MESSAGE_TYPE_METHOD_CALL:
		d = findMethod(methodName, methods_);
		if (d != NULL) {
			return d->handler_(msg, ctx);
		}
		Message::forError(msg, DBUS_ERROR_UNKNOWN_METHOD, methodName.str(), &error);
		break;

	case DBUS_MESSAGE_TYPE_SIGNAL:
		d = findMethod(methodName, signals_);
		if (d != NULL) {
			return d->handler_(msg, ctx);
		}
		Message::forError(msg, DBUS_ERROR_UNKNOWN_METHOD, methodName.str(), &error);
		break;

	default:
		LOG(ERROR) << "Unexpected method type: " << type;
		break;
	}
	return error;
}

} /* namepsace dbus */
