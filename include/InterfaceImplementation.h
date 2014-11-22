/*
 * InterfaceImplementation.h
 *
 *  Created on: Nov 21, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef INTERFACEIMPLEMENTATION_H_
#define INTERFACEIMPLEMENTATION_H_

#include "Message.h"
#include "StringWithHash.h"
#include "util.h"

#include <list>
#include <string>

namespace dbus {

class ObjectBase;

typedef Message (*MethodHandler)(Message& msg, ObjectBase* ctx);
struct MethodDescriptor {
	MethodDescriptor(const StringWithHash& methodName, MethodHandler handler)
		: methodName_(methodName),
		  handler_(handler) {}
	const StringWithHash methodName_;
	MethodHandler handler_;
};

class InterfaceImplementation {
public:
	template<class Cm, class Cs>
	InterfaceImplementation(const StringWithHash& interfaceName,
			const Cm& methods, const Cs& signals)
	    : interfaceName_(interfaceName) {
		for (const MethodDescriptor& d : methods) {
			addMethod(d);
		}
		for (const MethodDescriptor& d : signals) {
			addSignal(d);
		}
	}

	bool matchesInterface(const StringWithHash& interface) const {
		return interface == interfaceName_;
	}

	Message handleMessage(Message& msg, ObjectBase* ctx) const;

	void addMethod(StringWithHash methodName, MethodHandler handler) {
		addMethod(MethodDescriptor(methodName, handler));
	}

	void addMethod(const MethodDescriptor& method) {
		methods_.push_back(method);
	}

	void addSignal(StringWithHash methodName, MethodHandler handler) {
		addSignal(MethodDescriptor(methodName, handler));
	}

	void addSignal(const MethodDescriptor& method) {
		signals_.push_back(method);
	}
private:
	const MethodDescriptor* findMethod(const StringWithHash& name,
			const std::list<MethodDescriptor>& list) const;

	const StringWithHash interfaceName_;
	std::list<MethodDescriptor> methods_;
	std::list<MethodDescriptor> signals_;

	DISALLOW_COPY_AND_ASSIGN(InterfaceImplementation);
};

} /* namespace dbus */

#endif /* INTERFACEIMPLEMENTATION_H_ */
