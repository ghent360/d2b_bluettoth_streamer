/*
 * MethodLocator.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef METHODLOCATOR_H_
#define METHODLOCATOR_H_

#include <string>

namespace dbus {

class Message;

class MethodLocator {
public:
	enum Type {
		E_METHOD,
		E_SIGNAL
	};

	MethodLocator(Type type, const char* interface, const char* method);
	virtual ~MethodLocator() {}

	Type getType() const {
		return type_;
	}

	const char* getInterface() const {
		return interface_.c_str();
	}

	const char* getMethod() const {
		return method_.c_str();
	}

	bool matches(Message&);
	virtual Message handle(Message&) = 0;

	bool operator == (const MethodLocator &);
private:
	Type type_;
	std::string interface_;
	std::string method_;
};

} /* namespace dbus */

#endif /* METHODLOCATOR_H_ */
