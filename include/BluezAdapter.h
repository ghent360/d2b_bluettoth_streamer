/*
 * BluezAdapter.h
 *
 *  Created on: NOv 18, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef BLUEZADAPTER_H_
#define BLUEZADAPTER_H_

#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <list>

namespace dbus {

class Connection;

class BluezAdapter : public SimpleObjectBase {
public:
	BluezAdapter(Connection* conn, const ObjectPath& path)
        : SimpleObjectBase(path),
		  connection_(conn) {
		interface_ = &implementation_;
	}

	std::list<ObjectPath> getDevices();
    void startDiscovery();
    void stopDiscovery();

private:
	static Message handle_DeviceFound(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);

	Connection* connection_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash GETPROPERTIES_METHOD;
	static const StringWithHash SETPROPERTY_METHOD;
	static const StringWithHash REQUESTSESSION_METHOD;
	static const StringWithHash RELEASESESSION_METHOD;
	static const StringWithHash STARTDISCOVERY_METHOD;
	static const StringWithHash STOPDISCOVERY_METHOD;
	static const StringWithHash FINDDEVICE_METHOD;
	static const StringWithHash CREATEDEVICE_METHOD;
	static const StringWithHash CREATEPAIREDDEVICE_METHOD;
	static const StringWithHash CANCELDEVICECREATION_METHOD;
	static const StringWithHash REMOVEDEVICE_METHOD;
	static const StringWithHash REGISTERAGENT_METHOD;
	static const StringWithHash UNREGISTERAGENT_METHOD;

	static const StringWithHash PROPERTYCHANGED_SIGNAL;
	static const StringWithHash DEVICEFOUND_SIGNAL;
	static const StringWithHash DEVICEDISAPPEARED_SIGNAL;
	static const StringWithHash DEVICECREATED_SIGNAL;
	static const StringWithHash DEVICEREMOVED_SIGNAL;

	static const char* ADDRESS_PROPERTY;
	static const char* NAME_PROPERTY;
	static const char* CLASS_PROPERTY;
	static const char* POWERED_PROPERTY;
	static const char* DISCOVERABLE_PROPERTY;
	static const char* PAIRABLE_PROPERTY;
	static const char* PAIRABLETIMEOUT_PROPERTY;
	static const char* DISCOVERABLETIMEOUT_PROPERTY;
	static const char* DISCOVERING_PROPERTY;
	static const char* DEVICES_PROPERTY;
	static const char* UUIDS_PROPERTY;

	static const MethodDescriptor interfaceMethods_[];
	static const MethodDescriptor interfaceSignals_[];
	static const PropertyDescriptor interfaceProperties_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(BluezAdapter);
};

} /* namespace dbus */

#endif /* BLUEZADAPTER_H_ */
