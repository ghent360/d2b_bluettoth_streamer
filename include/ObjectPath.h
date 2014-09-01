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

#include <string>

namespace dbus {

class ObjectPath {
public:
	ObjectPath() {};
	ObjectPath(const char* path) : path_(path) {}
	virtual ~ObjectPath() {};

	const char* path() const {
		return path_.c_str();
	}

	bool isValid() const {
		return path_.length() > 0;
	}
private:
	std::string path_;
};

} /* namespace dbus */

#endif /* OBJECTPATH_H_ */
