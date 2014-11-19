/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */


#include "AudioSourcePropertyChanged.h"
#include "BluezManager.h"
#include "BluezMedia.h"
#include "Connection.h"
#include "MediaEndpoint.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "ObjectPath.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdint.h>
#include "a2dp-codecs.h"
#include "ipc.h"

//DEFINE_int32(end, 1000, "The last record to read");

#define A2DP_SINK_UUID        "0000110b-0000-1000-8000-00805f9b34fb"
#define A2DP_SINK_ENDPOINT    "/MediaEndpoint/A2DPSink"

class Application {
public:
	Application() {}
	virtual ~Application() {}

	int connectBus() {
		return dbus::Connection::getSystemConnection(&conn_);
	}

	bool getAdapterPath(const char* device, dbus::ObjectPath* path) {
		dbus::BluezManager bluezManager(&conn_);

	    LOG(INFO) << "Getting object path for adapter " << device;

	    // create a new method call msg
	    if (device == NULL || strlen(device) == 0) {
	    	*path = bluezManager.defaultAdapter();
	    } else {
	    	*path = bluezManager.findAdapter(device);
	    }
	    return path->isValid();
	}

	bool registerSinkEndpoint(const dbus::ObjectPath& adapter) {
		dbus::BluezMedia bluezMedia(&conn_, adapter);

		a2dp_sbc_t capabilities;
		capabilities.channel_mode = BT_A2DP_CHANNEL_MODE_MONO | BT_A2DP_CHANNEL_MODE_DUAL_CHANNEL |
									BT_A2DP_CHANNEL_MODE_STEREO | BT_A2DP_CHANNEL_MODE_JOINT_STEREO;
		capabilities.frequency = BT_SBC_SAMPLING_FREQ_16000 | BT_SBC_SAMPLING_FREQ_32000 |
								 BT_SBC_SAMPLING_FREQ_44100 | BT_SBC_SAMPLING_FREQ_48000;
		capabilities.allocation_method = BT_A2DP_ALLOCATION_SNR | BT_A2DP_ALLOCATION_LOUDNESS;
		capabilities.subbands = BT_A2DP_SUBBANDS_4 | BT_A2DP_SUBBANDS_8;
		capabilities.block_length = BT_A2DP_BLOCK_LENGTH_4 | BT_A2DP_BLOCK_LENGTH_8 |
									BT_A2DP_BLOCK_LENGTH_12 | BT_A2DP_BLOCK_LENGTH_16;
		capabilities.min_bitpool = MIN_BITPOOL;
		capabilities.max_bitpool = MAX_BITPOOL;

		return bluezMedia.registerEndpoint(SINK_ENDPOINT,
				SINK_UUID,
				A2DP_CODEC_SBC,
				&capabilities,
				sizeof(capabilities));
	}

	bool unregisterSinkEndpoint(const dbus::ObjectPath& adapter) {
		dbus::BluezMedia bluezMedia(&conn_, adapter);
		return bluezMedia.unregisterEndpoint(SINK_ENDPOINT);
	}

	void loop() {
		dbus::MediaEndpoint mep;
		dbus::MediaEndpointInterface::registerMethods(conn_, &mep);
		conn_.addMethodHandler(new dbus::AudioSourcePropertyChanged(), NULL);
		conn_.mainLoop();
	}
private:
	const static dbus::ObjectPath SINK_ENDPOINT;
	const static char* SINK_UUID;

	dbus::Connection conn_;
};

const dbus::ObjectPath Application::SINK_ENDPOINT(A2DP_SINK_ENDPOINT);
const char* Application::SINK_UUID = A2DP_SINK_UUID;

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	Application app;
	app.connectBus();
	dbus::ObjectPath path;
	app.getAdapterPath("", &path);
	app.registerSinkEndpoint(path);
	app.loop();
    app.unregisterSinkEndpoint(path);
	LOG(INFO) << "Exiting audio daemon";
	return 0;
}

