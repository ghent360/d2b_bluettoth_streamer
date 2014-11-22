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

class Connection;
class ObjectBase {
public:
	ObjectBase(Connection*);
	ObjectBase(const ObjectPath& path) : self_(path) {}
	virtual ~ObjectBase() {}

	bool matchesObject(const ObjectPath& path) {
		return path == self_;
	}

	const ObjectPath& getPathToSelf() const {
		return self_;
	}

	Message handleMessage(Message& msg);

protected:
	virtual const InterfaceImplementation* matchInterface(
			const StringWithHash& interface) const = 0;

private:
	const ObjectPath self_;
};

} /* namespace dbus */

#endif /* OBJECTBASE_H_ */
