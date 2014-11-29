/*
 * BluezAdapter.h
 *
 *  Created on: Nov 18, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef BLUEZADAPTER_H_
#define BLUEZADAPTER_H_

#include "MessageArgumentIterator.h"
#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <googleapis/base/callback.h>
#include <list>

namespace dbus {

class Connection;

class BluezAdapter : public SimpleObjectBase {
public:
	typedef googleapis::Callback2<const char*, BaseMessageIterator*> DeviceFoundCallback;
	typedef googleapis::Callback1<const char*> DeviceDisappearedCallback;
	typedef googleapis::Callback1<const ObjectPath&> DeviceListCallback;

	BluezAdapter(Connection* conn, const ObjectPath& path)
        : SimpleObjectBase(path),
		  connection_(conn),
		  device_found_cb_(NULL),
		  device_disappeared_cb_(NULL),
		  device_created_cb_(NULL),
		  device_removed_cb_(NULL),
		  class_(0),
		  powered_(false),
		  discoverable_(false),
		  pairable_(false),
		  discovering_(false),
		  pairable_timeout_(0),
		  discoverable_timeout_(0) {
		interface_ = &implementation_;
	}
	~BluezAdapter() {
		delete device_found_cb_;
		delete device_disappeared_cb_;
		delete device_created_cb_;
		delete device_removed_cb_;
	}

    void startDiscovery();
    void stopDiscovery();

    void setDiscoverable(bool value);
    void setPairable(bool value);
    void setPairableTimeout(uint32_t seconds);
    void setDiscoverableTimeout(uint32_t seconds);
    void setName(const char* name);

	void refreshProperties();

	const std::list<ObjectPath> getDevices() const {
		return devices_;
	}

    bool getDiscovering() const {
    	return discovering_;
    }
    bool getDiscoverable() const {
    	return discoverable_;
    }
    bool getPairable() const {
    	return pairable_;
    }
    bool getPowered() const {
    	return powered_;
    }
    uint32_t getClass() const {
    	return class_;
    }

    void setDeviceFoundCallback(DeviceFoundCallback* cb) {
   		delete device_found_cb_;
    	device_found_cb_ = cb;
    }

    void setDeviceDisappearedCallback(DeviceDisappearedCallback* cb) {
   		delete device_disappeared_cb_;
    	device_disappeared_cb_ = cb;
    }

    void setDeviceCreatedCallback(DeviceListCallback* cb) {
   		delete device_created_cb_;
   		device_created_cb_ = cb;
    }

    void setDeviceRemovedCallback(DeviceListCallback* cb) {
   		delete device_removed_cb_;
   		device_removed_cb_ = cb;
    }

private:
	static Message handle_DeviceFound(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_DeviceDisappeared(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_DeviceCreated(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_DeviceRemoved(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);

	Connection* connection_;
	DeviceFoundCallback* device_found_cb_;
	DeviceDisappearedCallback* device_disappeared_cb_;
	DeviceListCallback* device_created_cb_;
	DeviceListCallback* device_removed_cb_;

	std::list<ObjectPath> devices_;
	std::string name_;
	std::string address_;
	uint32_t class_;
	bool powered_;
	bool discoverable_;
	bool pairable_;
	bool discovering_;
	uint32_t pairable_timeout_;
	uint32_t discoverable_timeout_;
	std::list<std::string> uuids_;

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
