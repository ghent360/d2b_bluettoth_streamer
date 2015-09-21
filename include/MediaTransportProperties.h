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

	void parseDictionary(BaseMessageIterator* it_prop);

	MediaTransportProperties& operator = (
			const MediaTransportProperties& other) {
		assign(other);
		return *this;
	}

	void dump() const;

	int getCodecId() const {
		return codec_id_;
	}

	uint8_t* getConfiguration() const {
		return configuration_;
	}

	size_t getConfigurationLen() const {
		return configuration_len_;
	}

	unsigned short getDelay() const {
		return delay_;
	}

	bool isInboundRingtones() const {
		return inbound_ringtones_;
	}

	bool isNrec() const {
		return nrec_;
	}

	const std::string& getRouting() const {
		return routing_;
	}

	const std::string& getUuid() const {
		return uuid_;
	}

	unsigned short getVolume() const {
		return volume_;
	}

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
