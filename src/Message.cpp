/*
 * Message.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: vne-marti
 */

#include "Message.h"
#include "Connection.h"
#include "MessageArgumentIterator.h"

#include <glog/logging.h>

namespace dbus {

const std::string Message::EMPTY("");
const std::string DBUS_ERROR_FAILED_MSG_NAME(DBUS_ERROR_FAILED);

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

static const char* getString(const std::string& str) {
	if (str.empty()) {
		return NULL;
	}
	return str.c_str();
}

int Message::forMethodCall(const std::string& destination,
			const std::string& path,
			const std::string& iface,
			const std::string& method,
			Message *result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_method_call(getString(destination),
			getString(path),
			getString(iface),
			getString(method));
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return 0;
	}
	return 1;
}

int Message::forMethodReturn(const Message& methodCall, Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_method_return(methodCall.message_);
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return 0;
	}
	return 1;
}

int Message::forSignal(const std::string& path,
			const std::string& iface,
			const std::string& name,
			Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_signal(getString(path),
			getString(iface),
			getString(name));
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return 0;
	}
	return 1;
}

int Message::forError(const Message& replyTo,
			const std::string& name,
			const std::string& message,
			Message* result) {
	DBusError err;
	DBusMessage *msg;

	dbus_error_init(&err);
	msg = dbus_message_new_error(replyTo.message_,
			getString(name),
			getString(message));
	Connection::handleError(&err, __FUNCTION__, __LINE__);
	if (msg != NULL) {
		result->unref();
		result->message_ = msg;
		return 0;
	}
	return 1;
}

MessageArgumentIterator Message::argIterator() {
	return MessageArgumentIterator(*this);
}

} /* namespace dbus */
