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
#include <glog/logging.h>
#include <RemoteMethod.h>

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
	for (auto o : objects_) {
		delete o;
	}
	objects_.clear();
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

Message Connection::sendWithReplyAndBlock(const RemoteMethod& method, int timeout_msec) {
    Message msg = method.msg();
    return sendWithReplyAndBlock(msg, timeout_msec);
}

static void buildSignalRule(const char* path, const char* interface,
		const char* method, std::string* rule) {
	rule->append("type='signal',path='");
	rule->append(path);
	rule->append("',interface='");
	rule->append(interface);
	rule->append("',member='");
	rule->append(method);
	rule->append("'");
}

void Connection::registerSignal(const char* path, const char* interface,
		const char* method) {
	DBusError err;
	std::string rule;
	buildSignalRule(path, interface, method, &rule);
	LOG(INFO) << "register signal " << rule;
	dbus_error_init(&err);
	dbus_bus_add_match(connection_, rule.c_str(), &err);
	handleError(&err, __FUNCTION__, __LINE__);
}

void Connection::unregisterSignal(const char* path, const char* interface,
		const char* method) {
	DBusError err;
	std::string rule;
	buildSignalRule(path, interface, method, &rule);
	dbus_error_init(&err);
	dbus_bus_remove_match(connection_, rule.c_str(), &err);
	handleError(&err, __FUNCTION__, __LINE__);
}

void Connection::addObject(ObjectBase* object) {
	object->registerSignals(this);
	objects_.push_back(object);
}

void Connection::removeObject(const ObjectBase* object) {
	for (auto it = objects_.begin(); it != objects_.end(); ++it) {
		if ((*it)->matchesObject(object->getPathToSelf())) {
			delete *it;
			objects_.erase(it);
			break;
		}
	}
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
    		bool handled = false;
    		int type = msg.getType();
    		if (type == DBUS_MESSAGE_TYPE_METHOD_CALL ||
    			type == DBUS_MESSAGE_TYPE_SIGNAL) {
    			ObjectPath path = msg.getPath();
				for (auto object : objects_) {
					if (object->matchesObject(path)) {
						reply = object->handleMessage(msg);
						handled = true;
						break;
					}
				}
	    		if (!handled) {
	    			LOG(WARNING) << "Message not handled: ";
	    			msg.dump();
	    			Message::forError(msg, DBUS_ERROR_UNKNOWN_OBJECT, path.str(), &reply);
	    		}
    		} else {
    			msg.dump();
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
