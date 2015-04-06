/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014, 2015
 * All rights reserved.
 */


#include "AacMediaEndpoint.h"
#include "AudioSource.h"
#include "AudioTargetControl.h"
#include "BluezAdapter.h"
#include "BluezAgent.h"
#include "BluezManager.h"
#include "BluezMedia.h"
#include "BluezNames.h"
#include "Connection.h"
#include "CommandParser.h"
#include "DictionaryHelper.h"
#include "FirmwareUpdater.h"
#include "ObjectPath.h"
#include "SbcDecodeThread.h"
#include "SbcMediaEndpoint.h"
#include "time_util.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdint.h>

DEFINE_bool(autoconnect, true, "Connect to known devices automatically.");
DEFINE_string(command_file, "/dev/ttyAMA0",
		"File or FIFO where to read commands and write status to.");

class MyAudioSource : public dbus::AudioSource {
public:
	MyAudioSource(dbus::Connection* connection, const dbus::ObjectPath& path)
        : dbus::AudioSource(connection, path),
		  last_connect_time_(0),
		  last_play_time_(0),
		  is_connecting_(false),
		  tgt_ctl_(NULL) {}

	void connectAsync() {
		if (!is_connecting_) {
			last_connect_time_ = timeGetTime();
			is_connecting_ = true;
			dbus::AudioSource::connectAsync(-1, googleapis::NewCallback(this,
					&MyAudioSource::onConnectResult));
		}
	}

	void disconnectAsync() {
		dbus::AudioSource::disconnectAsync(-1, NULL);
	}

	bool isConencting() const { return is_connecting_; }

	uint32_t getLastConenctTime() const {
		return last_connect_time_;
	}

	uint32_t getLastPlayTime() const {
		return last_play_time_;
	}

	void setTargetControl(dbus::AudioTargetControl* tgt_ctl) {
		if (tgt_ctl_) {
			dbus::AudioSource::connection_->removeObject(tgt_ctl_);
		}
		tgt_ctl_ = tgt_ctl;
		if (tgt_ctl_) {
			dbus::AudioSource::connection_->addObject(tgt_ctl_);
		}
	}

	dbus::AudioTargetControl* getTargetControl() {
		return tgt_ctl_;
	}

protected:
	virtual void onStateChanged(State new_state) {
		if (state_ == dbus::AudioSource::State::PLAYING &&
		    new_state != dbus::AudioSource::State::PLAYING) {
			last_play_time_ = timeGetTime();
		}
		dbus::AudioSource::onStateChanged(new_state);
	}

	void onConnectResult(dbus::Message* msg) {
		msg->dump("onConnectResult: ");
		is_connecting_ = false;
	}

private:
	uint32_t last_connect_time_;
	uint32_t last_play_time_;
	bool is_connecting_;
	dbus::AudioTargetControl* tgt_ctl_;
};

const static dbus::StringWithHash CMD_PLAY("PLAY");
const static dbus::StringWithHash CMD_PAUS("PAUS");
const static dbus::StringWithHash CMD_STOP("STOP");
const static dbus::StringWithHash CMD_CONT("CONT");
const static dbus::StringWithHash CMD_NTRK("NTRK");
const static dbus::StringWithHash CMD_PTRK("PTRK");

class Application {
public:
	Application()
        : adapter_(NULL),
		  agent_(NULL),
		  media_endpoint_(NULL),
		  adapter_media_interface_(NULL),
		  playback_thread_(NULL),
		  last_connect_time_(0),
		  last_discovery_time_(0),
		  command_parser_(FLAGS_command_file) {
	}

