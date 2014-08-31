/*
 * MessageArgumentIterator.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: vne-marti
 */

#include "MessageArgumentIterator.h"

#include <glog/logging.h>

namespace dbus {

std::string MessageArgumentIterator::getStringForType(int type) {
	const char* str;
	int argType = getArgumentType();
	if (argType != type) {
		LOG(ERROR) << "Invalid argument type got " << argType <<
				" expected " << type;
		return "";
	}
	dbus_message_iter_get_basic(&iter_, &str);
	return std::string(str);
}

} /* namespace dbus */
