/*
 * BluezMedia.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef BLUEZMEDIA_H_
#define BLUEZMEDIA_H_

#include "ObjectPath.h"
#include "util.h"

namespace dbus {

class Connection;

class BluezMedia {
public:
	BluezMedia(Connection* connection, const ObjectPath& path)
	    : connection_(connection),
	      path_(path) {
	}

	bool registerEndpoint(const ObjectPath& endpoint_path,
			const char* uuid,
			int codec_id,
			const void* capabilities,
			size_t capabilities_len);

	bool unregisterEndpoint(const ObjectPath& endpoint_path);
private:
	static const char* INTERFACE;
	static const char* REGISTER_ENDPONT_METHOD;
	static const char* UNREGISTER_ENDPONT_METHOD;
	static const char* REGISTER_PLAYER_METHOD;
	static const char* UNREGISTER_PLAYER_METHOD;

	Connection* connection_;
	ObjectPath path_;
	DISALLOW_COPY_AND_ASSIGN(BluezMedia);
};

} /* namespace dbus */

#endif /* BLUEZMEDIA_H_ */