	virtual ~Application() {
		stopPlayback();
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

	bool isSourceConnecting() {
		for (MyAudioSource* audio_source : audio_sources_) {
			if (audio_source->isConencting()) {
				return true;
			}
		}
		return false;
	}

	MyAudioSource* createAudioSource(dbus::ObjectPath device_path) {
		MyAudioSource* audio_src = new MyAudioSource(&conn_, device_path);
		audio_src->setOnStateChangeCallback(googleapis::NewPermanentCallback(this,
				&Application::onAudioStateChange));
		audio_sources_.push_back(audio_src);
		conn_.addObject(audio_src);

		audio_src->setTargetControl(new dbus::AudioTargetControl(&conn_, device_path));
		return audio_src;
	}

	void initiateConnection() {
		adapter_->refreshProperties();
		for (dbus::ObjectPath device_path : adapter_->getDevices()) {
			MyAudioSource* audio_src = findAudioSource(device_path);
			if (audio_src == NULL) {
				audio_src = createAudioSource(device_path);
			}
			if (FLAGS_autoconnect) {
				audio_src->connectAsync();
				last_connect_time_ = timeGetTime();
			}
		}
	}

	void startDiscoverable() {
		last_discovery_time_ = timeGetTime();
		adapter_->refreshProperties();
		if (!adapter_->getDiscoverable()) {
			LOG(INFO) << "Set discoverable.";
			adapter_->setDiscoverable(true);
			//adapter_->setDiscoverableTimeout(3*60); // 3 minutes;
		}
		if (!adapter_->getPairable()) {
			LOG(INFO) << "Set pairable.";
			adapter_->setPairable(true);
		//  adapter_->setPairableTimeout(3*60);
		}
	}

	void stopDiscoverable() {
		adapter_->refreshProperties();
		if (adapter_->getDiscoverable()) {
			LOG(INFO) << "Stop discoverable.";
			adapter_->setDiscoverable(false);
		}
		//if (adapter_->getPairable()) {
		//	adapter_->setPairable(false);
		//}
	}

	void stopPlayback() {
	    if (playback_thread_) {
	    	playback_thread_->stop();
	    	delete playback_thread_;
	    	playback_thread_ = 0;
	    	LOG(INFO) << "Stopped playback thread.";
	    }
	}

	void startPlayback() {
		stopPlayback();
	    playback_thread_ = new dbus::SbcDecodeThread(&conn_,
	    		media_endpoint_->getTransportPath());
	    playback_thread_->start();
	    LOG(INFO) << "Started playback thread.";
	}

	void onAudioStateChange(dbus::AudioSource::State value,
			dbus::AudioSource* ctx) {
		MyAudioSource* audio_src = reinterpret_cast<MyAudioSource*>(ctx);
		dbus::AudioSource::State prev_state = audio_src->getState();
	    switch (value) {
	    case dbus::AudioSource::State::PLAYING:
	    	if (media_endpoint_->isTransportConfigValid()) {
	    		startPlayback();
		    	command_parser_.sendStatus("@&PLAY\n");
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTED:
			LOG(INFO) << "Connected to " << audio_src->getPathToSelf();
	    	// If this device was in playing state, stop the playback, otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
	    		stopPlayback();
	    	}
	    	stopDiscoverable();
	    	command_parser_.sendStatus("@&CONN\n");
	    	break;

	    case dbus::AudioSource::State::DISCONNECTED:
	    	// If this device was in playing state, stop the playback, otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
	    		stopPlayback();
	    	}
	    	command_parser_.sendStatus("@&DISC\n");
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
    			if (NULL != current &&
    				current->getPathToSelf() != ctx->getPathToSelf()) {
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
			audio_src = createAudioSource(device_path);
			audio_src->connectAsync();
		}
	}

	void onCommand(const char* command) {
		LOG(INFO) << "Got command: " << command;
		MyAudioSource* connected_source = sourceConnected();
		if (connected_source) {
			dbus::StringWithHash cmd(command);
			auto* control = connected_source->getTargetControl();
			control->refreshProperties();
			control->updatePlayStatus();
			if (cmd == CMD_PLAY || cmd == CMD_CONT) {
				if (control->getStatus() != dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(dbus::AudioTargetControl::BUTTON_ID_PLAY);
				}
			} else if (cmd == CMD_PAUS || cmd == CMD_STOP) {
				if (control->getStatus() == dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(dbus::AudioTargetControl::BUTTON_ID_PAUSE);
				}
			} else if (cmd == CMD_NTRK) {
				if (control->getStatus() == dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(dbus::AudioTargetControl::BUTTON_ID_NEXT);
				}
			} else if (cmd == CMD_PTRK) {
				if (control->getStatus() == dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(dbus::AudioTargetControl::BUTTON_ID_PREV);
				}
			}
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

		agent_ = new dbus::SimpleBluezAgent(&conn_, 2015);
		conn_.addObject(agent_);

		adapter_->registerAgent(agent_);
		adapter_media_interface_ = new dbus::BluezMedia(&conn_, adapter_path);

		adapter_->setDeviceCreatedCallback(googleapis::NewPermanentCallback(
				this, &Application::onDeviceCreated));
		adapter_->setName("iQurius JSync");

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

		command_parser_.setCommandCllaback(
				googleapis::NewPermanentCallback(this, &Application::onCommand));

		initiateConnection();
		if (audio_sources_.empty()) {
			startDiscoverable();
		}
		uint32_t start_time = timeGetTime();
		do
		{
			MyAudioSource* connected_source = sourceConnected();
			if (NULL == connected_source &&
				FLAGS_autoconnect &&
				elapsedTime(start_time) < TRY_CONNECT_TIME &&
				!audio_sources_.empty() &&
				elapsedTime(last_connect_time_) > RECONNECT_TIME) {
				LOG(INFO) << "Nothing connected for a while. Retry.";
				initiateConnection();
			}
			if (NULL == connected_source &&
				!isSourceConnecting() &&
				elapsedTime(last_discovery_time_) > DISCOVERY_FLAG_RETRY_TIMEOUT) {
				startDiscoverable();
			}
			command_parser_.process();
			conn_.process(100); // 100ms timeout
		} while (true);
		adapter_media_interface_->unregisterEndpoint(*media_endpoint_);
		delete adapter_media_interface_;
	}
private:
	static const uint32_t TRY_CONNECT_TIME = 90000;
	static const uint32_t RECONNECT_TIME = 20000;
	static const uint32_t CONNECT_TIMEOUT = 10000;
	static const uint32_t PLAY_TIMEOUT = 15000;
	static const uint32_t DISCOVERY_FLAG_RETRY_TIMEOUT = 5000;

	dbus::Connection conn_;
	dbus::BluezAdapter* adapter_;
	dbus::BluezAgent* agent_;
	dbus::SbcMediaEndpoint* media_endpoint_;
	dbus::BluezMedia* adapter_media_interface_;
	dbus::PlaybackThread* playback_thread_;
	uint32_t last_connect_time_;
	uint32_t last_discovery_time_;
	std::list<MyAudioSource*> audio_sources_;
	CommandParser command_parser_;
};

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	iqurius::FirmwareUpdater fwu;
	fwu.checksumFile("/home/vne/raspi/OpenELEC.tv/target/"
			"OpenBT-RPi.arm-devel-20150405165845-r20632-g0cc25f6.system", "");
	Application app;
	app.connectBus();
	app.loop();
	LOG(INFO) << "Exiting audio daemon";
	return 0;
}
