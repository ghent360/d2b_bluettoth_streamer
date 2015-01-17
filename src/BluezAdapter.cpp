/*
 * BluezAdapter.cpp
 *
 *  Created on: Nov 18, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "BluezAdapter.h"
#include "BluezAgent.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "RemoteMethod.h"

#include <dbus/dbus.h>
#include <glog/logging.h>

namespace dbus {

void BluezAdapter::startDiscovery() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, STARTDISCOVERY_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
	discovering_ = true;
}

void BluezAdapter::stopDiscovery() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, STOPDISCOVERY_METHOD);
	rpc.prepareCall();
	connection_->sendWithReplyAndBlock(rpc, -1);
	discovering_ = false;
}

void BluezAdapter::setDiscoverable(bool value) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SETPROPERTY_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(DISCOVERABLE_PROPERTY);
	args.appendVariant(value);
	connection_->sendWithReplyAndBlock(rpc, -1);
	discoverable_ = value;
}

void BluezAdapter::setPairable(bool value) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SETPROPERTY_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(PAIRABLE_PROPERTY);
	args.appendVariant(value);
	connection_->sendWithReplyAndBlock(rpc, -1);
	pairable_ = value;
}

void BluezAdapter::setPairableTimeout(uint32_t seconds) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SETPROPERTY_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(PAIRABLETIMEOUT_PROPERTY);
	args.appendVariant(seconds);
	connection_->sendWithReplyAndBlock(rpc, -1);
	pairable_timeout_ = seconds;
}

void BluezAdapter::setDiscoverableTimeout(uint32_t seconds) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SETPROPERTY_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(DISCOVERABLETIMEOUT_PROPERTY);
	args.appendVariant(seconds);
	connection_->sendWithReplyAndBlock(rpc, -1);
	discoverable_timeout_ = seconds;
}

void BluezAdapter::setName(const char* name) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, SETPROPERTY_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(NAME_PROPERTY);
	args.appendVariant(name);
	connection_->sendWithReplyAndBlock(rpc, -1);
	name_ = name;
}

void BluezAdapter::registerAgent(const ObjectPath& path, const char* capability) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, REGISTERAGENT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(path);
	args.append(capability);
	Message response = connection_->sendWithReplyAndBlock(rpc, -1);
	response.dump("registerAgent:");
}

void BluezAdapter::unregisterAgent(const ObjectPath& path) {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf(), INTERFACE, UNREGISTERAGENT_METHOD);
	rpc.prepareCall();
	auto args = rpc.argBuilder();
	args.append(path);
	Message response = connection_->sendWithReplyAndBlock(rpc, -1);
	response.dump("unregisterAgent:");
}

void BluezAdapter::registerAgent(BluezAgent* agent) {
	registerAgent(agent->getPathToSelf(), agent->getCapabilities());
}

void BluezAdapter::unregisterAgent(BluezAgent* agent) {
	unregisterAgent(agent->getPathToSelf());
}

const StringWithHash BluezAdapter::INTERFACE = "org.bluez.Adapter";

const StringWithHash BluezAdapter::GETPROPERTIES_METHOD = "GetProperties";
const StringWithHash BluezAdapter::SETPROPERTY_METHOD = "SetProperty";
const StringWithHash BluezAdapter::REQUESTSESSION_METHOD = "RequestSession";
const StringWithHash BluezAdapter::RELEASESESSION_METHOD = "ReleaseSession";
const StringWithHash BluezAdapter::STARTDISCOVERY_METHOD = "StartDiscovery";
const StringWithHash BluezAdapter::STOPDISCOVERY_METHOD = "StopDiscovery";
const StringWithHash BluezAdapter::FINDDEVICE_METHOD = "FindDevice";
const StringWithHash BluezAdapter::CREATEDEVICE_METHOD = "CreateDevice";
const StringWithHash BluezAdapter::CREATEPAIREDDEVICE_METHOD = "CreatePairedDevice";
const StringWithHash BluezAdapter::CANCELDEVICECREATION_METHOD = "CancelDeviceCreation";
const StringWithHash BluezAdapter::REMOVEDEVICE_METHOD = "RemoveDevice";
const StringWithHash BluezAdapter::REGISTERAGENT_METHOD = "RegisterAgent";
const StringWithHash BluezAdapter::UNREGISTERAGENT_METHOD = "UnregisterAgent";

const StringWithHash BluezAdapter::PROPERTYCHANGED_SIGNAL = "PropertyChanged";
const StringWithHash BluezAdapter::DEVICEFOUND_SIGNAL = "DeviceFound";
const StringWithHash BluezAdapter::DEVICEDISAPPEARED_SIGNAL = "DeviceDisappeared";
const StringWithHash BluezAdapter::DEVICECREATED_SIGNAL = "DeviceCreated";
const StringWithHash BluezAdapter::DEVICEREMOVED_SIGNAL = "DeviceRemoved";

const char* BluezAdapter::ADDRESS_PROPERTY = "Address";
const char* BluezAdapter::NAME_PROPERTY = "Name";
const char* BluezAdapter::CLASS_PROPERTY = "Class";
const char* BluezAdapter::POWERED_PROPERTY = "Powered";
const char* BluezAdapter::DISCOVERABLE_PROPERTY = "Discoverable";
const char* BluezAdapter::PAIRABLE_PROPERTY = "Pairable";
const char* BluezAdapter::PAIRABLETIMEOUT_PROPERTY = "PairableTimeout";
const char* BluezAdapter::DISCOVERABLETIMEOUT_PROPERTY = "DiscoverableTimeout";
const char* BluezAdapter::DISCOVERING_PROPERTY = "Discovering";
const char* BluezAdapter::DEVICES_PROPERTY = "Devices";
const char* BluezAdapter::UUIDS_PROPERTY = "UUIDs";

void BluezAdapter::refreshProperties() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf().str(), INTERFACE.str(), GETPROPERTIES_METHOD.str());
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg()) {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			DictionaryHelper dict(&iter);

			//dict.dump("Adapter properties:");
			devices_.clear();
			auto devices_iter = dict.getArray(DEVICES_PROPERTY).recurse();
			while (DBUS_TYPE_OBJECT_PATH == devices_iter.getArgumentType()) {
				ObjectPath adapter = devices_iter.getObjectPath();
				devices_.push_back(adapter);
				devices_iter.next();
			};

			uuids_.clear();
			auto uuid_iter = dict.getArray(UUIDS_PROPERTY).recurse();
			while (DBUS_TYPE_STRING == uuid_iter.getArgumentType()) {
				const char* uuid = uuid_iter.getString();
				uuids_.push_back(uuid);
				uuid_iter.next();
			};

			powered_ = dict.getBool(POWERED_PROPERTY);
			discovering_ = dict.getBool(DISCOVERING_PROPERTY);
			discoverable_ = dict.getBool(DISCOVERABLE_PROPERTY);
			pairable_ = dict.getBool(PAIRABLE_PROPERTY);

			discoverable_timeout_ = dict.getUint32(DISCOVERABLETIMEOUT_PROPERTY);
			pairable_timeout_ = dict.getUint32(PAIRABLETIMEOUT_PROPERTY);
			class_ = dict.getUint32(CLASS_PROPERTY);

			name_ = dict.getString(NAME_PROPERTY);
			address_ = dict.getString(ADDRESS_PROPERTY);
		}
	} else {
		LOG(ERROR) << "Error calling " << INTERFACE.str() << "." << GETPROPERTIES_METHOD.str();
	}
}

Message BluezAdapter::handle_DeviceFound(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAdapter* pThis = reinterpret_cast<BluezAdapter*>(ctx);
	if (NULL != pThis->device_found_cb_) {
		MessageArgumentIterator it = msg.argIterator();
		if (it.hasArgs() && it.getArgumentType() == DBUS_TYPE_STRING) {
			const char* address = it.getString();
			it.next();
			if (it.getArgumentType() == DBUS_TYPE_ARRAY) {
				bool once = !pThis->device_found_cb_->IsRepeatable();
				pThis->device_found_cb_->Run(address, &it);
				if (once) {
					pThis->device_found_cb_ = NULL;
				}
			}
		}
	}
	return Message();
}

Message BluezAdapter::handle_DeviceDisappeared(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAdapter* pThis = reinterpret_cast<BluezAdapter*>(ctx);
	if (NULL != pThis->device_disappeared_cb_) {
		MessageArgumentIterator it = msg.argIterator();
		if (it.hasArgs() && it.getArgumentType() == DBUS_TYPE_STRING) {
			const char* address = it.getString();
			bool once = !pThis->device_disappeared_cb_->IsRepeatable();
			pThis->device_disappeared_cb_->Run(address);
			if (once) {
				pThis->device_disappeared_cb_ = NULL;
			}
		}
	}
	return Message();
}

Message BluezAdapter::handle_DeviceCreated(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAdapter* pThis = reinterpret_cast<BluezAdapter*>(ctx);
	if (NULL != pThis->device_created_cb_) {
		MessageArgumentIterator it = msg.argIterator();
		if (it.hasArgs() && it.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
			ObjectPath device = it.getObjectPath();
			bool once = !pThis->device_created_cb_->IsRepeatable();
			pThis->device_created_cb_->Run(device);
			if (once) {
				pThis->device_created_cb_ = NULL;
			}
		}
	}
	return Message();
}

Message BluezAdapter::handle_DeviceRemoved(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAdapter* pThis = reinterpret_cast<BluezAdapter*>(ctx);
	if (NULL != pThis->device_removed_cb_) {
		MessageArgumentIterator it = msg.argIterator();
		if (it.hasArgs() && it.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
			ObjectPath device = it.getObjectPath();
			bool once = !pThis->device_removed_cb_->IsRepeatable();
			pThis->device_removed_cb_->Run(device);
			if (once) {
				pThis->device_removed_cb_ = NULL;
			}
		}
	}
	return Message();
}

const MethodDescriptor BluezAdapter::interfaceMethods_[] = {
};

const MethodDescriptor BluezAdapter::interfaceSignals_[] = {
	//MethodDescriptor(PROPERTYCHANGED_SIGNAL, propertyChange_noError_handler),
	MethodDescriptor(DEVICEFOUND_SIGNAL, handle_DeviceFound),
	MethodDescriptor(DEVICEDISAPPEARED_SIGNAL, handle_DeviceDisappeared),
	MethodDescriptor(DEVICECREATED_SIGNAL, handle_DeviceCreated),
	MethodDescriptor(DEVICEREMOVED_SIGNAL, handle_DeviceRemoved),
};

const PropertyDescriptor BluezAdapter::interfaceProperties_[] = {
    //PropertyDescriptor(STATE_PROPERTY, handle_stateChanged)
};

const InterfaceImplementation BluezAdapter::implementation_(INTERFACE,
		interfaceMethods_, interfaceSignals_, interfaceProperties_);

} /* namespace dbus */
