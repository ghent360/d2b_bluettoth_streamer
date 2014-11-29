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

DEFINE_bool(autoconnect, true, "Connect to known devices automatically.");

class MyAudioSource : public dbus::AudioSource {
public:
	MyAudioSource(dbus::Connection* connection, const dbus::ObjectPath& path)
        : dbus::AudioSource(connection, path),
		  last_connect_time_(0),
		  last_play_time_(0) {}

	void connectAsync() {
		last_connect_time_ = timeGetTime();
		dbus::AudioSource::connectAsync(-1, NULL);
	}

	void disconnectAsync() {
		dbus::AudioSource::disconnectAsync(-1, NULL);
	}

	uint32_t getLastConenctTime() const {
		return last_connect_time_;
	}

	uint32_t getLastPlayTime() const {
		return last_play_time_;
	}
protected:
	virtual void onStateChanged(State new_state) {
		if (state_ == dbus::AudioSource::State::PLAYING &&
		    new_state != dbus::AudioSource::State::PLAYING) {
			last_play_time_ = timeGetTime();
		}
		dbus::AudioSource::onStateChanged(new_state);
	}
private:
	uint32_t last_connect_time_;
	uint32_t last_play_time_;
};

class Application {
public:
	Application()
        : adapter_(NULL),
		  agent_(NULL),
		  media_endpoint_(NULL),
		  adapter_media_interface_(NULL),
		  playback_thread_(NULL),
		  last_connect_time_(0) {
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

	MyAudioSource* findAudioSource(const dbus::ObjectPath& path) {
		for (MyAudioSource* audio_source : audio_sources_) {
			if (audio_source->getPathToSelf() == path) {
                return audio_source;
			}
		}
		return NULL;
	}

	MyAudioSource* sourceConnected() {
		for (MyAudioSource* audio_source : audio_sources_) {
			auto state = audio_source->getState();
			if (state == dbus::AudioSource::State::CONNECTED ||
				state == dbus::AudioSource::State::PLAYING) {
				return audio_source;
			}
		}
		return NULL;
	}

	void initiateConnection() {
		adapter_->refreshProperties();
		for (dbus::ObjectPath device_path : adapter_->getDevices()) {
			MyAudioSource* audio_src = findAudioSource(device_path);
			if (audio_src == NULL) {
				audio_src = new MyAudioSource(&conn_, device_path);
				audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
						&Application::onStateChange));
				audio_sources_.push_back(audio_src);
				conn_.addObject(audio_src);
			}
			if (FLAGS_autoconnect) {
				audio_src->connectAsync();
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

	void stopDiscovery() {
		LOG(INFO) << "Stop discovery.";
		adapter_->refreshProperties();
		if (adapter_->getDiscovering()) {
			adapter_->stopDiscovery();
		}
		if (adapter_->getDiscoverable()) {
			adapter_->setDiscoverable(false);
		}
		if (adapter_->getPairable()) {
			adapter_->setPairable(false);
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
			dbus::AudioSource* ctx) {
		MyAudioSource* audio_src = reinterpret_cast<MyAudioSource*>(ctx);
		dbus::AudioSource::State prev_state = audio_src->getState();
	    switch (value) {
	    case dbus::AudioSource::State::PLAYING:
	    	if (media_endpoint_->isTransportConfigValid()) {
	    		startPlayback();
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTED:
			LOG(INFO) << "Connected to " << audio_src->getPathToSelf();
	    	// If this device was in playing state, stop the playback, otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
	    		stopPlayback();
	    	}
	    	stopDiscovery();
	    	break;

	    case dbus::AudioSource::State::DISCONNECTED:
	    	// If this device was in playing state, stop the playback, otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
	    		stopPlayback();
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTING:
	    	// We want to differentiate between connect attempts we initiated and
	    	// connections originating from the device, which are user initiated.
	    	if (elapsedTime(audio_src->getLastConenctTime()) > CONNECT_TIMEOUT) {
	    		LOG(INFO) << "Device initiated connect.";

    			// Update the last_connect_time_ timer, so we don't initiate group reconnects.
				last_connect_time_ = timeGetTime();

	    		// Get the device we are currently connected to.
    			MyAudioSource* current = sourceConnected();
    			if (NULL != current) {
    				dbus::AudioSource::State current_state = current->getState();
    				uint32_t time_since_last_play = elapsedTime(current->getLastPlayTime());
    				if (current_state != dbus::AudioSource::State::PLAYING &&
    					time_since_last_play > PLAY_TIMEOUT) {
    					// If the device is not currently playing and it's been on pause for a while,
    					// it's probably ok to disconnect it.
    					LOG(INFO) << "Disconnecting " << current->getPathToSelf();
    					current->disconnectAsync();

    					// And try to connect to the device that was initiating a connection with us.
    					audio_src->connectAsync();
    				}
	    		}
	    	}
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

	void onDeviceCreated(const dbus::ObjectPath& device_path) {
		LOG(INFO) << "Device created: " << device_path;
		MyAudioSource* audio_src = findAudioSource(device_path);
		if (audio_src == NULL) {
			audio_src = new MyAudioSource(&conn_, device_path);
			audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
					&Application::onStateChange));
			audio_sources_.push_back(audio_src);
			conn_.addObject(audio_src);
			audio_src->connectAsync();
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

		initiateConnection();
		last_connect_time_ = timeGetTime();
		if (audio_sources_.empty()) {
			startDiscovery();
		}
		do
		{
			if (NULL == sourceConnected() &&
				elapsedTime(last_connect_time_) > RECONNECT_TIME) {
				LOG(INFO) << "Nothing connected for a while. Retry.";
				initiateConnection();
				last_connect_time_ = timeGetTime();
				startDiscovery();
			}
			conn_.process(100); // 100ms timeout
		} while (true);
		adapter_media_interface_->unregisterEndpoint(*media_endpoint_);
		delete adapter_media_interface_;
	}
private:
	static const uint32_t RECONNECT_TIME = 15000;
	static const uint32_t CONNECT_TIMEOUT = 10000;
	static const uint32_t PLAY_TIMEOUT = 15000;

	dbus::Connection conn_;
	dbus::BluezAdapter* adapter_;
	dbus::BluezAgent* agent_;
	dbus::SbcMediaEndpoint* media_endpoint_;
	dbus::BluezMedia* adapter_media_interface_;
	dbus::PlaybackThread* playback_thread_;
	uint32_t last_connect_time_;
	std::list<MyAudioSource*> audio_sources_;
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

