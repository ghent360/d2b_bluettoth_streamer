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
#include "MessageArgumentIterator.h"

#include <glog/logging.h>

namespace dbus {

void PropertyHandler::callHandler(BaseMessageIterator* value_iterator,
		ObjectBase* ctx) const {
    int type = value_iterator->getArgumentType();
    switch (type) {
    case DBUS_TYPE_STRING:
    {
    	const char* string_value = value_iterator->getString();
    	string_handler_(string_value, ctx);
    	break;
    }

    case DBUS_TYPE_ARRAY:
    {
    	BaseMessageIterator array_value = value_iterator->recurse();
    	array_handler_(array_value, ctx);
    	break;
    }

    case DBUS_TYPE_BOOLEAN:
    {
    	bool bool_value = value_iterator->getBool();
    	bool_handler_(bool_value, ctx);
    	break;
    }

    case DBUS_TYPE_OBJECT_PATH:
    {
    	ObjectPath obj_path_value = value_iterator->getObjectPath();
    	object_handler_(obj_path_value, ctx);
    	break;
    }

    case DBUS_TYPE_UINT32:
    {
    	uint32_t uint32_value = value_iterator->getUint32();
    	uint32_handler_(uint32_value, ctx);
    	break;
    }

    case DBUS_TYPE_VARIANT:
    {
    	auto variant_value = value_iterator->recurse();
    	callHandler(&variant_value, ctx);
    	break;
    }

    default:
    	LOG(ERROR) << "Type " << type << " is not supported yet.";
		break;
    }
}

const MethodDescriptor* InterfaceImplementation::findMethod(const StringWithHash& name,
		const std::list<MethodDescriptor>& list) const {
	for (const MethodDescriptor& d : list) {
		if (d.method_name_ == name) {
			return &d;
		}
	}
	LOG(ERROR) << "Unable to find method: " << name;
	LOG(ERROR) << "Available methods:";
	for (const MethodDescriptor& d : list) {
		LOG(ERROR) << "    " << d.method_name_;
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
			return d->handler_(msg, ctx, this);
		}
		Message::forError(msg, DBUS_ERROR_UNKNOWN_METHOD, methodName.str(), &error);
		break;

	case DBUS_MESSAGE_TYPE_SIGNAL:
		d = findMethod(methodName, signals_);
		if (d != NULL) {
			return d->handler_(msg, ctx, this);
		}
		Message::forError(msg, DBUS_ERROR_UNKNOWN_METHOD, methodName.str(), &error);
		break;

	default:
		LOG(ERROR) << "Unexpected method type: " << type;
		break;
	}
	return error;
}

void InterfaceImplementation::registerSignals(Connection* conn,
		const ObjectBase* object) const {
	ObjectPath path = object->getPathToSelf();
	for (const MethodDescriptor& d : signals_) {
		conn->registerSignal(path.str(), getInterfaceName(), d.method_name_.str());
	}
}

void InterfaceImplementation::unregisterSignals(Connection* conn,
		const ObjectBase* object) const {
	ObjectPath path = object->getPathToSelf();
	for (const MethodDescriptor& d : signals_) {
		conn->unregisterSignal(path.str(), getInterfaceName(), d.method_name_.str());
	}
}

bool InterfaceImplementation::handlePropertyChanged(
		const StringWithHash& property_name, BaseMessageIterator* value,
		ObjectBase* ctx) const {
    for (const PropertyDescriptor& d : properties_) {
    	if (d.property_name_ == property_name) {
    		d.handler_.callHandler(value, ctx);
    		return true;
    	}
    }
    return false;
}
} /* namepsace dbus */
