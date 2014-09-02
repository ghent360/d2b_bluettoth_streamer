/*
 * BluezManager.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef BLUEZMANAGER_H_
#define BLUEZMANAGER_H_

#include "util.h"

namespace dbus {

class Connection;
class ObjectPath;
class Message;

class BluezManager {
public:
	BluezManager(Connection* connection) : connection_(connection) {}

	ObjectPath defaultAdapter();
	ObjectPath findAdapter(const char* pattern);

private:
	static const char* INTERFACE;
	static const char* PATH;
	static const char* DEFAULT_ADAPTER_METHOD;
	static const char* FIND_ADAPTER_METHOD;

	ObjectPath returnObjectPath(Message& msg, const char* method_name);

	Connection* connection_;
	DISALLOW_COPY_AND_ASSIGN(BluezManager);
};

} /* namespace dbus */

#endif /* BLUEZMANAGER_H_ */
