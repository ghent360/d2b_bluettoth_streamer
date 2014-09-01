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

class MediaEndpoint {
public:
	struct TransportProperties {
		TransportProperties() :
			codec_id(0),
			configuration(NULL),
			configuration_len(0),
			delay(0),
			nrec(false),
			inbound_ringtones(false),
			volume(0) {
		}
		~TransportProperties() {
			if (configuration) {
				delete [] configuration;
			}
		}

        ObjectPath device;
        std::string uuid;
        int codec_id;
        char* configuration;
        size_t configuration_len;
        unsigned short delay;
        bool nrec;
        bool inbound_ringtones;
        std::string routing;
        unsigned short volume;
	};

	void selectConfiguration(void* capabilities,
			size_t capabilities_len,
			void** selected_capabilities,
			size_t* selected_capabilities_len);

	void setConfiguration(ObjectPath transport, TransportProperties& properties);
	void clearConfiguration(ObjectPath transport);
	void release();

	void registerMethods(Connection&);
	void unregisterMethods(Connection&);
};

} /* namespace dbus */

#endif /* MEDIAENDPOINT_H_ */
