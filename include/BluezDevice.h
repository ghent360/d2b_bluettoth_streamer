/*
 * BluezDevice.h
 *
 *  Created on: May 22, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef BLUEZDEVICE_H_
#define BLUEZDEVICE_H_

#include "ObjectBase.h"
#include "ObjectPath.h"
#include "util.h"

namespace dbus {

class Connection;
class DictionaryHelper;
class ObjectPath;
class Message;

class BluezDevice : public SimpleObjectBase {
public:
	BluezDevice(Connection* connection, const ObjectPath& path)
        : SimpleObjectBase(path),
		  connection_(connection) {
	}

	void GetProperties(DictionaryHelper** properties);
	void SetProperty(const char* name, int32_t value);
	void DiscoverServices(const char* pattern, DictionaryHelper** services);
	void CancelDiscovery();
	void Disconnect();

private:
	Connection* connection_;

	// DBus metadata
	static const char* INTERFACE;
	static const char* GET_PROPERTIES_METHOD;
	static const char* SET_PROPERTY_METHOD;
	static const char* DISCOVER_SERVICES_METHOD;
	static const char* CANCEL_DISCOVERY_METHOD;
	static const char* DISCONNECT_METHOD;
	static const char* LIST_NODES_METHOD;
	static const char* CREATE_NODE_METHOD;
	static const char* REMOVE_NODE_METHOD;

	DISALLOW_COPY_AND_ASSIGN(BluezDevice);
};

} /* namespace dbus */

#endif /* BLUEZMANAGER_H_ */
