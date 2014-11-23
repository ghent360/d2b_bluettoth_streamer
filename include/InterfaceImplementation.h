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

#include <glog/logging.h>
#include <list>
#include <string>

namespace dbus {

class ObjectBase;
class BaseMessageIterator;
class InterfaceImplementation;

typedef Message (*MethodHandler)(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface);
struct MethodDescriptor {
	MethodDescriptor(const StringWithHash& method_name, MethodHandler handler)
		: method_name_(method_name),
		  handler_(handler) {}
	const StringWithHash method_name_;
	MethodHandler handler_;
};

typedef void (*StringProperytHandler)(const char* value, ObjectBase* ctx);
typedef void (*Uint32ProperytHandler)(uint32_t value, ObjectBase* ctx);
typedef void (*BoolProperytHandler)(bool value, ObjectBase* ctx);
typedef void (*ArrayProperytHandler)(BaseMessageIterator& value_iterator, ObjectBase* ctx);
typedef void (*ObjectProperytHandler)(ObjectPath& value, ObjectBase* ctx);

union PropertyHandler {
public:
	PropertyHandler(StringProperytHandler handler)
        : string_handler_(handler) {}
	PropertyHandler(Uint32ProperytHandler handler)
        : uint32_handler_(handler) {}
	PropertyHandler(BoolProperytHandler handler)
        : bool_handler_(handler) {}
	PropertyHandler(ArrayProperytHandler handler)
        : array_handler_(handler) {}
	PropertyHandler(ObjectProperytHandler handler)
        : object_handler_(handler) {}

	void callHandler(BaseMessageIterator* vaue, ObjectBase* ctx) const;
private:
	StringProperytHandler string_handler_;
	Uint32ProperytHandler uint32_handler_;
	BoolProperytHandler bool_handler_;
	ArrayProperytHandler array_handler_;
	ObjectProperytHandler object_handler_;
};

struct PropertyDescriptor {
	PropertyDescriptor(const StringWithHash& property_name, StringProperytHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	PropertyDescriptor(const StringWithHash& property_name, Uint32ProperytHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	PropertyDescriptor(const StringWithHash& property_name, BoolProperytHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	PropertyDescriptor(const StringWithHash& property_name, ArrayProperytHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	PropertyDescriptor(const StringWithHash& property_name, ObjectProperytHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	PropertyDescriptor(const StringWithHash& property_name, PropertyHandler handler)
		: property_name_(property_name),
		  handler_(handler) {}
	const StringWithHash property_name_;
	PropertyHandler handler_;
};

class Connection;
class InterfaceImplementation {
public:
	template<class Cm, class Cs, class Cp>
	InterfaceImplementation(const StringWithHash& interface_name,
			const Cm& methods, const Cs& signals, const Cp& properties)
	    : interface_name_(interface_name) {
		for (const MethodDescriptor& d : methods) {
			addMethod(d);
		}
		for (const MethodDescriptor& d : signals) {
			addSignal(d);
		}
		for (const PropertyDescriptor& d : properties) {
            addProperty(d);
		}
	}

	template<class Cm, class Cs>
	InterfaceImplementation(const StringWithHash& interface_name,
			const Cm& methods, const Cs& signals)
		: interface_name_(interface_name) {
		for (const MethodDescriptor& d : methods) {
			addMethod(d);
		}
		for (const MethodDescriptor& d : signals) {
			addSignal(d);
		}
	}

	template<class Cm>
	InterfaceImplementation(const StringWithHash& interface_name,
			const Cm& methods) : interface_name_(interface_name) {
		for (const MethodDescriptor& d : methods) {
			addMethod(d);
		}
	}

	~InterfaceImplementation() {
	    LOG(INFO) << "destrction";
	}

	bool matchesInterface(const StringWithHash& interface) const {
		return interface == interface_name_;
	}

	Message handleMessage(Message& msg, ObjectBase* ctx) const;
	bool handlePropertyChanged(const StringWithHash& property_name,
			BaseMessageIterator* vaue, ObjectBase* ctx) const;

	void addMethod(const StringWithHash& method_name, MethodHandler handler) {
		addMethod(MethodDescriptor(method_name, handler));
	}

	void addMethod(const MethodDescriptor& method) {
		methods_.push_back(method);
	}

	void addSignal(const StringWithHash& method_name, MethodHandler handler) {
		addSignal(MethodDescriptor(method_name, handler));
	}

	void addSignal(const MethodDescriptor& method) {
		signals_.push_back(method);
	}

	void addProperty(const StringWithHash& property_name, PropertyHandler handler) {
		addProperty(PropertyDescriptor(property_name, handler));
	}

	void addProperty(const PropertyDescriptor& property) {
		properties_.push_back(property);
	}

	void registerSignals(Connection*, const ObjectBase*) const;
	void unregisterSignals(Connection*, const ObjectBase*) const;

	const char* getInterfaceName() const {
		return interface_name_.str();
	}

private:
	const MethodDescriptor* findMethod(const StringWithHash& name,
			const std::list<MethodDescriptor>& list) const;

	const StringWithHash interface_name_;
	std::list<MethodDescriptor> methods_;
	std::list<MethodDescriptor> signals_;
	std::list<PropertyDescriptor> properties_;

	DISALLOW_COPY_AND_ASSIGN(InterfaceImplementation);
};

} /* namespace dbus */

#endif /* INTERFACEIMPLEMENTATION_H_ */
