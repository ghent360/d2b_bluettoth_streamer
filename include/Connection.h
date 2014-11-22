/*
 * DbusConnection.h
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef SRC_CONNECTION_H_
#define SRC_CONNECTION_H_

#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <dbus/dbus.h>
#include <list>

namespace dbus {

class Message;

class Connection {
public:
	Connection(DBusConnection* conn = NULL, bool shared = false,
			const char* app_object_path_prefix = "");
	virtual ~Connection();

	static int getSystemConnection(Connection* connection);
	void flush();
	void close();

	Message sendWithReplyAndBlock(const MethodBase& method, int timeout_msec);
	void addObject(ObjectBase* object);
	void removeObject(const ObjectBase* object);

	void mainLoop();
	void requestTermination() {
		termination_requested_ = true;
	}

	static int handleError(DBusError *err, const char *func, int line);
	ObjectPath makeObjectPath(const void* object);
private:
	Message sendWithReplyAndBlock(Message& msg, int timeout_msec);

	DBusConnection *connection_;
	bool shared_;
    std::list<ObjectBase*> objects_;
    bool termination_requested_;
    const char* app_object_path_prefix_;
    size_t app_object_path_prefix_len_;

	DISALLOW_COPY_AND_ASSIGN(Connection);
};

} /* namespace dbus */

#endif /* SRC_CONNECTION_H_ */
