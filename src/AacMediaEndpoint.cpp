/*
 * AacMediaEndpoint.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "AacMediaEndpoint.h"

#include <glog/logging.h>
#include <stdlib.h>

#define MAX(a,b) ((a) < (b)) ? (b) : (a)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

namespace dbus {

AacMediaEndpoint::AacMediaEndpoint()
    : MediaEndpoint(ObjectBase::makeObjectPath("/MediaEndpoint/A2DPAAC", this)) {
}

const a2dp_aac_t AacMediaEndpoint::CAPABILITIES = {
	// object_type
	AAC_MPEG2_LC | AAC_MPEG4_LC | AAC_MPEG4_LTP | AAC_MPEG4_SCALABLE,
	// sampling_freq1
	AAC_SAMPLING_FREQ_8000 | AAC_SAMPLING_FREQ_11025 | AAC_SAMPLING_FREQ_12000 |
	AAC_SAMPLING_FREQ_16000 | AAC_SAMPLING_FREQ_22050 | AAC_SAMPLING_FREQ_24000 |
	AAC_SAMPLING_FREQ_32000 | AAC_SAMPLING_FREQ_44100,
	// rfa:2
	0,
	// channels:2
	AAC_CHANNEL_MODE_MONO | AAC_CHANNEL_MODE_STEREO,
	// sampling_freq2:4
	AAC_SAMPLING_FREQ_48000 | AAC_SAMPLING_FREQ_64000 | AAC_SAMPLING_FREQ_88200 |
	AAC_SAMPLING_FREQ_96000,
	// bitrate_hi:7
	// Apple says should be 264630 bps or 0x409B6
	0x04,
	// vbr:1
	1,

	// bitrate_mid
	0x09,
	// bitrate_lo
	0xb6
};

bool AacMediaEndpoint::getCapabilities(uint8_t* capabilities,
		size_t* capabilities_max_len) const {
	if (*capabilities_max_len < sizeof(CAPABILITIES)) {
		*capabilities_max_len = sizeof(CAPABILITIES);
		return false;
	}
	*capabilities_max_len = sizeof(CAPABILITIES);
	memcpy(capabilities, &CAPABILITIES, sizeof(CAPABILITIES));
	return true;
}

bool AacMediaEndpoint::selectConfiguration(void* capabilities,
		size_t capabilities_len,
		uint8_t** selected_capabilities,
		size_t* selected_capabilities_len) {
	LOG(INFO) << "MediaEndpoint::selectConfiguration";
	a2dp_aac_t *input_config;
	a2dp_aac_t *selected_config;

	*selected_capabilities_len = sizeof(a2dp_aac_t);
	selected_config = reinterpret_cast<a2dp_aac_t*>(new uint8_t[sizeof(a2dp_aac_t)]);

    memset(selected_config, 0, sizeof(a2dp_aac_t));
    input_config = reinterpret_cast<a2dp_aac_t *>(capabilities);

    if (input_config->object_type & AAC_MPEG2_LC) {
    	selected_config->object_type = AAC_MPEG2_LC;
    } else {
    	LOG(ERROR) << "No supported codec mode";
    	return false;
    }

    selected_config->channels = AAC_CHANNEL_MODE_STEREO;
    selected_config->sampling_freq1 = AAC_SAMPLING_FREQ_44100;
    selected_config->vbr = input_config->vbr;
    selected_config->bitrate_hi = input_config->bitrate_hi;
    selected_config->bitrate_mid = input_config->bitrate_mid;
    selected_config->bitrate_lo = input_config->bitrate_lo;
    *selected_capabilities = reinterpret_cast<uint8_t*>(selected_config);
	return true;
}

int AacMediaEndpoint::getSamplingRate() const {
	return 44100;
}
} /* namespace dbus */
