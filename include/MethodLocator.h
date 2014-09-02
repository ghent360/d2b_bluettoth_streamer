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
		return interface_;
	}

	const char* getMethod() const {
		return method_;
	}

	virtual bool matches(Message&);
	virtual Message handle(Message&, void* ctx) = 0;

	bool operator == (const MethodLocator &);
private:
	Type type_;
	const char* interface_;
	const char* method_;
};

} /* namespace dbus */

#endif /* METHODLOCATOR_H_ */
