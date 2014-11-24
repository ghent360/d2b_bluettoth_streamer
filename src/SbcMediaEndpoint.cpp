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

#include "ipc.h"

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
	BT_A2DP_CHANNEL_MODE_MONO | BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL |
	BT_A2DP_CHANNEL_MODE_STEREO | BT_A2DP_CHANNEL_MODE_JOINT_STEREO,
	// frequency
	BT_SBC_SAMPLING_FREQ_16000 | BT_SBC_SAMPLING_FREQ_32000 |
	BT_SBC_SAMPLING_FREQ_44100 | BT_SBC_SAMPLING_FREQ_48000,
	// allocation_method
	BT_A2DP_ALLOCATION_SNR | BT_A2DP_ALLOCATION_LOUDNESS,
	// subbands
	BT_A2DP_SUBBANDS_4 | BT_A2DP_SUBBANDS_8,
	// block_length
	BT_A2DP_BLOCK_LENGTH_4 | BT_A2DP_BLOCK_LENGTH_8 |
	BT_A2DP_BLOCK_LENGTH_12 | BT_A2DP_BLOCK_LENGTH_16,
	// min_bitpool
	MIN_BITPOOL,
	// max_bitpool
	MAX_BITPOOL
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

/*****************//**
 * Helper to calculate the optimum bitpool, given the sampling frequency,
 * and number of channels.
 * Taken verbatim from pulseaudio 2.1
 * (which took it from bluez audio - a2dp.c & pcm_bluetooth.c - default_bitpool)
 *
 * @param [in] frequency
 * @param [in] channel mode
 * @returns coded SBC bitpool
 *********************/
static uint8_t sbc_default_bitpool(uint8_t freq, uint8_t mode) {

    switch (freq) {
        case BT_SBC_SAMPLING_FREQ_16000:
        case BT_SBC_SAMPLING_FREQ_32000:
            return 53;

        case BT_SBC_SAMPLING_FREQ_44100:

            switch (mode) {
                case BT_A2DP_CHANNEL_MODE_MONO:
                case BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL:
                    return 31;

                case BT_A2DP_CHANNEL_MODE_STEREO:
                case BT_A2DP_CHANNEL_MODE_JOINT_STEREO:
                    return 53;

                default:
                    LOG(ERROR) << "Invalid channel mode " << mode;
                    return 53;
            }
            break;

        case BT_SBC_SAMPLING_FREQ_48000:

            switch (mode) {
                case BT_A2DP_CHANNEL_MODE_MONO:
                case BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL:
                    return 29;

                case BT_A2DP_CHANNEL_MODE_STEREO:
                case BT_A2DP_CHANNEL_MODE_JOINT_STEREO:
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
    selected_config->frequency = BT_SBC_SAMPLING_FREQ_44100;
    if (input_config->channel_mode & BT_A2DP_CHANNEL_MODE_JOINT_STEREO)
        selected_config->channel_mode = BT_A2DP_CHANNEL_MODE_JOINT_STEREO;
    else if (input_config->channel_mode & BT_A2DP_CHANNEL_MODE_STEREO)
        selected_config->channel_mode = BT_A2DP_CHANNEL_MODE_STEREO;
    else if (input_config->channel_mode & BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL)
        selected_config->channel_mode = BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL;
    else if (input_config->channel_mode & BT_A2DP_CHANNEL_MODE_MONO) {
        selected_config->channel_mode = BT_A2DP_CHANNEL_MODE_MONO;
    } else {
        LOG(ERROR) << "No supported channel modes";
        return false;
    }

    if (input_config->block_length & BT_A2DP_BLOCK_LENGTH_16)
        selected_config->block_length = BT_A2DP_BLOCK_LENGTH_16;
    else if (input_config->block_length & BT_A2DP_BLOCK_LENGTH_12)
        selected_config->block_length = BT_A2DP_BLOCK_LENGTH_12;
    else if (input_config->block_length & BT_A2DP_BLOCK_LENGTH_8)
        selected_config->block_length = BT_A2DP_BLOCK_LENGTH_8;
    else if (input_config->block_length & BT_A2DP_BLOCK_LENGTH_4)
        selected_config->block_length = BT_A2DP_BLOCK_LENGTH_4;
    else {
    	LOG(ERROR) << "No supported block lengths";
        return false;
    }

    if (input_config->subbands & BT_A2DP_SUBBANDS_8)
        selected_config->subbands = BT_A2DP_SUBBANDS_8;
    else if (input_config->subbands & BT_A2DP_SUBBANDS_4)
        selected_config->subbands = BT_A2DP_SUBBANDS_4;
    else {
    	LOG(ERROR) << "No supported sub-bands";
        return false;
    }

    if (input_config->allocation_method & BT_A2DP_ALLOCATION_LOUDNESS)
        selected_config->allocation_method = BT_A2DP_ALLOCATION_LOUDNESS;
    else if (input_config->allocation_method & BT_A2DP_ALLOCATION_SNR)
        selected_config->allocation_method = BT_A2DP_ALLOCATION_SNR;

    selected_config->min_bitpool = (uint8_t) MAX(MIN_BITPOOL, input_config->min_bitpool);
    selected_config->max_bitpool = (uint8_t) MIN(sbc_default_bitpool(selected_config->frequency,
    		selected_config->channel_mode), input_config->max_bitpool);
    *selected_capabilities = reinterpret_cast<uint8_t*>(selected_config);
	return true;
}

} /* namespace dbus */
