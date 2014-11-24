/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */


#include <AudioSource.h>
#include "BluezAdapter.h"
#include "BluezManager.h"
#include "BluezMedia.h"
#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "MpegMediaEndpoint.h"
#include "ObjectPath.h"
#include "SbcMediaEndpoint.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdint.h>

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

	void onAudioConnected(dbus::AudioSource* device, dbus::Message* result) {
		if (result == NULL) {
			LOG(ERROR) << "Connect timout " << device->getPathToSelf();
		} else if (result->getType() == DBUS_MESSAGE_TYPE_ERROR ){
			LOG(ERROR) << "Connect error " << device->getPathToSelf();
			result->dump("");
		} else {
			LOG(INFO) << "Connected " << device->getPathToSelf();
		}
	}

	void onDeviceFound(const char* address, dbus::BaseMessageIterator* dict) {
        LOG(INFO) << "Device found " << address;
	}

	void loop() {
		dbus::ObjectPath adapter_path;
		getAdapterPath("", &adapter_path);
		dbus::BluezAdapter* adp = new dbus::BluezAdapter(&conn_, adapter_path);
		dbus::SbcMediaEndpoint* mep1 = new dbus::SbcMediaEndpoint();
		dbus::MpegMediaEndpoint* mep2 = new dbus::MpegMediaEndpoint();
		dbus::BluezMedia* bluezMedia = new dbus::BluezMedia(&conn_, adapter_path);
		adp->setDeviceFoundCallback(googleapis::NewPermanentCallback(this, &Application::onDeviceFound));
		conn_.addObject(adp);
		conn_.addObject(mep1);
		conn_.addObject(mep2);
		bluezMedia->registerEndpoint(*mep1);
		bluezMedia->registerEndpoint(*mep2);
		for (dbus::ObjectPath d : adp->getDevices()) {
			dbus::AudioSource* asrc = new dbus::AudioSource(&conn_, d);
			LOG(INFO) << "Trying to connect to " << d;
			conn_.addObject(asrc);
			asrc->connectAsync(googleapis::NewPermanentCallback(this, &Application::onAudioConnected, asrc));
		}
		do
		{
			conn_.process(200);
		} while (true);
		bluezMedia->unregisterEndpoint(*mep1);
		bluezMedia->unregisterEndpoint(*mep2);
	}
private:
	dbus::Connection conn_;
};

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	Application app;
	app.connectBus();
	app.loop();
	LOG(INFO) << "Exiting audio daemon";
	return 0;
}

