/*
 * Serial.cpp
 *
 *  Created on: Jun 27, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "Serial.h"
#include "BluezNames.h"
#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "RemoteMethod.h"

#include <dbus/dbus.h>
#include <glog/logging.h>

namespace dbus {
Serial::Serial(Connection* connection, const ObjectPath& path)
    : SimpleObjectBase(path),
	  connection_(connection) {
	interface_ = &implementation_;
}

Serial::~Serial() {
}

void Serial::connect(const char* prefix, std::string* result) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(prefix);
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	result->clear();
	if (reply.msg() != NULL &&
		reply.getType() == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			result->assign(iter.getString());
		}
	}
}

void Serial::connectAsync(const char* prefix,
		int timeout,
		googleapis::Callback1<Message*>* cb) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, CONNECT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(prefix);
	connection_->send(rpc, timeout, cb);
}

void Serial::disconnect(const char* device) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, DISCONNECT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(device);
	connection_->sendWithReplyAndBlock(rpc, -1);
}

void Serial::disconnectAsync(const char* device,
		int timeout,
		googleapis::Callback1<Message*>* cb) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, DISCONNECT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(device);
	connection_->send(rpc, timeout, cb);
}

const StringWithHash Serial::INTERFACE("org.bluez.Serial");
const StringWithHash Serial::CONNECT_METHOD("Connect");
const StringWithHash Serial::DISCONNECT_METHOD("Disconnect");

const MethodDescriptor Serial::interfaceMethods_[] = {
};

const InterfaceImplementation Serial::implementation_(INTERFACE,
		interfaceMethods_);

} /* namespace dbus */
