/*
 * BluezMedia.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "BluezMedia.h"
#include "BluezNames.h"
#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include <glog/logging.h>
#include <RemoteMethod.h>

namespace dbus {
const char* BluezMedia::INTERFACE = "org.bluez.Media";
const char* BluezMedia::REGISTER_ENDPONT_METHOD = "RegisterEndpoint";
const char* BluezMedia::UNREGISTER_ENDPONT_METHOD = "UnregisterEndpoint";
const char* BluezMedia::REGISTER_PLAYER_METHOD = "RegisterPlayer";
const char* BluezMedia::UNREGISTER_PLAYER_METHOD = "UnregisterPlayer";

bool BluezMedia::registerEndpoint(const ObjectPath& endpoint_path,
			const char* uuid,
			int codec_id,
			const void* capabilities,
			size_t capabilities_len) {
	RemoteMethod rpc(ORG_BLUEZ, path_.path(), INTERFACE, REGISTER_ENDPONT_METHOD);
	rpc.prepareCall();
	MessageArgumentBuilder iter = rpc.argBuilder();
	iter.append(endpoint_path);
	ContainerIterator args = iter.openContainer(DBUS_TYPE_ARRAY, "{sv}");
	args.appendDictEntry("UUID", uuid);
	args.appendDictEntry("Codec", (unsigned char) codec_id);
	args.appendDictEntry("Capabilities", capabilities, capabilities_len);
	args.close();

	LOG(INFO) << "registerEndpoint: " << endpoint_path.str();

	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return reply.msg() != NULL;
}

bool BluezMedia::unregisterEndpoint(const ObjectPath& endpoint_path) {
	RemoteMethod rpc(ORG_BLUEZ, path_.path(), INTERFACE, UNREGISTER_ENDPONT_METHOD);
	rpc.prepareCall();
	MessageArgumentBuilder iter = rpc.argBuilder();
	iter.append(endpoint_path);
	Message reply = connection_->sendWithReplyAndBlock(rpc, -1);
	return reply.msg() != NULL;
}
} /* namespace dbus */
