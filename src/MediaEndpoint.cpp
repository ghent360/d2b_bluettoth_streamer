/*
 * MediaEndpoint.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MediaEndpoint.h"

#include "Connection.h"
#include "MediaTransportProperties.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "MethodLocator.h"
#include "a2dp-codecs.h"
#include "ipc.h"

#include <glog/logging.h>

#define MAX(a,b) ((a) < (b)) ? (b) : (a)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

namespace dbus {

const char* MediaEndpointInterface::INTERFACE = "org.bluez.MediaEndpoint";
const char* MediaEndpointInterface::SELECTCONFIGURATION_METHOD = "SelectConfiguration";
const char* MediaEndpointInterface::SETCONFIGURATION_METHOD = "SetConfiguration";
const char* MediaEndpointInterface::CLEARCONFIGURATION_METHOD = "ClearConfiguration";
const char* MediaEndpointInterface::RELEASE_METHOD = "Release";

class MediaEndpointSelectConfiguration : public MethodLocator {
public:
	MediaEndpointSelectConfiguration(): MethodLocator(Type::E_METHOD,
			MediaEndpointInterface::INTERFACE,
			MediaEndpointInterface::SELECTCONFIGURATION_METHOD) {}
	virtual Message handle(Message& msg, void* ctx) {
		MediaEndpointInterface* pThis = reinterpret_cast<MediaEndpointInterface*>(ctx);
	    void *capabilities_in;
	    uint8_t *capabilities_out;
	    int size_in, size_out;
	    Message reply;
	    DBusError err;
	    bool success;

	    LOG(INFO) << "MediaEndpoint::selectConfiguration(handler)";
	    if (!dbus_message_get_args(msg.msg(), &err, DBUS_TYPE_ARRAY,
	    		DBUS_TYPE_BYTE, &capabilities_in, &size_in, DBUS_TYPE_INVALID)) {
	    	Connection::handleError(&err, __FUNCTION__, __LINE__);
	    	Message::forError(msg, "org.bluez.MediaEndpoint.Error.InvalidArguments",
	    			"Unable to select configuration", &reply);
	        return reply;
	    }
	    success = pThis->selectConfiguration(capabilities_in, size_in,
	    		&capabilities_out, &size_out);
	    if (success) {
	    	Message::forMethodReturn(msg, &reply);
	    	dbus_message_append_args(reply.msg(), DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
	    			&capabilities_out, size_out, DBUS_TYPE_INVALID);
	    	delete [] capabilities_out;
	    } else {
	    	Message::forError(msg, "org.bluez.MediaEndpoint.Error.InvalidArguments",
	    			"Unable to select configuration", &reply);
	    }
		return reply;
	}
};

class MediaEndpointSetConfiguration : public MethodLocator {
public:
	MediaEndpointSetConfiguration(): MethodLocator(Type::E_METHOD,
			MediaEndpointInterface::INTERFACE,
			MediaEndpointInterface::SETCONFIGURATION_METHOD) {}
	virtual Message handle(Message& msg, void* ctx) {
		MessageArgumentIterator it = msg.argIterator();
        ObjectPath transport = it.getObjectPath();
        it.next();
        MediaTransportProperties transport_properties;
        BaseMessageIterator itprop = it.recurse();
        transport_properties.parseDictionary(&itprop);
        MediaEndpointInterface* pThis = reinterpret_cast<MediaEndpointInterface*>(ctx);
		pThis->setConfiguration(transport, transport_properties);
        Message reply;
        Message::forMethodReturn(msg, &reply);
		return reply;
	}
};

class MediaEndpointClearConfiguration : public MethodLocator {
public:
	MediaEndpointClearConfiguration(): MethodLocator(Type::E_METHOD,
			MediaEndpointInterface::INTERFACE,
			MediaEndpointInterface::CLEARCONFIGURATION_METHOD) {}
	virtual Message handle(Message& msg, void* ctx) {
		MediaEndpointInterface* pThis = reinterpret_cast<MediaEndpointInterface*>(ctx);
		pThis->clearConfiguration(msg.argIterator().getObjectPath());
		Message reply;
		Message::forMethodReturn(msg, &reply);
		return reply;
	}
};

class MediaEndpointRelease : public MethodLocator {
public:
	MediaEndpointRelease(): MethodLocator(Type::E_METHOD,
			MediaEndpointInterface::INTERFACE,
			MediaEndpointInterface::RELEASE_METHOD) {}
	virtual Message handle(Message& msg, void* ctx) {
		MediaEndpointInterface* pThis = reinterpret_cast<MediaEndpointInterface*>(ctx);
		pThis->release();
		Message reply;
		Message::forMethodReturn(msg, &reply);
		return reply;
	}
};

void MediaEndpointInterface::registerMethods(Connection& connection, MediaEndpointInterface* impl) {
    connection.addMethodHandler(new MediaEndpointSelectConfiguration(), impl);
    connection.addMethodHandler(new MediaEndpointSetConfiguration(), impl);
    connection.addMethodHandler(new MediaEndpointClearConfiguration(), impl);
    connection.addMethodHandler(new MediaEndpointRelease(), impl);
}

void MediaEndpointInterface::unregisterMethods(Connection& connection) {
    connection.removeMethodHandler(MediaEndpointSelectConfiguration());
    connection.removeMethodHandler(MediaEndpointSetConfiguration());
    connection.removeMethodHandler(MediaEndpointClearConfiguration());
    connection.removeMethodHandler(MediaEndpointRelease());
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
static uint8_t a2dp_default_bitpool(uint8_t freq, uint8_t mode) {

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

bool MediaEndpoint::selectConfiguration(void* capabilities,
		int capabilities_len,
		uint8_t** selected_capabilities,
		int* selected_capabilities_len) {
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
    selected_config->max_bitpool = (uint8_t) MIN(a2dp_default_bitpool(selected_config->frequency,
    		selected_config->channel_mode), input_config->max_bitpool);
    *selected_capabilities = reinterpret_cast<uint8_t*>(selected_config);
	return false;
}

void MediaEndpoint::setConfiguration(const ObjectPath& transport,
		const MediaTransportProperties& properties) {
	LOG(INFO) << "MediaEndpoint::setConfiguration transport=" << transport.path();
}

void MediaEndpoint::clearConfiguration(const ObjectPath& transport) {
	LOG(INFO) << "MediaEndpoint::clearConfiguration";
}

void MediaEndpoint::release() {
	LOG(INFO) << "MediaEndpoint::release";
}

} /* namespace dbus */
