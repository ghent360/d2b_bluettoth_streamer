/*
 * SbcMediaEndpoint.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "SbcMediaEndpoint.h"

#include <glog/logging.h>
#include <stdlib.h>

#define MAX(a,b) ((a) < (b)) ? (b) : (a)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

namespace dbus {

SbcMediaEndpoint::SbcMediaEndpoint()
    : MediaEndpoint(ObjectBase::makeObjectPath("/MediaEndpoint/A2DPSBC", this)) {
}

const a2dp_sbc_t SbcMediaEndpoint::CAPABILITIES = {
	// channel_mode
	SBC_CHANNEL_MODE_MONO | SBC_CHANNEL_MODE_DUAL_CHANNEL |
	SBC_CHANNEL_MODE_STEREO | SBC_CHANNEL_MODE_JOINT_STEREO,
	// frequency
	/*SBC_SAMPLING_FREQ_16000 | SBC_SAMPLING_FREQ_32000 |*/
	SBC_SAMPLING_FREQ_44100 | SBC_SAMPLING_FREQ_48000,
	// allocation_method
	SBC_ALLOCATION_SNR | SBC_ALLOCATION_LOUDNESS,
	// subbands
	SBC_SUBBANDS_4 | SBC_SUBBANDS_8,
	// block_length
	SBC_BLOCK_LENGTH_4 | SBC_BLOCK_LENGTH_8 |
	SBC_BLOCK_LENGTH_12 | SBC_BLOCK_LENGTH_16,
	// min_bitpool
	SBC_MIN_BITPOOL,
	// max_bitpool
	SBC_MAX_BITPOOL
};

bool SbcMediaEndpoint::getCapabilities(uint8_t* capabilities,
		size_t* capabilities_max_len) const {
	if (*capabilities_max_len < sizeof(CAPABILITIES)) {
		*capabilities_max_len = sizeof(CAPABILITIES);
		return false;
	}
	*capabilities_max_len = sizeof(CAPABILITIES);
	memcpy(capabilities, &CAPABILITIES, sizeof(CAPABILITIES));
	return true;
}

// Refer to table 4.7 from the A2DP bluetooth spec V1.3
static uint8_t sbc_default_bitpool(uint8_t freq, uint8_t mode) {
    switch (freq) {
        case SBC_SAMPLING_FREQ_16000:
        case SBC_SAMPLING_FREQ_32000:
            return 53;

        case SBC_SAMPLING_FREQ_44100:

            switch (mode) {
                case SBC_CHANNEL_MODE_MONO:
                case SBC_CHANNEL_MODE_DUAL_CHANNEL:
                	return 31;

                case SBC_CHANNEL_MODE_STEREO:
                case SBC_CHANNEL_MODE_JOINT_STEREO:
                    return 53;

                default:
                    LOG(ERROR) << "Invalid channel mode " << mode;
                    return 53;
            }
            break;

        case SBC_SAMPLING_FREQ_48000:

            switch (mode) {
                case SBC_CHANNEL_MODE_MONO:
                case SBC_CHANNEL_MODE_DUAL_CHANNEL:
                	return 29;

                case SBC_CHANNEL_MODE_STEREO:
                case SBC_CHANNEL_MODE_JOINT_STEREO:
                    return 51;

                default:
                    LOG(ERROR) << "Invalid channel mode " << mode;
                    return 51;
            }
            break;

        default:
            LOG(ERROR) << "Invalid sampling freq " << freq;
            return 53;
    }
}

