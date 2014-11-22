/*
 * RemoteMethod.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef METHOD_BASE_H_
#define METHOD_BASE_H_

#include "ObjectPath.h"
#include "Message.h"
#include "MessageArgumentIterator.h"

#include <string>

namespace dbus {

class RemoteMethod {
public:
	RemoteMethod(const char* destination,
			const char* path,
			const char* interface,
			const char* method_name)
        : destination_(destination),
          path_(path),
          interface_(interface),
          method_name_(method_name) {
	};

	virtual ~RemoteMethod();

	const char* destination() const {
		return destination_;
	}

	const char* path() const {
		return path_;
	}

	const char* interface() const {
		return interface_;
	}

	const char* methodName() const {
		return method_name_;
	}

	bool prepareCall();

	MessageArgumentBuilder argBuilder() {
		return call_.argBuilder();
	}

    Message msg() const {
    	return call_;
    }
private:
    const char* destination_;
    const char* path_;
    const char* interface_;
    const char* method_name_;

    Message call_;
};

} /* namespace dbus */

#endif /* METHOD_BASE_H_ */
