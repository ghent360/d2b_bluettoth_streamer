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

#include "ObjectPath.h"
#include "util.h"

namespace dbus {

class Connection;

class BluezAdapter {
public:
	BluezAdapter(Connection* connection, const ObjectPath& path)
	    : connection_(connection),
	      path_(path) {
	}

private:
	static const char* INTERFACE;
	static const char* GETPROPERTIES_METHOD;
	static const char* SETPROPERTY_METHOD;
	static const char* REQUESTSESSION_METHOD;
	static const char* RELEASESESSION_METHOD;
	static const char* STARTDISCOVERY_METHOD;
	static const char* STOPDISCOVERY_METHOD;
	static const char* FINDDEVICE_METHOD;
	static const char* CREATEDEVICE_METHOD;
	static const char* CREATEPAIREDDEVICE_METHOD;
	static const char* CANCELDEVICECREATION_METHOD;
	static const char* REMOVEDEVICE_METHOD;
	static const char* REGISTERAGENT_METHOD;
	static const char* UNREGISTERAGENT_METHOD;

	static const char* PROPERTYCHANGED_SIGNAL;
	static const char* DEVICEFOUND_SIGNAL;
	static const char* DEVICEDISAPPEARED_SIGNAL;
	static const char* DEVICECREATED_SIGNAL;
	static const char* DEVICEREMOVED_SIGNAL;

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

	Connection* connection_;
	ObjectPath path_;
	DISALLOW_COPY_AND_ASSIGN(BluezAdapter);
};

} /* namespace dbus */

#endif /* BLUEZADAPTER_H_ */
