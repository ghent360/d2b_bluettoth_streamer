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
	Connection(DBusConnection* conn = NULL, bool shared = false);
	virtual ~Connection();

	static int getSystemConnection(Connection* connection);
	void flush();
	void close();

	Message sendWithReplyAndBlock(const RemoteMethod& method, int timeout_msec);
	void addObject(ObjectBase* object);
	void removeObject(const ObjectBase* object);

	void mainLoop();
	void requestTermination() {
		termination_requested_ = true;
	}

	static int handleError(DBusError *err, const char *func, int line);
	void registerSignal(const char* path, const char* interface,
			const char* method);
	void unregisterSignal(const char* path, const char* interface,
			const char* method);
private:
	Message sendWithReplyAndBlock(Message& msg, int timeout_msec);

	DBusConnection *connection_;
	bool shared_;
    std::list<ObjectBase*> objects_;
    bool termination_requested_;

	DISALLOW_COPY_AND_ASSIGN(Connection);
};

} /* namespace dbus */

#endif /* SRC_CONNECTION_H_ */
