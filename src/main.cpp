/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */


#include "AacMediaEndpoint.h"
#include "AudioSource.h"
#include "BluezAdapter.h"
#include "BluezAgent.h"
#include "BluezManager.h"
#include "BluezMedia.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DictionaryHelper.h"
#include "ObjectPath.h"
#include "SbcDecodeThread.h"
#include "SbcMediaEndpoint.h"
#include "time_util.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdint.h>

DEFINE_bool(autoconnect, false, "Connect to known devices automatically.");
DEFINE_bool(discover_on_start, false, "Always start the discovery procedure.");

class Application {
public:
	Application()
        : adapter_(NULL),
		  agent_(NULL),
		  media_endpoint_(NULL),
		  adapter_media_interface_(NULL),
		  playback_thread_(NULL) {
	}

	virtual ~Application() {
	    if (playback_thread_) {
	    	playback_thread_->stop();
	    	delete playback_thread_;
	    }
	}

	int connectBus() {
		return dbus::Connection::getSystemConnection(&conn_);
	}

	bool getAdapterPath(const char* device, dbus::ObjectPath* path) {
		dbus::BluezManager bluezManager(&conn_);

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

	dbus::AudioSource* findAudioSource(const dbus::ObjectPath& path) {
		for (dbus::AudioSource* audio_source : audio_sources_) {
			if (audio_source->getPathToSelf() == path) {
                return audio_source;
			}
		}
		return NULL;
	}

	bool isAudioConnected() const {
		for (const dbus::AudioSource* audio_source : audio_sources_) {
			auto state = audio_source->getState();
			if (state == dbus::AudioSource::State::CONNECTED ||
				state == dbus::AudioSource::State::PLAYING) {
                return true;
			}
		}
		return false;
	}

	void initiateConnection() {
		adapter_->refreshProperties();
		for (dbus::ObjectPath device_path : adapter_->getDevices()) {
			dbus::AudioSource* audio_src = findAudioSource(device_path);
			if (audio_src == NULL) {
				audio_src = new dbus::AudioSource(&conn_, device_path);
				audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
						&Application::onStateChange));
				audio_sources_.push_back(audio_src);
				conn_.addObject(audio_src);
			}
			if (FLAGS_autoconnect) {
				audio_src->connectAsync(-1, NULL);
			}
		}
	}

	void startDiscovery() {
		LOG(INFO) << "Start discovery.";
		if (!adapter_->getDiscovering()) {
			adapter_->startDiscovery();
		}
		if (!adapter_->getDiscoverable()) {
			adapter_->setDiscoverable(true);
		}
		if (!adapter_->getPairable()) {
			adapter_->setPairable(true);
		}
	}

	void stopPlayback() {
	    if (playback_thread_) {
	    	playback_thread_->stop();
	    	delete playback_thread_;
	    	playback_thread_ = 0;
	    }
	}

	void startPlayback() {
		stopPlayback();
	    playback_thread_ = new dbus::SbcDecodeThread(&conn_,
	    		media_endpoint_->getTransportPath());
	    playback_thread_->start();
	}

	void onStateChange(dbus::AudioSource::State value,
			const dbus::AudioSource* audio_src) {
		LOG(INFO) << "onStateChange " << audio_src->getPathToSelf()
				<< " " << value;
	    switch (value) {
	    case dbus::AudioSource::State::PLAYING:
	    	if (media_endpoint_->isTransportConfigValid()) {
	    		startPlayback();
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTED:
	    	stopPlayback();
	    	adapter_->setDiscoverable(false);
	    	adapter_->setPairable(false);
	    	adapter_->stopDiscovery();
	    	break;

	    case dbus::AudioSource::State::DISCONNECTED:
	    	stopPlayback();
	    	break;

	    default:
	    	break;
	    }
	}

	bool supports(dbus::BaseMessageIterator& iterator, const char* service_uuid) {
		auto services = iterator.recurse();
		while (services.getArgumentType() == DBUS_TYPE_STRING) {
			const char* service_id = services.getString();
			if (strcasecmp(service_id, service_uuid) == 0) {
				return true;
			}
			services.next();
		}
		return false;
	}

	void onDeviceDiscovered(const char* bt_address, dbus::BaseMessageIterator* property_iterator) {
		dbus::DictionaryHelper properties(property_iterator);
		const char* name = properties.getString("Name");
		bool paired = properties.getBool("Paired");
		//bool trusted = properties.getBool("Trusted");
		auto services = properties.getArray("UUIDs");
		LOG(INFO) << "Discovered " << bt_address << " " << name;
		if (!paired && supports(services, dbus::UIID_AUDIOSOURCE)) {
			LOG(INFO) << "Try to pair with " << name;
		}
	}

	void onDeviceCreated(const dbus::ObjectPath& device_path) {
		LOG(INFO) << "Device created: " << device_path;
		dbus::AudioSource* audio_src = findAudioSource(device_path);
		if (audio_src == NULL) {
			audio_src = new dbus::AudioSource(&conn_, device_path);
			audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
					&Application::onStateChange));
			audio_sources_.push_back(audio_src);
			conn_.addObject(audio_src);
			audio_src->connectAsync(-1, NULL);
		}
	}

	void loop() {
		dbus::ObjectPath adapter_path;
		if (!getAdapterPath("", &adapter_path)) {
			LOG(ERROR) << "Unable to connect to the bluetooth adapter";
			return;
		}

		adapter_ = new dbus::BluezAdapter(&conn_, adapter_path);
		conn_.addObject(adapter_);

		agent_ = new dbus::SimpleBluezAgent(&conn_, 2014);
		conn_.addObject(agent_);

		adapter_->registerAgent(agent_);
		adapter_media_interface_ = new dbus::BluezMedia(&conn_, adapter_path);

		adapter_->setDeviceFoundCallback(googleapis::NewPermanentCallback(
				this, &Application::onDeviceDiscovered));
		adapter_->setDeviceCreatedCallback(googleapis::NewPermanentCallback(
				this, &Application::onDeviceCreated));

		adapter_->setName("Raspberry Sync");
		adapter_->setDiscoverableTimeout(5*60); // 5 minutes;
		adapter_->setPairableTimeout(5*60);

		media_endpoint_ = new dbus::SbcMediaEndpoint();
		//media_endpoint_ = new dbus::AacMediaEndpoint();
		if (!adapter_media_interface_->registerEndpoint(*media_endpoint_)) {
			LOG(ERROR) << "Unable to register the A2DP sync. Check if bluez \n"
					"has audio support and the configuration is enabled.";
			delete media_endpoint_;
			delete adapter_media_interface_;
			delete adapter_;
			return;
		}
		conn_.addObject(media_endpoint_);

		if (FLAGS_discover_on_start) {
			adapter_->startDiscovery();
		}
		initiateConnection();
		uint32_t last_connect_time = timeGetTime();
		if (audio_sources_.empty()) {
			startDiscovery();
		}
		do
		{
			if (!isAudioConnected() &&
				elapsedTime(last_connect_time) > RECONNECT_TIME) {
				LOG(INFO) << "Nothing connected for a while. Retry.";
				initiateConnection();
				last_connect_time = timeGetTime();
				startDiscovery();
			}
			conn_.process(100); // 100ms timeout
		} while (true);
		adapter_media_interface_->unregisterEndpoint(*media_endpoint_);
		delete adapter_media_interface_;
	}
private:
	static const uint32_t RECONNECT_TIME = 15000;
	dbus::Connection conn_;
	dbus::BluezAdapter* adapter_;
	dbus::BluezAgent* agent_;
	dbus::SbcMediaEndpoint* media_endpoint_;
	dbus::BluezMedia* adapter_media_interface_;
	dbus::PlaybackThread* playback_thread_;
	std::list<dbus::AudioSource*> audio_sources_;
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

