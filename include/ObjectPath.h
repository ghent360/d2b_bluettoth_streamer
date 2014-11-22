/*
 * ObjectPath.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef OBJECTPATH_H_
#define OBJECTPATH_H_

#include "StringWithHash.h"

namespace dbus {

class ObjectPath : public StringWithHash {
public:
	ObjectPath() {};
	ObjectPath(const char* path) : StringWithHash(path) {}
	ObjectPath(const std::string& path) : StringWithHash(path) {}
	virtual ~ObjectPath() {};

	const char* path() const {
		return StringWithHash::str();
	}

	bool isValid() const {
		return length() > 0;
	}
};

} /* namespace dbus */

#endif /* OBJECTPATH_H_ */
