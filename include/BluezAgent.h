/*
 * BluezAgent.h
 *
 *  Created on: Nov 29, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef BLUEZAGENT_H_
#define BLUEZAGENT_H_

#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <googleapis/base/callback.h>

namespace dbus {

class Connection;

class BluezAgent : public SimpleObjectBase {
public:
	BluezAgent(Connection* conn, const ObjectPath& path)
        : SimpleObjectBase(path),
		  connection_(conn) {
		interface_ = &implementation_;
	}
	virtual ~BluezAgent() {}

private:
	//static Message handle_DeviceRemoved(Message& msg, ObjectBase* ctx,
	//		const InterfaceImplementation* interface);

	Connection* connection_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash RELEASE_METHOD;
	static const StringWithHash REQUESTPINCODE_METHOD;
	static const StringWithHash REQUESTPASSKEY_METHOD;
	static const StringWithHash DISPLAYPASSKEY_METHOD;
	static const StringWithHash DISPLAYPINCODE_METHOD;
	static const StringWithHash REQUESTCONFIRMATION_METHOD;
	static const StringWithHash AUTHORIZE_METHOD;
	static const StringWithHash CONFIRMMODECHANGE_METHOD;
	static const StringWithHash CANCEL_METHOD;

	static const MethodDescriptor interfaceMethods_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(BluezAgent);
};

} /* namespace dbus */

#endif /* BLUEZAGENT_H_ */
