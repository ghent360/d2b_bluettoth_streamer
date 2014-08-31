/*
 * MessageArgumentIterator.h
 *
 *  Created on: Aug 31, 2014
 *      Author: vne-marti
 */

#ifndef MESSAGEARGUMENTITERATOR_H_
#define MESSAGEARGUMENTITERATOR_H_

#include "Message.h"

#include <dbus/dbus.h>

namespace dbus {

class MessageArgumentIterator {
public:
	MessageArgumentIterator(Message& msg) {
		has_args_ = dbus_message_iter_init(msg.msg(), &iter_) == TRUE;
	}

	bool hasArgs() const {
		return has_args_;
	}

	bool next() {
		return dbus_message_iter_next(&iter_) == TRUE;
	}

	bool hasNext() {
		return dbus_message_iter_has_next(&iter_) == TRUE;
	}

	bool append(const std::string& str) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_STRING,
				str.c_str()) == TRUE;
	}

	bool appendObjectPath(const std::string& str) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_OBJECT_PATH,
				str.c_str()) == TRUE;
	}

	int getArgumentType() {
		return dbus_message_iter_get_arg_type(&iter_);
	}

	std::string getString() {
		return getStringForType(DBUS_TYPE_STRING);
	}

	std::string getObjectPath() {
		return getStringForType(DBUS_TYPE_OBJECT_PATH);
	}
private:
	std::string getStringForType(int type);
	DBusMessageIter iter_;
	bool has_args_;
};

} /* namespace dbus */

#endif /* MESSAGEARGUMENTITERATOR_H_ */
