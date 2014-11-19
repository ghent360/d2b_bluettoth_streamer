/*
 * MediaTransport.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MEDIATRANSPORT_H_
#define MEDIATRANSPORT_H_

#include "ObjectPath.h"
#include "util.h"

namespace dbus {

class Connection;
class MediaTransportProperties;

class MediaTransport {
public:
	MediaTransport(Connection* connection, const ObjectPath& path)
	    : connection_(connection),
	      path_(path) {
	}

	bool getProperties(MediaTransportProperties* property_bag);
    bool acquire(const char* access_type,
    		int* fd,
    		int* read_mtu,
    		int* write_mtu);
    bool release(const char* accesstype);
    bool setProperty(const char* property_name,
    		const MediaTransportProperties& property_bag);
private:
	static const char* INTERFACE;
	static const char* GETPROPERTIES_METHOD;
	static const char* ACQUIRE_METHOD;
	static const char* RELEASE_METHOD;
	static const char* SETPROPERTY_METHOD;

	Connection* connection_;
	ObjectPath path_;
	DISALLOW_COPY_AND_ASSIGN(MediaTransport);
};

} /* namespace dbus */

#endif /* MEDIATRANSPORT_H_ */
