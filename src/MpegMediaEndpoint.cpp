/*
 * MpegMediaEndpoint.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MpegMediaEndpoint.h"

#include <glog/logging.h>
#include <stdlib.h>

#define MAX(a,b) ((a) < (b)) ? (b) : (a)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

namespace dbus {

MpegMediaEndpoint::MpegMediaEndpoint()
    : MediaEndpoint(ObjectBase::makeObjectPath("/MediaEndpoint/A2DPMPG", this)) {
}

const a2dp_mpeg_t MpegMediaEndpoint::CAPABILITIES = {
	// channel_mode
	MPEG_CHANNEL_MODE_MONO | MPEG_CHANNEL_MODE_DUAL_CHANNEL |
	MPEG_CHANNEL_MODE_STEREO | MPEG_CHANNEL_MODE_JOINT_STEREO,
	// crc
	MPEG_CRC_SUPPORT,
	// layer
	MPEG_LAYER_MP1 | MPEG_LAYER_MP2 | MPEG_LAYER_MP3,
	// frequency
	MPEG_SAMPLING_FREQ_16000 | MPEG_SAMPLING_FREQ_22050 | MPEG_SAMPLING_FREQ_24000 |
	MPEG_SAMPLING_FREQ_32000 | MPEG_SAMPLING_FREQ_44100 | MPEG_SAMPLING_FREQ_48000,
	// mpf2
	MPEG_MPF2_SUPPORT,
	// rfa
	0,
	// bitrate1
	MPEG_BITRATE_IDX8_SUPPORT | MPEG_BITRATE_IDX9_SUPPORT | MPEG_BITRATE_IDX10_SUPPORT |
	MPEG_BITRATE_IDX11_SUPPORT | MPEG_BITRATE_IDX12_SUPPORT | MPEG_BITRATE_IDX13_SUPPORT |
	MPEG_BITRATE_IDX14_SUPPORT,
	// vbr
	MPEG_VBR_SUPPORT,
	// bitrate
	MPEG_BITRATE_IDX0_SUPPORT | MPEG_BITRATE_IDX1_SUPPORT | MPEG_BITRATE_IDX2_SUPPORT |
	MPEG_BITRATE_IDX3_SUPPORT | MPEG_BITRATE_IDX4_SUPPORT | MPEG_BITRATE_IDX5_SUPPORT |
	MPEG_BITRATE_IDX6_SUPPORT | MPEG_BITRATE_IDX7_SUPPORT
};

bool MpegMediaEndpoint::getCapabilities(uint8_t* capabilities,
		size_t* capabilities_max_len) const {
	if (*capabilities_max_len < sizeof(CAPABILITIES)) {
		*capabilities_max_len = sizeof(CAPABILITIES);
		return false;
	}
	*capabilities_max_len = sizeof(CAPABILITIES);
	memcpy(capabilities, &CAPABILITIES, sizeof(CAPABILITIES));
	return true;
}

bool MpegMediaEndpoint::selectConfiguration(void* capabilities,
		size_t capabilities_len,
		uint8_t** selected_capabilities,
		size_t* selected_capabilities_len) {
	LOG(INFO) << "MediaEndpoint::selectConfiguration";
	a2dp_mpeg_t *input_config;
	a2dp_mpeg_t *selected_config;

	*selected_capabilities_len = sizeof(a2dp_mpeg_t);
	selected_config = reinterpret_cast<a2dp_mpeg_t*>(new uint8_t[sizeof(a2dp_mpeg_t)]);

    memset(selected_config, 0, sizeof(a2dp_mpeg_t));
    input_config = reinterpret_cast<a2dp_mpeg_t *>(capabilities);

    if (input_config->channel_mode & MPEG_CHANNEL_MODE_JOINT_STEREO)
        selected_config->channel_mode = MPEG_CHANNEL_MODE_JOINT_STEREO;
    else if (input_config->channel_mode & MPEG_CHANNEL_MODE_STEREO)
        selected_config->channel_mode = MPEG_CHANNEL_MODE_STEREO;
    else if (input_config->channel_mode & MPEG_CHANNEL_MODE_DUAL_CHANNEL)
        selected_config->channel_mode = MPEG_CHANNEL_MODE_DUAL_CHANNEL;
    else if (input_config->channel_mode & MPEG_CHANNEL_MODE_MONO) {
        selected_config->channel_mode = MPEG_CHANNEL_MODE_MONO;
    } else {
        LOG(ERROR) << "No supported channel modes";
        return false;
    }

    if (input_config->crc) {
    	selected_config->crc = MPEG_CRC_SUPPORT;
    }

    if (input_config->layer & MPEG_LAYER_MP3) {
    	selected_config->layer = MPEG_LAYER_MP3;
    } else if (input_config->layer & MPEG_LAYER_MP2) {
    	selected_config->layer = MPEG_LAYER_MP2;
    } if (input_config->layer & MPEG_LAYER_MP1) {
    	selected_config->layer = MPEG_LAYER_MP1;
    } else {
        LOG(ERROR) << "No supported layers selected";
        return false;
    }

    selected_config->frequency = MPEG_SAMPLING_FREQ_44100;

    if (input_config->vbr) {
    	selected_config->vbr = MPEG_CRC_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX0_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX0_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX14_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX14_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX13_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX13_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX12_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX12_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX11_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX11_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX10_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX10_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX9_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX9_SUPPORT;
    } else if (input_config->bitrate1 & MPEG_BITRATE_IDX8_SUPPORT) {
    	selected_config->bitrate1 = MPEG_BITRATE_IDX8_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX7_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX7_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX6_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX6_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX5_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX5_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX4_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX4_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX3_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX3_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX2_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX2_SUPPORT;
    } else if (input_config->bitrate2 & MPEG_BITRATE_IDX1_SUPPORT) {
    	selected_config->bitrate2 = MPEG_BITRATE_IDX1_SUPPORT;
    } else {
        LOG(ERROR) << "No supported bitrate selected";
        return false;
    }

    *selected_capabilities = reinterpret_cast<uint8_t*>(selected_config);
	return true;
}

} /* namespace dbus */
