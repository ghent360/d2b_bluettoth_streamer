/*
 * MethodLocator.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "MethodLocator.h"

#include "Message.h"

#include <dbus/dbus.h>

namespace dbus {

MethodLocator::MethodLocator(Type type,
		const char* interface,
		const char* method)
    : type_(type),
      interface_(interface),
      method_(method) {
}

bool MethodLocator::matches(Message& msg) {
    if (type_ == E_SIGNAL) {
    	return dbus_message_is_signal(msg.msg(),
    			interface_.c_str(),
    			method_.c_str()) == TRUE;
    }
	return dbus_message_is_method_call(msg.msg(),
			interface_.c_str(),
			method_.c_str()) == TRUE;
}

bool MethodLocator::operator == (const MethodLocator &other) {
    return type_ == other.type_ &&
    		interface_ == other.interface_ &&
    		method_ == other.method_;
}

} /* namespace dbus */
