/*
 * Message.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "util.h"
#include <dbus/dbus.h>
#include <string>

namespace dbus {

class MessageArgumentBuilder;
class MessageArgumentIterator;
class MethodBase;

class Message {
public:
	Message(DBusMessage* msg = NULL);
	Message(const Message& other);
	virtual ~Message();

	static bool forMethodCall(const MethodBase& method, Message* result);

	static bool forMethodCall(const char* destination,
			const char* path,
			const char* iface,
			const char* method,
			Message* result);

	static bool forMethodCall(const char* destination,
			const char* path,
			const char* method,
			Message* result) {
		return forMethodCall(destination, path, NULL, method, result);
	}

	static bool forMethodCall(const char* path,
			const char* method,
			Message* result) {
		return forMethodCall(NULL, path, method, result);
	}

	static bool forMethodReturn(const Message& method_call, Message* result);

	static bool forSignal(const char* path,
			const char* iface,
			const char* name,
			Message* result);

	static bool forError(const Message& reply_to,
			const char* name,
			const char* message,
			Message* result);

	static bool forError(const Message& reply_to,
			const char* message,
			Message* result) {
		return forError(reply_to, DBUS_ERROR_FAILED_MSG_NAME, message, result);
	}

	MessageArgumentIterator argIterator();
	MessageArgumentBuilder argBuilder();

	DBusMessage* msg() {
		return message_;
	}

	void assign(const Message& other);
	void takeOwnership(DBusMessage* message);

	Message& operator = (const Message& other) {
		assign(other);
		return *this;
	}

	unsigned int getSerial() {
        return dbus_message_get_serial(message_);
	}
private:
	const static char* DBUS_ERROR_FAILED_MSG_NAME;

	void unref();

	DBusMessage* message_;
};

} /* namespace dbus */

#endif /* MESSAGE_H_ */
