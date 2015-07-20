/*
 * AacDecodeThread.cpp
 *
 *  Created on: July 19, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "AacDecodeThread.h"

#include <glog/logging.h>
#include <stdio.h>

namespace dbus {

static FILE* dump = NULL;
static int dump_no = 1;

AacDecodeThread::AacDecodeThread(Connection* connection,
		const ObjectPath& path,
		iqurius::AudioChannel* audio_channel)
    : PlaybackThread(connection, path, audio_channel) {
	char dump_name[128];
	snprintf(dump_name, sizeof(dump_name) - 1, "/tmp/aac_%d.bin", dump_no++);
	dump = fopen(dump_name, "wb");
}

AacDecodeThread::~AacDecodeThread() {
	if (dump) {
		fclose(dump);
	}
}

void AacDecodeThread::decode(const uint8_t* buffer, size_t size) {
	if (size < 12) {
		LOG(ERROR) << "Invalid RTP packet size " << size;
		return;
	}
	if ((buffer[0] & 0xC0) != 0x80) {
		LOG(WARNING) << "Invalid RTP version " << buffer[0];
		return;
	}
	bool has_padding = 0 != (buffer[0] & 0x20);
	bool has_ext = 0 != (buffer[0] & 0x10);
	uint8_t csrs_count = buffer[0] & 0x0f;
	bool has_marker = 0 != (buffer[1] & 0x80);
	uint8_t payload_type = buffer[1] & 0x7f;
	uint16_t seq_no = (buffer[2] << 8) | buffer[3];
	uint32_t timestamp = (buffer[4] << 24) |
			(buffer[5] << 16) |
			(buffer[6] << 8) |
			buffer[7];
	uint8_t padding_size = 0;

	if (has_padding) {
		padding_size = buffer[size - 1];
		if (size >= 12 + padding_size) {
			size -= padding_size;
		}
	}
	buffer += 12;
	size -= 12;
	if (size < csrs_count * 4) {
		LOG(ERROR) << "Invalid RTP packet size " << size << " missing " <<
				csrs_count << " CSRCs";
		return;
	}
	size -= csrs_count * 4;
	if (has_ext) {
		LOG(WARNING) << "Extensions are not supported";
	}
	if (dump) {
		uint32_t hdr = 0x2b7 << 13;
		hdr |= size & 0x1FFF;
		uint8_t hdr_buf[3];
		hdr_buf[0] = (hdr >> 16);
		hdr_buf[1] = (hdr >> 8);
		hdr_buf[2] = hdr;
		fwrite(hdr_buf, 3, 1, dump);
		fwrite(buffer, size, 1, dump);
	}
}

} /* namespace dbus */
