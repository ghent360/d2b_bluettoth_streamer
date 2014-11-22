/*
 * BluezManager.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "BluezManager.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "MessageArgumentIterator.h"
#include "ObjectPath.h"
#include "RemoteMethod.h"

#include <glog/logging.h>

namespace dbus {

const char* BluezManager::INTERFACE = "org.bluez.Manager";
const char* BluezManager::PATH = "/";
const char* BluezManager::DEFAULT_ADAPTER_METHOD = "DefaultAdapter";
const char* BluezManager::FIND_ADAPTER_METHOD = "FindAdapter";
const char* BluezManager::GET_PROPERTIES_METHOD = "GetProperties";
const char* BluezManager::ADAPTERS_PROPERTY = "Adapters";

ObjectPath BluezManager::defaultAdapter() {
	RemoteMethod rpc(ORG_BLUEZ, PATH, INTERFACE, DEFAULT_ADAPTER_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return returnObjectPath(reply, DEFAULT_ADAPTER_METHOD);
}

ObjectPath BluezManager::findAdapter(const char* pattern) {
	RemoteMethod rpc(ORG_BLUEZ, PATH, INTERFACE, FIND_ADAPTER_METHOD);
	rpc.prepareCall();
	MessageArgumentBuilder args = rpc.argBuilder();
	args.append(pattern);
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return returnObjectPath(reply, FIND_ADAPTER_METHOD);
}

ObjectPath BluezManager::returnObjectPath(Message& msg,
		const char* method_name) {
	if (!msg.msg()) {
		LOG(ERROR) << "Error calling " << INTERFACE << "." <<
				method_name;
		return ObjectPath("");
	}
	MessageArgumentIterator iter = msg.argIterator();
	if (iter.hasArgs() && DBUS_TYPE_OBJECT_PATH == iter.getArgumentType()) {
		return iter.getObjectPath();
	}
	LOG(ERROR) << "Error calling " << INTERFACE << "." <<
			method_name << " got incorrect result type.";
	return ObjectPath("");
}

std::list<ObjectPath> BluezManager::getAdapters() {
	RemoteMethod rpc(ORG_BLUEZ, PATH, INTERFACE, GET_PROPERTIES_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	std::list<ObjectPath> result;
	if (!reply.msg()) {
		LOG(ERROR) << "Error calling " << INTERFACE << "." << GET_PROPERTIES_METHOD;
	} else {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			DictionaryHelper dict(&iter);
			auto adapter_iter = dict.getArray(ADAPTERS_PROPERTY).recurse();
			while (DBUS_TYPE_OBJECT_PATH == adapter_iter.getArgumentType()) {
				ObjectPath adapter = adapter_iter.getObjectPath();
				result.push_back(adapter);
				adapter_iter.next();
			};
		}
	}
	return result;
}

} /* namespace dbus */
