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

#include "util.h"
#include <dbus/dbus.h>
#include <string>

namespace dbus {

class Message;

class Connection {
public:
	Connection(DBusConnection* conn = NULL, bool shared = false);
	virtual ~Connection();

	static int getSystemConnection(Connection* connection);
	void flush();
	void close();

	int getBluetoothObjectPath(const std::string& device, std::string* path);

	Message sendWithReplyAndBlock(Message& msg, int timeoutMsec);

	static int handleError(DBusError *err, const char *func, int line);
private:
	DBusConnection *connection_;
	bool shared_;

	DISALLOW_COPY_AND_ASSIGN(Connection);
};

} /* namespace dbus */

#endif /* SRC_CONNECTION_H_ */
