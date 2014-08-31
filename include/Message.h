/*
 * Message.h
 *
 *  Created on: Aug 31, 2014
 *      Author: vne-marti
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "util.h"
#include <dbus/dbus.h>
#include <string>

namespace dbus {

class MessageArgumentIterator;

class Message {
public:
	Message(DBusMessage* msg = NULL);
	Message(const Message& other);
	virtual ~Message();

	static int forMethodCall(const std::string& destination,
			const std::string& path,
			const std::string& iface,
			const std::string& method,
			Message* result);

	static int forMethodCall(const std::string& destination,
			const std::string& path,
			const std::string& method,
			Message* result) {
		return forMethodCall(destination, path, EMPTY, method, result);
	}

	static int forMethodCall(const std::string& path,
			const std::string& method,
			Message* result) {
		return forMethodCall(EMPTY, path, method, result);
	}

	static int forMethodReturn(const Message& methodCall, Message* result);

	static int forSignal(const std::string& path,
			const std::string& iface,
			const std::string& name,
			Message* result);

	static int forError(const Message& replyTo,
			const std::string& name,
			const std::string& message,
			Message* result);

	static int forError(const Message& replyTo,
			const std::string& message,
			Message* result) {
		return forError(replyTo, DBUS_ERROR_FAILED_MSG_NAME, message, result);
	}

	MessageArgumentIterator argIterator();

	DBusMessage* msg() {
		return message_;
	}

	void assign(const Message& other);
	Message& operator = (const Message& other) {
		assign(other);
		return *this;
	}

private:
	const static std::string EMPTY;
	const static std::string DBUS_ERROR_FAILED_MSG_NAME;

	void unref();

	DBusMessage* message_;
};

} /* namespace dbus */

#endif /* MESSAGE_H_ */
