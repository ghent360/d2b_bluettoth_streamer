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

AacDecodeThread::AacDecodeThread(Connection* connection,
		const ObjectPath& path,
		iqurius::AudioChannel* audio_channel)
    : PlaybackThread(connection, path, audio_channel) {
	decoder_ = aacDecoder_Open(TT_MP4_LATM_MCP1, 1);
}

AacDecodeThread::~AacDecodeThread() {
	aacDecoder_Close(decoder_);
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
	unsigned int valid[1];
	unsigned int sizes[1];
	unsigned char* buffers[1];
	valid[0] = size;
	sizes[0] = size;
	buffers[0] = const_cast<unsigned char*>(buffer);
	int err = aacDecoder_Fill(decoder_, buffers, sizes, valid);
    if (err != 0) {
		LOG(WARNING) << "Decoder Fill err = " << err;
	} else {
		err = aacDecoder_DecodeFrame(decoder_, pcm_buffer_, sizeof(pcm_buffer_), 0);
		if (err != 0) {
			LOG(WARNING) << "Decode Frame err = %d\n" << err;
		} else {
			CStreamInfo* info = aacDecoder_GetStreamInfo(decoder_);
			const uint8_t* pcm_data = reinterpret_cast<const uint8_t*>(pcm_buffer_);
			playPcm(pcm_data, info->numChannels * info->frameSize * sizeof(INT_PCM));
		}
	}
}

} /* namespace dbus */
