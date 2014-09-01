/*
 * AudioSourcePropertyChanged.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "AudioSourcePropertyChanged.h"

#include "Message.h"
#include "MessageArgumentIterator.h"

#include <glog/logging.h>

namespace dbus {

AudioSourcePropertyChanged::AudioSourcePropertyChanged()
    : MethodLocator(Type::E_SIGNAL,
    		"org.bluez.AudioSource",
    		"PropertyChanged") {
}

Message AudioSourcePropertyChanged::handle(Message& msg) {
	MessageArgumentIterator it = msg.argIterator();
	if (it.hasArgs()) {
		const char* property_name = it.getString();
		if (it.next()) {
			BaseMessageIterator itv = it.recurse();
            const char* value = itv.getString();
			LOG(INFO) << "AudioSourcePropertyChanged: " << property_name
					<< " " << value;
		}
	}
    return Message();
}

} /* namespace dbus */
