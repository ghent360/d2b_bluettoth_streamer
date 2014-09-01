/*
 * DbusConnection.cpp
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "Connection.h"
#include "Message.h"
#include "MethodBase.h"
#include "MethodLocator.h"

#include <glog/logging.h>

namespace dbus {

Connection::Connection(DBusConnection* connection, bool shared)
    : connection_(connection),
      shared_(shared),
      termination_requested_(false) {
}

Connection::~Connection() {
	if (connection_) {
      close();
	}
}

void Connection::close() {
	for (auto handler : handlers_) {
		delete handler;
	}
	handlers_.clear();
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
        LOG(ERROR) << "DBus error at " << func << "@" << line << " :" <<
        		err->message;
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

Message Connection::sendWithReplyAndBlock(Message& msg, int timeout_msec) {
	DBusMessage* reply;
	DBusError err;

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(connection_,
			msg.msg(),
			timeout_msec,
			&err);
	handleError(&err, __FUNCTION__, __LINE__);
	return reply;
}

Message Connection::sendWithReplyAndBlock(const MethodBase& method, int timeout_msec) {
    Message msg = method.msg();
    return sendWithReplyAndBlock(msg, timeout_msec);
}

void Connection::addMethodHandler(MethodLocator* handler) {
	handlers_.push_back(handler);
}

void Connection::removeMethodHandler(MethodLocator* handler) {
	handlers_.remove(handler);
}

void Connection::mainLoop() {
	termination_requested_ = false;
    while (!termination_requested_) {
    	if (dbus_connection_read_write (connection_, 200) == FALSE) {
    		break;
    	}
    	Message msg(dbus_connection_pop_message (connection_));
    	while (msg.msg() != NULL && !termination_requested_) {
    		Message reply;
    		for (auto handler : handlers_) {
    			if (handler->matches(msg)) {
                    reply = handler->handle(msg);
                    break;
    			}
    		}
    		if (reply.msg() != NULL) {
                uint32_t serial = msg.getSerial();
    			dbus_connection_send(connection_, reply.msg(), &serial);
    		}
    		msg.takeOwnership(dbus_connection_pop_message (connection_));
    	}
    }
}
} /* namespace dbus */
