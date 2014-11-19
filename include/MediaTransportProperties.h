/*
 * MediaTransportProperties.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MEDIATRANSPORTPROPERTIES_H_
#define MEDIATRANSPORTPROPERTIES_H_

#include "ObjectPath.h"
#include "util.h"

#include <string>

namespace dbus {

class BaseMessageIterator;

class MediaTransportProperties {
public:
	MediaTransportProperties();
	MediaTransportProperties(const MediaTransportProperties& other) {
		assign(other);
	}
	~MediaTransportProperties();

	void parseDictionary(BaseMessageIterator* itprop);

	MediaTransportProperties& operator = (
			const MediaTransportProperties& other) {
		assign(other);
		return *this;
	}

	void dump() const;
private:
	void assign(const MediaTransportProperties&);

    ObjectPath     device_;
    std::string    uuid_;
    int            codec_id_;
    uint8_t*       configuration_;
    size_t         configuration_len_;
    unsigned short delay_;
    bool           nrec_;
    bool           inbound_ringtones_;
    std::string    routing_;
    unsigned short volume_;
};

} /* namespace dbus */

#endif /* MEDIATRANSPORTPROPERTIES_H_ */
