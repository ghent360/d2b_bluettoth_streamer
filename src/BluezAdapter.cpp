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
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "MethodBase.h"

namespace dbus {
const char* BluezAdapter::INTERFACE = "org.bluez.Adapter";

const char* BluezAdapter::GETPROPERTIES_METHOD = "GetProperties";
const char* BluezAdapter::SETPROPERTY_METHOD = "SetProperty";
const char* BluezAdapter::REQUESTSESSION_METHOD = "RequestSession";
const char* BluezAdapter::RELEASESESSION_METHOD = "ReleaseSession";
const char* BluezAdapter::STARTDISCOVERY_METHOD = "StartDiscovery";
const char* BluezAdapter::STOPDISCOVERY_METHOD = "StopDiscovery";
const char* BluezAdapter::FINDDEVICE_METHOD = "FindDevice";
const char* BluezAdapter::CREATEDEVICE_METHOD = "CreateDevice";
const char* BluezAdapter::CREATEPAIREDDEVICE_METHOD = "CreatePairedDevice";
const char* BluezAdapter::CANCELDEVICECREATION_METHOD = "CancelDeviceCreation";
const char* BluezAdapter::REMOVEDEVICE_METHOD = "RemoveDevice";
const char* BluezAdapter::REGISTERAGENT_METHOD = "RegisterAgent";
const char* BluezAdapter::UNREGISTERAGENT_METHOD = "UnregisterAgent";

const char* BluezAdapter::PROPERTYCHANGED_SIGNAL = "PropertyChanged";
const char* BluezAdapter::DEVICEFOUND_SIGNAL = "DeviceFound";
const char* BluezAdapter::DEVICEDISAPPEARED_SIGNAL = "DeviceDisappeared";
const char* BluezAdapter::DEVICECREATED_SIGNAL = "DeviceCreated";
const char* BluezAdapter::DEVICEREMOVED_SIGNAL = "DeviceRemoved";

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

} /* namespace dbus */
