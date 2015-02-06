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
#include "time_util.h"

#include <glog/logging.h>
#include <RemoteMethod.h>

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
    //LOG(INFO) << "Name " << dbus_bus_get_unique_name(conn);
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
	if (dbus_error_is_set(&err)) {
		msg.dump("Error calling: ");
	}
	handleError(&err, __FUNCTION__, __LINE__);
	return reply;
}

Message Connection::sendWithReplyAndBlock(const RemoteMethod& method, int timeout_msec) {
    Message msg = method.msg();
    return sendWithReplyAndBlock(msg, timeout_msec);
}

void Connection::send(const RemoteMethod& method, int timeout_msec,
		googleapis::Callback1<Message*>* cb) {
    Message msg = method.msg();
    send(msg, timeout_msec, cb);
}

void Connection::send(Message& msg, int timeout_msec,
		googleapis::Callback1<Message*>* cb) {
	PendingResponse response;

	response.has_expiration = false;
	response.cb = cb;
	if (timeout_msec >= 0) {
		response.has_expiration = true;
		response.expiration = timeGetTime() + timeout_msec;
	}
	dbus_connection_send(connection_, msg.msg(), &response.serial);
	pending_responses_.push_back(response);
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
	//LOG(INFO) << "register signal " << rule;
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

void Connection::process(int time_out) {
	if (dbus_connection_read_write (connection_, time_out) == FALSE) {
		return;
	}
	Message msg(dbus_connection_pop_message (connection_));
	while (msg.msg() != NULL) {
		Message reply;
		bool handled = false;
		int type = msg.getType();
		//msg.dump("Incomming:");
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
				msg.dump("Message not handled: ");
				if (type == DBUS_MESSAGE_TYPE_METHOD_CALL) {
					Message::forError(msg, DBUS_ERROR_UNKNOWN_OBJECT, path.str(), &reply);
				}
			}
		} else {
			uint32_t serial = msg.getReplySerial();
			bool found = false;
			for (auto it = pending_responses_.begin(); it != pending_responses_.end(); ++it) {
				if (it->serial == serial) {
					if (NULL != it->cb) it->cb->Run(&msg);
					found = true;
					pending_responses_.erase(it);
					break;
				}
			}
			if (!found) {
				msg.dump("Reply not handled: ");
			}
		}
		if (!msg.getNoReply() && reply.msg() == NULL) {
			Message::forMethodReturn(msg, &reply);
		}
		if (reply.msg() != NULL) {
			uint32_t serial;
			dbus_connection_send(connection_, reply.msg(), &serial);
		}
		msg.takeOwnership(dbus_connection_pop_message (connection_));
	}
	processTimeouts();
}

void Connection::processTimeouts() {
	uint32_t time = timeGetTime();
	for (const auto& p : pending_responses_) {
		if (p.has_expiration && p.expiration > time) {
			p.cb->Run(NULL);
		}
	}
}

} /* namespace dbus */