bool SbcMediaEndpoint::selectConfiguration(void* capabilities,
		size_t capabilities_len,
		uint8_t** selected_capabilities,
		size_t* selected_capabilities_len) {
	LOG(INFO) << "MediaEndpoint::selectConfiguration";
	a2dp_sbc_t *input_config;
	a2dp_sbc_t *selected_config;

	*selected_capabilities_len = sizeof(a2dp_sbc_t);
	selected_config = reinterpret_cast<a2dp_sbc_t*>(new uint8_t[sizeof(a2dp_sbc_t)]);
    //taken from pulseaudio with modification
    memset(selected_config, 0, sizeof(a2dp_sbc_t));
    input_config = reinterpret_cast<a2dp_sbc_t *>(capabilities);
    if (input_config->frequency & SBC_SAMPLING_FREQ_44100) {
        selected_config->frequency = SBC_SAMPLING_FREQ_44100;
        LOG(INFO) << "Selected 44.1kHZ sampling rate";
    } else if (input_config->frequency & SBC_SAMPLING_FREQ_48000) {
        selected_config->frequency = SBC_SAMPLING_FREQ_48000;
        LOG(INFO) << "Selected 48kHZ sampling rate";
    } else {
        LOG(ERROR) << "No supported sampling frequency";
        return false;
    }

    if (input_config->channel_mode & SBC_CHANNEL_MODE_JOINT_STEREO) {
        selected_config->channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
        LOG(INFO) << "Selected Joint Stereo.";
    } else if (input_config->channel_mode & SBC_CHANNEL_MODE_STEREO) {
        selected_config->channel_mode = SBC_CHANNEL_MODE_STEREO;
        LOG(INFO) << "Selected Stereo.";
    } else if (input_config->channel_mode & SBC_CHANNEL_MODE_DUAL_CHANNEL) {
        selected_config->channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
        LOG(INFO) << "Selected dual channel.";
    } else if (input_config->channel_mode & SBC_CHANNEL_MODE_MONO) {
        selected_config->channel_mode = SBC_CHANNEL_MODE_MONO;
        LOG(INFO) << "Selected mono.";
    } else {
        LOG(ERROR) << "No supported channel modes";
        return false;
    }

    if (input_config->block_length & SBC_BLOCK_LENGTH_16)
        selected_config->block_length = SBC_BLOCK_LENGTH_16;
    else if (input_config->block_length & SBC_BLOCK_LENGTH_12)
        selected_config->block_length = SBC_BLOCK_LENGTH_12;
    else if (input_config->block_length & SBC_BLOCK_LENGTH_8)
        selected_config->block_length = SBC_BLOCK_LENGTH_8;
    else if (input_config->block_length & SBC_BLOCK_LENGTH_4)
        selected_config->block_length = SBC_BLOCK_LENGTH_4;
    else {
    	LOG(ERROR) << "No supported block lengths";
        return false;
    }

    if (input_config->subbands & SBC_SUBBANDS_8)
        selected_config->subbands = SBC_SUBBANDS_8;
    else if (input_config->subbands & SBC_SUBBANDS_4)
        selected_config->subbands = SBC_SUBBANDS_4;
    else {
    	LOG(ERROR) << "No supported sub-bands";
        return false;
    }

    if (input_config->allocation_method & SBC_ALLOCATION_LOUDNESS)
        selected_config->allocation_method = SBC_ALLOCATION_LOUDNESS;
    else if (input_config->allocation_method & SBC_ALLOCATION_SNR)
        selected_config->allocation_method = SBC_ALLOCATION_SNR;

    selected_config->min_bitpool = (uint8_t) MAX(SBC_MIN_BITPOOL, input_config->min_bitpool);
    selected_config->max_bitpool = (uint8_t) MIN(sbc_default_bitpool(selected_config->frequency,
    		selected_config->channel_mode), input_config->max_bitpool);
    *selected_capabilities = reinterpret_cast<uint8_t*>(selected_config);
	return true;
}

void SbcMediaEndpoint::setConfiguration(const ObjectPath& transport,
		const MediaTransportProperties& properties) {
	MediaEndpoint::setConfiguration(transport, properties);
	if (properties.getConfigurationLen() != sizeof(a2dp_sbc_t)) {
		LOG(ERROR) << "Invalid SBC configuration set. Expected "
				<< sizeof(a2dp_sbc_t) << " bytes got "
				<< properties.getConfigurationLen();
		return;
	}
	a2dp_sbc_t *configuration = reinterpret_cast<a2dp_sbc_t *>(
			properties.getConfiguration());
    if (configuration->frequency & SBC_SAMPLING_FREQ_44100) {
        LOG(INFO) << "Set 44.1kHZ sampling rate";
        sampling_rate_ = 44100;
    } else if (configuration->frequency & SBC_SAMPLING_FREQ_48000) {
        LOG(INFO) << "Set 48kHZ sampling rate";
        sampling_rate_ = 48000;
    } else {
        LOG(ERROR) << "Set invalid sampling frequency";
        return;
    }
}

} /* namespace dbus */
