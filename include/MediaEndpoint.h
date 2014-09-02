/*
 * MediaEndpoint.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MEDIAENDPOINT_H_
#define MEDIAENDPOINT_H_

#include "ObjectPath.h"

#include <string>

namespace dbus {

class Connection;
class MediaTransportProperties;

class MediaEndpoint {
public:
	void selectConfiguration(void* capabilities,
			size_t capabilities_len,
			void** selected_capabilities,
			size_t* selected_capabilities_len);

	void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties);
	void clearConfiguration(const ObjectPath& transport);
	void release();

	void registerMethods(Connection&);
	void unregisterMethods(Connection&);
};

} /* namespace dbus */

#endif /* MEDIAENDPOINT_H_ */
