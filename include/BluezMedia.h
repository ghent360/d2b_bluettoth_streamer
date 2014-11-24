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
class MediaEndpoint;
class BluezMedia {
public:
	BluezMedia(Connection* connection, const ObjectPath& path)
	    : connection_(connection),
	      path_(path) {
	}

	bool registerEndpoint(const ObjectPath& endpoint_path,
			const char* uuid,
			int codec_id,
			const uint8_t* capabilities,
			size_t capabilities_len);

	bool registerEndpoint(const MediaEndpoint&);

	bool unregisterEndpoint(const ObjectPath& endpoint_path);
	bool unregisterEndpoint(const MediaEndpoint&);
private:
	static const char* INTERFACE;
	static const char* REGISTER_ENDPONT_METHOD;
	static const char* UNREGISTER_ENDPONT_METHOD;
	static const char* REGISTER_PLAYER_METHOD;
	static const char* UNREGISTER_PLAYER_METHOD;
	static const size_t MAX_CAPABILITIES_SIZE;

	Connection* connection_;
	ObjectPath path_;
	DISALLOW_COPY_AND_ASSIGN(BluezMedia);
};

} /* namespace dbus */

#endif /* BLUEZMEDIA_H_ */
