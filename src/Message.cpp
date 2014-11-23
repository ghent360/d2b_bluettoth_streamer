/*
 * Message.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "Message.h"
#include "Connection.h"
#include "MessageArgumentIterator.h"
#include <glog/logging.h>
#include <RemoteMethod.h>

namespace dbus {

const char* DBUS_ERROR_FAILED_MSG_NAME = DBUS_ERROR_FAILED;

Message::~Message() {
	unref();
}

Message::Message(DBusMessage* message) : message_(message) {
}

Message::Message(const Message& other) {
	message_ = other.message_;
	if (NULL != message_) {
		dbus_message_ref(message_);
	}
}

void Message::unref() {
	if (message_) {
		dbus_message_unref(message_);
	}
	message_ = NULL;
}

void Message::assign(const Message& other) {
	unref();
	message_ = other.message_;
	if (NULL != message_) {
		dbus_message_ref(message_);
	}
}

void Message::takeOwnership(DBusMessage* message) {
	unref();
	message_ = message;
}

bool Message::forMethodCall(const char* destination,
			const char* path,
			const char* iface,
			const char* method,
			Message *result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_method_call(destination,
			path,
			iface,
			method);
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return true;
	}
	return false;
}

bool Message::forMethodCall(const RemoteMethod& method, Message* result) {
    return forMethodCall(method.destination(),
    		method.path(),
    		method.interface(),
    		method.methodName(),
    		result);
}

bool Message::forMethodReturn(const Message& method_call, Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_method_return(method_call.message_);
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return true;
	}
	return false;
}

bool Message::forSignal(const char* path,
			const char* iface,
			const char* name,
			Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_signal(path, iface, name);
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return true;
	}
	return false;
}

bool Message::forError(const Message& reply_to,
			const char* name,
			const char* message,
			Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_error(reply_to.message_, name, message);
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return true;
	}
	return false;
}

void Message::dump() {
	int type = getType();
	const char* dest = dbus_message_get_destination(message_);
	const char* path = dbus_message_get_path(message_);
	const char* interface = dbus_message_get_interface(message_);
	const char* member = dbus_message_get_member(message_);
	if (!dest) dest = "";
	if (!path) path = "";
	if (!interface) interface = "";
	if (!member) member = "";
	if (type == 1 || type == 4) {
		LOG(INFO) << " type=" << type << " dest=" << dest << " path=" << path <<
				" '" << interface << "::"<< member << "'";
	} else if (type == 3) {
		const char* err_name = dbus_message_get_error_name(message_);
		uint32_t serial = dbus_message_get_reply_serial(message_);
		if (!err_name) err_name = "";
		LOG(ERROR) << "error: " << err_name << " dest=" << dest << " path=" <<
				path << " '" << interface << "::"<< member << "' serial=" << serial;
	}
}

MessageArgumentIterator Message::argIterator() {
	return MessageArgumentIterator(*this);
}

MessageArgumentBuilder Message::argBuilder() {
	return MessageArgumentBuilder(*this);
}

} /* namespace dbus */
