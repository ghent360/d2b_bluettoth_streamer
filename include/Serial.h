/*
 * Serial.h
 *
 *  Created on: Jun 27, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "InterfaceImplementation.h"
#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <googleapis/base/callback.h>
#include <string>

namespace dbus {

class Connection;
class Serial : public SimpleObjectBase {
public:
	Serial(Connection* connection, const ObjectPath& path);
	virtual ~Serial();

	void connect(const char* prefix, std::string* result);
	void connectAsync(const char* prefix,
			int timeout,
			googleapis::Callback1<Message*>* cb);
	void disconnect(const char* device);
	void disconnectAsync(const char* device,
			int timeout,
			googleapis::Callback1<Message*>* cb);

private:
	Connection* connection_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash CONNECT_METHOD;
	static const StringWithHash DISCONNECT_METHOD;

	static const MethodDescriptor interfaceMethods_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(Serial);
};

} /* namespace dbus */

#endif /* SERIAL_H_ */
