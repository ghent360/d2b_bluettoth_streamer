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

	virtual const char* getCapabilities() const = 0;
protected:
	enum Result {
		OK = 0,
		REJECTED = 1,
		CANCELED = 2
	};

	virtual void release() = 0;
	virtual const char* requestPinCode(const ObjectPath&) = 0;
	virtual uint32_t requestPasskey(const ObjectPath&) = 0;
	virtual void displayPasskey(const ObjectPath&, uint32_t, uint8_t) = 0;
	virtual void displayPinCode(const ObjectPath&, const char*) = 0;
	virtual Result requestConfirmation(const ObjectPath&, uint32_t) = 0;
	virtual Result authorize(const ObjectPath&, const char*) = 0;
	virtual Result confirmModeChange(const char*) = 0;
	virtual void cancel() = 0;

private:
	static Message handle_Release(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_RequestPinCode(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_RequestPasskey(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_DisplayPasskey(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_DisplayPinCode(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_RequestConfirmation(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_Authorize(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_ConfirmModeChange(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);
	static Message handle_Cancel(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);

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

	static const char* REJECTED_ERROR;
	static const char* CANCELED_ERROR;

	static const MethodDescriptor interfaceMethods_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(BluezAgent);
};

class SimpleBluezAgent : public BluezAgent {
public:
	SimpleBluezAgent(Connection* conn, uint32_t pin_code);

	virtual const char* getCapabilities() const {
		return "KeyboardDisplay";
	}
protected:
	virtual void release() {}
	virtual const char* requestPinCode(const ObjectPath&);
	virtual uint32_t requestPasskey(const ObjectPath&) {
		return pin_code_;
	}
	virtual void displayPasskey(const ObjectPath&, uint32_t, uint8_t);
	virtual void displayPinCode(const ObjectPath&, const char*);
	virtual Result requestConfirmation(const ObjectPath&, uint32_t);
	virtual Result authorize(const ObjectPath&, const char*);
	virtual Result confirmModeChange(const char*);
	virtual void cancel() {}
private:
	uint32_t pin_code_;
	char buffer_[10];
	DISALLOW_COPY_AND_ASSIGN(SimpleBluezAgent);
};

} /* namespace dbus */

#endif /* BLUEZAGENT_H_ */
