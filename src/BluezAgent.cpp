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

const MethodDescriptor BluezAgent::interfaceMethods_[] = {
};

const InterfaceImplementation BluezAgent::implementation_(INTERFACE,
		interfaceMethods_);

} /* namespace dbus */
