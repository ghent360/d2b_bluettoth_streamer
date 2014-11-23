/*
 * ObjectBase.h
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef OBJECTBASE_H_
#define OBJECTBASE_H_

#include "InterfaceImplementation.h"
#include "MediaTransportProperties.h"
#include "Message.h"
#include "ObjectPath.h"
#include "util.h"

namespace dbus {

class ObjectBase {
public:
	ObjectBase(const ObjectPath& path) : self_(path) {}
	virtual ~ObjectBase() {}

	bool matchesObject(const ObjectPath& path) {
		return path == self_;
	}

	const ObjectPath& getPathToSelf() const {
		return self_;
	}

	Message handleMessage(Message& msg);

	virtual void registerSignals(Connection*) const = 0;
	virtual void unregisterSignals(Connection*) const = 0;

protected:
	static ObjectPath makeObjectPath(const char* prefix, const void* object);
	static ObjectPath makeObjectPath(const void* object) {
		return makeObjectPath("obj", object);
	}

	virtual const InterfaceImplementation* matchInterface(
			const StringWithHash& interface) const = 0;

private:
	const ObjectPath self_;
};

// A DBus object that implements a single interface. The object constructor
// must initialize the interface_ pointer, to point to the DBus metadata.
class SimpleObjectBase : public ObjectBase {
public:
	SimpleObjectBase(const ObjectPath& path)
        : ObjectBase(path),
		  interface_(NULL) {}

protected:
	virtual const InterfaceImplementation* matchInterface(
			const StringWithHash& interface) const;
	virtual void registerSignals(Connection* conn) const;
	virtual void unregisterSignals(Connection* conn) const;

	static Message default_PropertyChange_handler(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);

	static Message propertyChange_noError_handler(Message& msg, ObjectBase* ctx,
			const InterfaceImplementation* interface);

	const InterfaceImplementation* interface_;
};

} /* namespace dbus */

#endif /* OBJECTBASE_H_ */
