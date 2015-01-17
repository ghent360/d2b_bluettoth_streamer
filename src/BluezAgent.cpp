/*
 * BluezAgent.cpp
 *
 *  Created on: Nov 29, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "BluezAgent.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "RemoteMethod.h"

#include <dbus/dbus.h>
#include <glog/logging.h>

namespace dbus {

Message BluezAgent::handle_Release(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	pThis->release();
	return Message();
}

Message BluezAgent::handle_RequestPinCode(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	Message response;
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		const char* result = pThis->requestPinCode(device);
	    Message::forMethodReturn(msg, &response);
	    response.argBuilder().append(result);
	}
	return response;
}

Message BluezAgent::handle_RequestPasskey(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	Message response;
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		uint32_t result = pThis->requestPasskey(device);
	    Message::forMethodReturn(msg, &response);
	    response.argBuilder().append(result);
	}
	return response;
}

Message BluezAgent::handle_DisplayPasskey(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		args.next();
		if (args.getArgumentType() == DBUS_TYPE_UINT32) {
			uint32_t passkey = args.getUint32();
			args.next();
			if (args.getArgumentType() == DBUS_TYPE_BYTE) {
				uint8_t num_key_entered = args.getByte();
				pThis->displayPasskey(device, passkey, num_key_entered);
			}
		}
	}
	return Message();
}

Message BluezAgent::handle_DisplayPinCode(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		args.next();
		if (args.getArgumentType() == DBUS_TYPE_STRING) {
			const char* pin_code = args.getString();
			pThis->displayPinCode(device, pin_code);
		}
	}
	return Message();
}

Message BluezAgent::handle_RequestConfirmation(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	Message response;
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		args.next();
		if (args.getArgumentType() == DBUS_TYPE_UINT32) {
			uint32_t passkey = args.getUint32();
			Result result = pThis->requestConfirmation(device, passkey);
			if (result == REJECTED) {
				Message::forError(msg, REJECTED_ERROR, "Rejected", &response);
			} else if (result == CANCELED) {
				Message::forError(msg, CANCELED_ERROR, "Canceled", &response);
			}
		}
	}
	return response;
}

Message BluezAgent::handle_Authorize(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	Message response;
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_OBJECT_PATH) {
		ObjectPath device = args.getObjectPath();
		args.next();
		if (args.getArgumentType() == DBUS_TYPE_STRING) {
			const char* uuid = args.getString();
			Result result = pThis->authorize(device, uuid);
			if (result == REJECTED) {
				Message::forError(msg, REJECTED_ERROR, "Rejected", &response);
			} else if (result == CANCELED) {
				Message::forError(msg, CANCELED_ERROR, "Canceled", &response);
			}
		}
	}
	return response;
}

Message BluezAgent::handle_ConfirmModeChange(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	Message response;
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	auto args = msg.argIterator();
	if (args.getArgumentType() == DBUS_TYPE_STRING) {
		const char* mode = args.getString();
		Result result = pThis->confirmModeChange(mode);
		if (result == REJECTED) {
			Message::forError(msg, REJECTED_ERROR, "Rejected", &response);
		} else if (result == CANCELED) {
			Message::forError(msg, CANCELED_ERROR, "Canceled", &response);
		}
	}
	return response;
}

Message BluezAgent::handle_Cancel(Message& msg, ObjectBase* ctx,
		const InterfaceImplementation* interface) {
	BluezAgent* pThis = reinterpret_cast<BluezAgent*>(ctx);
	pThis->cancel();
	return Message();
}

// DBus metadata
const StringWithHash BluezAgent::INTERFACE = "org.bluez.Agent";

const StringWithHash BluezAgent::RELEASE_METHOD("Release");
const StringWithHash BluezAgent::REQUESTPINCODE_METHOD("RequestPinCode");
const StringWithHash BluezAgent::REQUESTPASSKEY_METHOD("RequestPasskey");
const StringWithHash BluezAgent::DISPLAYPASSKEY_METHOD("DisplayPasskey");
const StringWithHash BluezAgent::DISPLAYPINCODE_METHOD("DisplayPinCode");
const StringWithHash BluezAgent::REQUESTCONFIRMATION_METHOD("RequestConfirmation");
const StringWithHash BluezAgent::AUTHORIZE_METHOD("Authorize");
const StringWithHash BluezAgent::CONFIRMMODECHANGE_METHOD("ConfirmModeChange");
const StringWithHash BluezAgent::CANCEL_METHOD("Cancel");

const char* BluezAgent::REJECTED_ERROR = "org.bluez.Error.Rejected";
const char* BluezAgent::CANCELED_ERROR = "org.bluez.Error.Canceled";

const MethodDescriptor BluezAgent::interfaceMethods_[] = {
	MethodDescriptor(RELEASE_METHOD, handle_Release),
	MethodDescriptor(REQUESTPINCODE_METHOD, handle_RequestPinCode),
	MethodDescriptor(REQUESTPASSKEY_METHOD, handle_RequestPasskey),
	MethodDescriptor(DISPLAYPASSKEY_METHOD, handle_DisplayPasskey),
	MethodDescriptor(DISPLAYPINCODE_METHOD, handle_DisplayPinCode),
	MethodDescriptor(REQUESTCONFIRMATION_METHOD, handle_RequestConfirmation),
	MethodDescriptor(AUTHORIZE_METHOD, handle_Authorize),
	MethodDescriptor(CONFIRMMODECHANGE_METHOD, handle_ConfirmModeChange),
	MethodDescriptor(CANCEL_METHOD, handle_Cancel)
};

const InterfaceImplementation BluezAgent::implementation_(INTERFACE,
		interfaceMethods_);

SimpleBluezAgent::SimpleBluezAgent(Connection* conn, uint32_t pin_code)
    : BluezAgent(conn, ObjectBase::makeObjectPath("/BluezAgent", this)),
	  pin_code_(pin_code) {
}

const char* SimpleBluezAgent::requestPinCode(const ObjectPath&) {
    snprintf(buffer_, sizeof(buffer_), "%d", pin_code_);
    LOG(INFO) << "requestPinCode: " << buffer_;
    return buffer_;
}

void SimpleBluezAgent::displayPasskey(const ObjectPath& device,
		uint32_t passkey, uint8_t) {
    LOG(INFO) << "Passkey :" << passkey;
}

void SimpleBluezAgent::displayPinCode(const ObjectPath& device, const char* pin_code) {
    LOG(INFO) << "Pin Code :" << pin_code;
}

BluezAgent::Result SimpleBluezAgent::requestConfirmation(const ObjectPath&, uint32_t) {
	LOG(INFO) << "Request confirmation";
	return OK;
}

BluezAgent::Result SimpleBluezAgent::authorize(const ObjectPath& device, const char* uuid) {
	LOG(INFO) << "Authorize " << uuid;
	return OK;
}

BluezAgent::Result SimpleBluezAgent::confirmModeChange(const char* mode) {
	LOG(INFO) << "Confirm mode change: " << mode;
	return OK;
}

} /* namespace dbus */
