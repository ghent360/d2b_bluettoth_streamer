/*
 * MediaEndpoint.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MEDIAENDPOINT_H_
#define MEDIAENDPOINT_H_

#include "InterfaceImplementation.h"
#include "MediaTransportProperties.h"
#include "Message.h"
#include "ObjectBase.h"
#include "ObjectPath.h"
#include "util.h"

namespace dbus {

#define A2DP_SINK_UUID        "0000110b-0000-1000-8000-00805f9b34fb"

class MediaEndpoint : public SimpleObjectBase {
public:
	MediaEndpoint(const ObjectPath&);
	virtual ~MediaEndpoint() {}

	bool isTransportConfigValid() const {
		return transport_config_valid_;
	}

	const ObjectPath& getTransportPath() const {
		return transport_path_;
	}

	virtual const char* getUuid() const = 0;
	virtual uint8_t getCodecId() const = 0;
	virtual bool getCapabilities(uint8_t* capabilities,
			size_t* capabilities_max_len) const = 0;
protected:
	virtual bool selectConfiguration(void* capabilities,
			size_t capabilities_len,
			uint8_t** selected_capabilities,
			size_t* selected_capabilities_len) = 0;

	virtual void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties);
	virtual void clearConfiguration(const ObjectPath& transport);
	virtual void release();

	bool transport_config_valid_;
	ObjectPath transport_path_;
	MediaTransportProperties transport_properties_;

private:
	static Message handle_selectConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_setConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_clearConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_release(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);

	// DBus metadata
	static const StringWithHash INTERFACE;

	static const StringWithHash SETCONFIGURATION_METHOD;
	static const StringWithHash SELECTCONFIGURATION_METHOD;
	static const StringWithHash CLEARCONFIGURATION_METHOD;
	static const StringWithHash RELEASE_METHOD;

	static const MethodDescriptor interfaceMethods_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(MediaEndpoint);
};

} /* namespace dbus */

#endif /* MEDIAENDPOINT_H_ */
