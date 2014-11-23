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
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "RemoteMethod.h"

#include <glog/logging.h>

namespace dbus {
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

std::list<ObjectPath> BluezAdapter::getDevices() {
	RemoteMethod rpc(ORG_BLUEZ, getPathToSelf().str(), INTERFACE.str(), GETPROPERTIES_METHOD.str());
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	std::list<ObjectPath> result;
	if (!reply.msg()) {
		LOG(ERROR) << "Error calling " << INTERFACE.str() << "." << GETPROPERTIES_METHOD.str();
	} else {
		MessageArgumentIterator iter = reply.argIterator();
		if (iter.hasArgs()) {
			DictionaryHelper dict(&iter);
			auto adapter_iter = dict.getArray(DEVICES_PROPERTY).recurse();
			while (DBUS_TYPE_OBJECT_PATH == adapter_iter.getArgumentType()) {
				ObjectPath adapter = adapter_iter.getObjectPath();
				result.push_back(adapter);
				adapter_iter.next();
			};
		}
	}
	return result;
}

const MethodDescriptor BluezAdapter::interfaceMethods_[] = {
};

const MethodDescriptor BluezAdapter::interfaceSignals_[] = {
	MethodDescriptor(PROPERTYCHANGED_SIGNAL, default_PropertyChange_handler),
};

const PropertyDescriptor BluezAdapter::interfaceProperties_[] = {
    //PropertyDescriptor(STATE_PROPERTY, handle_stateChanged)
};

const InterfaceImplementation BluezAdapter::implementation_(INTERFACE,
		interfaceMethods_, interfaceSignals_, interfaceProperties_);

} /* namespace dbus */
