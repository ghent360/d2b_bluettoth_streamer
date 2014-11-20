/*
 * MediaPlayerProperties.h
 *
 *  Created on: Nov 18, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MEDIAPLAYERPROPERTIES_H_
#define MEDIAPLAYERPROPERTIES_H_

#include "ObjectPath.h"
#include "util.h"

#include <string>

namespace dbus {

class BaseMessageIterator;

class MediaPlayerProperties {
public:
	MediaPlayerProperties();
	~MediaPlayerProperties();

	void parseDictionary(BaseMessageIterator* it_prop);

private:
    std::string Equalizer;  // "off" or "on"
    std::string Repeat;     // "off", "singletrack", "alltracks" or "group"
    std::string Shuffle;    // "off", "alltracks" or "group"
    std::string Scan;       // "off", "alltracks" or "group"
    std::string Status;     // "playing", "stopped", "paused", "forward-seek", "reverse-seek" or "error"
    uint32_t Position;      // milliseconds

    DISALLOW_COPY_AND_ASSIGN(MediaPlayerProperties);
};

} /* namespace dbus */

#endif /* MEDIAPLAYERPROPERTIES_H_ */
