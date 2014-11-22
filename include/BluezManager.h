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

#include <list>

namespace dbus {

class Connection;
class ObjectPath;
class Message;

class BluezManager {
public:
	BluezManager(Connection* connection) : connection_(connection) {}

	ObjectPath defaultAdapter();
	ObjectPath findAdapter(const char* pattern);
    std::list<ObjectPath> getAdapters();

private:
	ObjectPath returnObjectPath(Message& msg, const char* method_name);

	Connection* connection_;

	// DBus metadata
	static const char* INTERFACE;
	static const char* PATH;
	static const char* DEFAULT_ADAPTER_METHOD;
	static const char* FIND_ADAPTER_METHOD;
	static const char* GET_PROPERTIES_METHOD;
	static const char* ADAPTERS_PROPERTY;

	DISALLOW_COPY_AND_ASSIGN(BluezManager);
};

} /* namespace dbus */

#endif /* BLUEZMANAGER_H_ */
