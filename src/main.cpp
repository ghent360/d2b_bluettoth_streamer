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
#include "BluezManager.h"
#include "BluezMedia.h"
#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"
#include "ObjectPath.h"
#include "SbcDecodeThread.h"
#include "SbcMediaEndpoint.h"
#include "time_util.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdint.h>

class Application {
public:
	Application()
        : adapter_(NULL),
		  media_endpoint_(NULL),
		  adapter_media_interface_(NULL),
		  phone_connected_(false),
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

	const dbus::AudioSource* audioSourceActive(
			const dbus::ObjectPath& path) const {
		for (const dbus::AudioSource* audio_source : audio_sources_) {
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
		for (dbus::ObjectPath device_path : adapter_->getDevices()) {
			if (audioSourceActive(device_path) == NULL) {
				dbus::AudioSource* audio_src = new dbus::AudioSource(&conn_, device_path);
				audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
						&Application::onStateChange));
				audio_sources_.push_back(audio_src);
				conn_.addObject(audio_src);
				audio_src->connectAsync(-1, NULL);
			}
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
	    	break;

	    case dbus::AudioSource::State::DISCONNECTED:
	    	stopPlayback();
	    	break;

	    default:
	    	break;
	    }
	}

	void loop() {
		dbus::ObjectPath adapter_path;
		if (!getAdapterPath("", &adapter_path)) {
			LOG(ERROR) << "Unable to connect to the bluetooth adapter";
			return;
		}

		adapter_ = new dbus::BluezAdapter(&conn_, adapter_path);
		adapter_media_interface_ = new dbus::BluezMedia(&conn_, adapter_path);

		adapter_->setName("A2DP Raspi Sync");
		adapter_->setDiscoverableTimeout(5*60); // 5 minutes;
		adapter_->setPairableTimeout(5*60);
		adapter_->setDiscoverable(true);
		adapter_->setPairable(true);

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

		phone_connected_ = false;
		initiateConnection();
		uint32_t last_connect_time = timeGetTime();
		do
		{
			if (!isAudioConnected() &&
				elapsedTime(last_connect_time) > RECONNECT_TIME) {
				initiateConnection();
				last_connect_time = timeGetTime();
			}
			conn_.process(200);
		} while (true);
		adapter_media_interface_->unregisterEndpoint(*media_endpoint_);
		delete adapter_media_interface_;
		delete adapter_;
	}
private:
	static const uint32_t RECONNECT_TIME = 30000;
	dbus::Connection conn_;
	dbus::BluezAdapter* adapter_;
	dbus::SbcMediaEndpoint* media_endpoint_;
	dbus::BluezMedia* adapter_media_interface_;
	bool phone_connected_;
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

