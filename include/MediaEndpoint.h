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

#include "MediaTransportProperties.h"
#include "ObjectPath.h"
#include "util.h"

#include <string>

namespace dbus {

class Connection;

class MediaEndpointInterface {
protected:
	virtual ~MediaEndpointInterface() {}

	virtual bool selectConfiguration(void* capabilities,
			int capabilities_len,
			uint8_t** selected_capabilities,
			int* selected_capabilities_len) = 0;

	virtual void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties) = 0;
	virtual void clearConfiguration(const ObjectPath& transport) = 0;
	virtual void release() = 0;

public:
	static void registerMethods(Connection&, MediaEndpointInterface*);
	static void unregisterMethods(Connection&);

protected:
	static const char* INTERFACE;

	static const char* SETCONFIGURATION_METHOD;
	static const char* SELECTCONFIGURATION_METHOD;
	static const char* CLEARCONFIGURATION_METHOD;
	static const char* RELEASE_METHOD;

	friend class MediaEndpointSelectConfiguration;
	friend class MediaEndpointSetConfiguration;
	friend class MediaEndpointClearConfiguration;
	friend class MediaEndpointRelease;
};

class MediaEndpoint : public MediaEndpointInterface {
public:
	MediaEndpoint();
	virtual ~MediaEndpoint() {}

	bool isTransportConfigValid() const {
		return transport_config_valid_;
	}

	const ObjectPath& getTransportPath() const {
		return transport_path_;
	}

protected:
	virtual bool selectConfiguration(void* capabilities,
			int capabilities_len,
			uint8_t** selected_capabilities,
			int* selected_capabilities_len);

	virtual void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties);
	virtual void clearConfiguration(const ObjectPath& transport);
	virtual void release();

private:
	bool transport_config_valid_;
	ObjectPath transport_path_;
	MediaTransportProperties transport_properties_;

	DISALLOW_COPY_AND_ASSIGN(MediaEndpoint);
};

} /* namespace dbus */

#endif /* MEDIAENDPOINT_H_ */
