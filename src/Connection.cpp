/*
 * DbusConnection.cpp
 *
 *  Created on: Aug 30, 2014
 *      Author: vne-marti
 */

#include "Connection.h"
#include "Message.h"

#include <glog/logging.h>

namespace dbus {

Connection::Connection(DBusConnection* connection, bool shared)
    : connection_(connection),
      shared_(shared) {
}

Connection::~Connection() {
	if (connection_) {
      close();
	}
}

void Connection::close() {
	if (connection_) {
		flush();
		if (shared_) {
			dbus_connection_unref(connection_);
		} else {
			dbus_connection_close(connection_);
		}
	}
	connection_ = NULL;
	shared_ = false;
}

void Connection::flush() {
    dbus_connection_flush(connection_);
}

int Connection::handleError (DBusError *err, const char *func, int line) {
    if (dbus_error_is_set (err)) {
        LOG(ERROR) << "DBus error at " << func << "@" << line << " :" << err->message;
        dbus_error_free(err);
        return 1;
    }
    return 0;
}

int Connection::getSystemConnection(Connection* connection) {
    DBusError err;
    DBusConnection* conn;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    handleError(&err, __FUNCTION__, __LINE__);
    if (NULL == conn) return 1;

    connection->close();
    connection->connection_ = conn;
    connection->shared_ = true;
    LOG(INFO) << "Name " << dbus_bus_get_unique_name(conn);
    return 0;
}

Message Connection::sendWithReplyAndBlock(Message& msg, int timeoutMsec) {
	DBusMessage* reply;
	DBusError err;

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection_, msg.msg(), timeoutMsec, &err);
	handleError(&err, __FUNCTION__, __LINE__);
	return reply;
}

} /* namespace dbus */
