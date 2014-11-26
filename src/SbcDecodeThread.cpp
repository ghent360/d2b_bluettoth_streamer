/*
 * SbcDecodeThread.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "SbcDecodeThread.h"

#include <glog/logging.h>

namespace dbus {

SbcDecodeThread::SbcDecodeThread(Connection* connection, const ObjectPath& path)
    : PlaybackThread(connection, path) {
	sbc_init(&codec_, 0);
}

SbcDecodeThread::~SbcDecodeThread() {
	sbc_finish(&codec_);
}

void SbcDecodeThread::decode(const uint8_t* buffer, size_t size) {
	if (size < 13) return;
	buffer += 13; // Skip the RTP header and the SBC header
	size -= 13;
	while (size > 0) {
		size_t written;
		ssize_t read;
		read = sbc_decode(&codec_, buffer, size,
				pcm_buffer_, sizeof(pcm_buffer_), &written);
		if (read > 0) {
			play_pcm(pcm_buffer_, written);
		} else {
			LOG(ERROR) << "Decode error, skipping packet.";
			break;
		}
		buffer += read;
		size -= read;
	}
}
} /* namespace dbus */
