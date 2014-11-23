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
#include <googleapis/base/callback.h>
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
	void send(const RemoteMethod& method, int timeout_msec,
			googleapis::Callback1<Message*>* cb);
	void addObject(ObjectBase* object);
	void removeObject(const ObjectBase* object);

	void process(int time_out);
	void processTimeouts();

	static int handleError(DBusError *err, const char *func, int line);
	void registerSignal(const char* path, const char* interface,
			const char* method);
	void unregisterSignal(const char* path, const char* interface,
			const char* method);
private:
	Message sendWithReplyAndBlock(Message& msg, int timeout_msec);
	void send(Message& msg, int timeout_msec,
			googleapis::Callback1<Message*>* cb);

	struct PendingResponse {
		dbus_uint32_t serial;
		bool has_expiration;
		uint32_t expiration;
		googleapis::Callback1<Message*>* cb;
	};

	DBusConnection *connection_;
	bool shared_;
    std::list<ObjectBase*> objects_;
    std::list<PendingResponse> pending_responses_;

	DISALLOW_COPY_AND_ASSIGN(Connection);
};

} /* namespace dbus */

#endif /* SRC_CONNECTION_H_ */
