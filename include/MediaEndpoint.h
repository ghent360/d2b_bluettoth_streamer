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

class MediaEndpoint : public SimpleObjectBase {
public:
	MediaEndpoint(Connection*);
	MediaEndpoint(const ObjectPath&);
	virtual ~MediaEndpoint() {}

	bool isTransportConfigValid() const {
		return transport_config_valid_;
	}

	const ObjectPath& getTransportPath() const {
		return transport_path_;
	}

private:
	bool selectConfiguration(void* capabilities,
			int capabilities_len,
			uint8_t** selected_capabilities,
			int* selected_capabilities_len);

	void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties);
	void clearConfiguration(const ObjectPath& transport);
	void release();

	static Message handle_selectConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_setConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_clearConfiguration(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);
	static Message handle_release(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation*);

	bool transport_config_valid_;
	ObjectPath transport_path_;
	MediaTransportProperties transport_properties_;

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
