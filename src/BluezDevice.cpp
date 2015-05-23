/*
 * BluezDevice.cpp
 *
 *  Created on: May 22, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "BluezDevice.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "MessageArgumentIterator.h"
#include "ObjectPath.h"
#include "RemoteMethod.h"

#include <glog/logging.h>

namespace dbus {

const char* BluezDevice::INTERFACE = "org.bluez.Device";
const char* BluezDevice::GET_PROPERTIES_METHOD = "GetProperties";
const char* BluezDevice::SET_PROPERTY_METHOD = "SetProperty";
const char* BluezDevice::DISCOVER_SERVICES_METHOD = "DiscoverServices";
const char* BluezDevice::CANCEL_DISCOVERY_METHOD = "CancelDiscovery";
const char* BluezDevice::DISCONNECT_METHOD = "Disconnect";
const char* BluezDevice::LIST_NODES_METHOD = "ListNodes";
const char* BluezDevice::CREATE_NODE_METHOD = "CreateNode";
const char* BluezDevice::REMOVE_NODE_METHOD = "RemoveNode";

void BluezDevice::GetProperties(DictionaryHelper** properties) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, GET_PROPERTIES_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	*properties = NULL;
	if (reply.msg() != NULL) {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			*properties = new DictionaryHelper(&iter);
		}
	}
}

} /* namespace dbus */
