/*
 * MediaTransport.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MediaTransport.h"

#include "BluezNames.h"
#include "Connection.h"
#include "MediaTransportProperties.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include <glog/logging.h>
#include <RemoteMethod.h>

namespace dbus {

const char* MediaTransport::INTERFACE = "org.bluez.MediaTransport";
const char* MediaTransport::GETPROPERTIES_METHOD = "GetProperties";
const char* MediaTransport::ACQUIRE_METHOD = "Acquire";
const char* MediaTransport::RELEASE_METHOD = "Release";
const char* MediaTransport::SETPROPERTY_METHOD = "SetProperty";

bool MediaTransport::getProperties(MediaTransportProperties* property_bag) {
	RemoteMethod rpc(ORG_BLUEZ, path_.path(), INTERFACE, GETPROPERTIES_METHOD);
	rpc.prepareCall();
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg() != NULL) {
		MessageArgumentIterator reply_args = reply.argIterator();
		if (reply_args.hasArgs() &&
			reply_args.getArgumentType() == DBUS_TYPE_DICT_ENTRY) {
			BaseMessageIterator iter = reply_args.recurse();
			property_bag->parseDictionary(&iter);
			return true;
		}
	}
	return false;
}

bool MediaTransport::acquire(const char* access_type,
    		int* fd,
    		int* read_mtu,
    		int* write_mtu) {
	RemoteMethod rpc(ORG_BLUEZ, path_.path(), INTERFACE, ACQUIRE_METHOD);
	rpc.prepareCall();
	MessageArgumentBuilder args = rpc.argBuilder();
	args.append(access_type);
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	if (reply.msg() != NULL) {
		MessageArgumentIterator reply_args = reply.argIterator();
		*fd = reply_args.getFileDescriptor();
		if (reply_args.next()) {
			*read_mtu = reply_args.getWord();
			if (reply_args.next()) {
				*write_mtu = reply_args.getWord();
				return true;
			}
		}
	}
	return false;
}

bool MediaTransport::release(const char* access_type) {
	RemoteMethod rpc(ORG_BLUEZ, path_.path(), INTERFACE, RELEASE_METHOD);
	rpc.prepareCall();
	MessageArgumentBuilder args = rpc.argBuilder();
	args.append(access_type);
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return reply.msg() != NULL;
}

bool MediaTransport::setProperty(const char* property_name,
    		const MediaTransportProperties& property_bag) {
    LOG(ERROR) << "MediaTransport::setProperty is not implemented yet";
    return false;
}

} /* namespace dbus */
