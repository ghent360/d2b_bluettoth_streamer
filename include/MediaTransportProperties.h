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
	~MediaTransportProperties();

	void parseDictionary(BaseMessageIterator* itprop);

private:
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

    DISALLOW_COPY_AND_ASSIGN(MediaTransportProperties);
};

} /* namespace dbus */

#endif /* MEDIATRANSPORTPROPERTIES_H_ */
