/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014, 2015
 * All rights reserved.
 */

#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioTargetControl.h"
#include "BluezAdapter.h"
#include "BluezAgent.h"
#include "BluezDevice.h"
#include "BluezManager.h"
#include "BluezMedia.h"
#include "BluezNames.h"
#include "Connection.h"
#include "CommandParser.h"
#include "DelayedProcessing.h"
#include "DictionaryHelper.h"
#include "FirmwareUpdater.h"
#include "ObjectPath.h"
#include "SbcDecodeThread.h"
#include "SbcMediaEndpoint.h"
#include "SoundFragment.h"
#include "SoundManager.h"
#include "SoundQueue.h"
#include "TextScreen.h"
#include "time_util.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <googleapis/base/callback.h>
#include <googleapis/base/stringprintf.h>
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <stdint.h>

DEFINE_bool(autoconnect, true, "Connect to known devices automatically.");
DEFINE_string(command_file, "/dev/ttyAMA0",
		"File or FIFO where to read commands and write status to.");

static const char* A2DP_UUID = "0000110a-0000-1000-8000-00805f9b34fb";

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
		//msg->dump("onConnectResult: ");
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
const static dbus::StringWithHash CMD_SDWN("SDWN");
//const static dbus::StringWithHash CMD_NEXT("*2");
//const static dbus::StringWithHash CMD_PREV("*3");
const static dbus::StringWithHash CMD_C522("*522");
//const static dbus::StringWithHash CMD_C525("*525");
//const static dbus::StringWithHash CMD_C526("*526");
const static dbus::StringWithHash CMD_C533("*533");
const static dbus::StringWithHash CMD_C534("*534");
const static dbus::StringWithHash CMD_PB01("PB01");
//const static dbus::StringWithHash CMD_PB00("PB00");

class Application {
public:
	Application()
        : adapter_(NULL),
		  agent_(NULL),
		  sbc_media_endpoint_(NULL),
		  adapter_media_interface_(NULL),
		  playback_thread_(NULL),
		  reconnect_token_(0),
		  update_checker_token_(0),
		  shutdown_(false),
		  mixer_(2),
		  sound_queue_(mixer_.getAudioChannel(1), mixer_.getAudioChannel(0)),
		  command_parser_(FLAGS_command_file),
		  phone_connected_(false) {
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
		audio_src->setOnStateChangeCallback(
				googleapis::NewPermanentCallback(this,
						&Application::onAudioStateChange));
		audio_sources_.push_back(audio_src);
		conn_.addObject(audio_src);

		audio_src->setTargetControl(new dbus::AudioTargetControl(&conn_,
				device_path));
		return audio_src;
	}

	void enumerateBluetoothDevices() {
		if (adapter_) {
			adapter_->refreshProperties();
			for (dbus::ObjectPath device_path : adapter_->getDevices()) {
				MyAudioSource* audio_src = findAudioSource(device_path);
				if (!audio_src) {
					dbus::BluezDevice device(&conn_, device_path);
					dbus::DictionaryHelper* properties = NULL;
					device.GetProperties(&properties);
					if (properties) {
						auto services = properties->getArray("UUIDs");
						if (supports(services, A2DP_UUID) && audio_src == NULL) {
							audio_src = createAudioSource(device_path);
						}
					}
					delete properties;
				}
				if (FLAGS_autoconnect && audio_src) {
					audio_src->connectAsync();
				}
			}
		}
	}

	void clearBluetoothDevices() {
		if (adapter_) {
			// Disable watchdog so we don't get rebooted
			command_parser_.sendStatus("@&NOWD\n");
			adapter_->refreshProperties();
			for (dbus::ObjectPath device_path : adapter_->getDevices()) {
				MyAudioSource* audio_src = findAudioSource(device_path);
				if (audio_src && audio_src == sourceConnected()) {
					continue; // Can't remove the currently connected phone
				}
				adapter_->removeDevice(device_path);
			}
			// Re-enable the watchdog.
			command_parser_.sendStatus("@&PING\n");
		}
	}

	void startDiscoverable() {
		MyAudioSource* connected_source = sourceConnected();
		if (adapter_ && !connected_source && !isSourceConnecting()) {
			adapter_->refreshProperties();
			if (!adapter_->getDiscoverable()) {
				LOG(INFO) << "Set discoverable.";
				adapter_->setDiscoverable(true);
				//adapter_->setDiscoverableTimeout(3*60); // 3 minutes;
				sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
						iqurius::SoundManager::SOUND_READY_TO_PAIR));
			}
			if (!adapter_->getPairable()) {
				LOG(INFO) << "Set pairable.";
				adapter_->setPairable(true);
			//  adapter_->setPairableTimeout(3*60);
			}
		}
	}

	void stopDiscoverable() {
		if (adapter_) {
			adapter_->refreshProperties();
			if (adapter_->getDiscoverable()) {
				LOG(INFO) << "Stop discoverable.";
				adapter_->setDiscoverable(false);
			}
			//if (adapter_->getPairable()) {
			//	adapter_->setPairable(false);
			//}
		}
	}

	void stopPlayback() {
	    if (playback_thread_) {
	    	playback_thread_->stop();
	    	delete playback_thread_;
	    	playback_thread_ = 0;
	    	LOG(INFO) << "Stopped playback thread.";
	    }
	}

	void suspendPlayback() {
		LOG(INFO) << "Playback thread suspended.";
	}

	void startPlayback() {
		if (sbc_media_endpoint_ &&
			sbc_media_endpoint_->isTransportConfigValid() &&
			(!playback_thread_ ||
			 !playback_thread_->ok() ||
			 playback_thread_->codecId() != dbus::PlaybackThread::E_SBC)) {
			stopPlayback();
			playback_thread_ = new dbus::SbcDecodeThread(&conn_,
					sbc_media_endpoint_->getTransportPath(),
					mixer_.getAudioChannel(0),
					sbc_media_endpoint_->getSamplingRate());
			playback_thread_->start();
			LOG(INFO) << "Started SBC playback thread.";
			return;
		}
	}

	void delayedPlaybackStatusCheck() {
		command_parser_.sendStatus("@&QSPB\n");
	}

	void onAudioStateChange(dbus::AudioSource::State value,
			dbus::AudioSource* ctx) {
		MyAudioSource* audio_src = reinterpret_cast<MyAudioSource*>(ctx);
		dbus::AudioSource::State prev_state = audio_src->getState();
	    switch (value) {
	    case dbus::AudioSource::State::PLAYING:
	    	if (sbc_media_endpoint_ && sbc_media_endpoint_->isTransportConfigValid()) {
	    		startPlayback();
		    	command_parser_.sendStatus("@&PLAY\n");
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTED:
			LOG(INFO) << "Connected to " << audio_src->getPathToSelf();
	    	// If this device was in playing state, stop the playback,
			// otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
                suspendPlayback();
	    	}
	    	if (!phone_connected_) {
				stopDiscoverable();
	    		phone_connected_ = true;
		    	command_parser_.sendStatus("@&CONN\n");
				iqurius::PostDelayedCallback(PLAYBACK_CHECK_TIME,
					googleapis::NewCallback(this,
							&Application::delayedPlaybackStatusCheck));
				sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
						iqurius::SoundManager::SOUND_CORRECT));
	    	} else {
		    	command_parser_.sendStatus("@&STOP\n");
	    	}
	    	break;

	    case dbus::AudioSource::State::DISCONNECTED:
	    	// If this device was in playing state, stop the playback,
	    	// otherwise ignore.
	    	if (prev_state == dbus::AudioSource::State::PLAYING) {
	    		stopPlayback();
	    	}
	    	command_parser_.sendStatus("@&DISC\n");
	    	if (phone_connected_) {
				phone_connected_ = false;
				sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
						iqurius::SoundManager::SOUND_INCORRECT));
	    	}
	    	break;

	    case dbus::AudioSource::State::CONNECTING:
	    	// We want to differentiate between connect attempts we initiated
	    	// and connections originating from the device, which are user
	    	// initiated.
	    	if (elapsedTime(audio_src->getLastConenctTime()) >
	    			CONNECT_TIMEOUT) {
	    		LOG(INFO) << "Device initiated connect.";

	    		// Get the device we are currently connected to.
    			MyAudioSource* current = sourceConnected();
    			if (NULL != current &&
    				current->getPathToSelf() != ctx->getPathToSelf()) {
    				dbus::AudioSource::State current_state =
    						current->getState();
    				uint32_t time_since_last_play = elapsedTime(
    						current->getLastPlayTime());
    				if (current_state != dbus::AudioSource::State::PLAYING &&
    					time_since_last_play > PLAY_TIMEOUT) {
    					// If the device is not currently playing and it's been
    					// on pause for a while, it's probably ok to disconnect
    					// it.
    					LOG(INFO) << "Disconnecting "
    							<< current->getPathToSelf();
    					current->disconnectAsync();

    					// And try to connect to the device that was initiating
    					// a connection with us.
    					audio_src->connectAsync();
    				}
	    		}
	    	}
	    	break;

	    default:
	    	break;
	    }
	}

	bool supports(dbus::BaseMessageIterator& iterator,
			const char* service_uuid) {
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
			dbus::BluezDevice device(&conn_, device_path);
			dbus::DictionaryHelper* properties = NULL;
			device.GetProperties(&properties);
			if (properties) {
				auto services = properties->getArray("UUIDs");
				if (supports(services, A2DP_UUID)) {
					audio_src = createAudioSource(device_path);
					audio_src->connectAsync();
					sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
							iqurius::SoundManager::SOUND_CORRECT));
				} else {
					LOG(WARNING) << "Device " << device_path << " is not an audio player";
				}
			} else {
				LOG(WARNING) << "Can't fetch properties for " << device_path;
			}
			delete properties;
		}
	}

	void doUpdate() {
		if (updater_.CheckUpdateAvailable()) {
			command_parser_.sendStatus("@&FWUP\n");
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_PLEASE_DONT_TURH_THE_POWER));
			sound_queue_.waitQueueEmpty();
			sound_queue_.autoReplay(true);
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_PREPARING_THE_UPDATE), 1, 1500000);
			if (updater_.UpdateValid()) {
				sound_queue_.waitQueueEmpty();
				sound_queue_.autoReplay(true);
				sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
						iqurius::SoundManager::SOUND_UPDATING));
				if (updater_.Update()) {
					updater_.SyncDisc();
					shutdown_ = true;
					sound_queue_.autoReplay(false);
					sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
							iqurius::SoundManager::SOUND_UPDATE_COMPLETED));
					sound_queue_.waitQueueEmpty();
				} else {
					command_parser_.sendStatus("@&PING\n");
					sound_queue_.autoReplay(false);
					sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
							iqurius::SoundManager::SOUND_INCORRECT));
				}
			} else {
				command_parser_.sendStatus("@&PING\n");
				sound_queue_.autoReplay(false);
				sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
						iqurius::SoundManager::SOUND_INCORRECT));
			}
		}
	}

	void onCommand(const char* command) {
		LOG(INFO) << "Got command: " << command;
		MyAudioSource* connected_source = sourceConnected();
		dbus::StringWithHash cmd(command);
		if (cmd == CMD_SDWN) {
			shutdown_ = true;
			if (connected_source) {
				connected_source->disconnect();
			}
		} else if (cmd == CMD_C522) {
			removeUpdateChecker();
			if (connected_source) {
				connected_source->disconnect();
			}
			doUpdate();
		} else if (cmd == CMD_C533) {
			adapter_->startDiscovery();
		} else if (cmd == CMD_C534) {
			if (connected_source) {
				connected_source->disconnect();
			}
			clearBluetoothDevices();
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_CORRECT));
		} else if (connected_source) {
			auto* control = connected_source->getTargetControl();
			control->refreshProperties();
			control->updatePlayStatus();
			if (cmd == CMD_PLAY || cmd == CMD_CONT || cmd == CMD_PB01) {
				if (control->getStatus() !=
						dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(
							dbus::AudioTargetControl::BUTTON_ID_PLAY);
				}
			} else if (cmd == CMD_PAUS || cmd == CMD_STOP) {
				if (control->getStatus() ==
						dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(
							dbus::AudioTargetControl::BUTTON_ID_PAUSE);
				}
			} else if (cmd == CMD_NTRK) {
				if (control->getStatus() ==
						dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(
							dbus::AudioTargetControl::BUTTON_ID_NEXT);
				}
			} else if (cmd == CMD_PTRK) {
				if (control->getStatus() ==
						dbus::AudioTargetControl::STATUS_PLAYING) {
					control->sendButton(
							dbus::AudioTargetControl::BUTTON_ID_PREV);
				}
			}
		} else {
			//sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
			//		iqurius::SoundManager::SOUND_DISABLED));
		}
	}

	void sendPing() {
		command_parser_.sendStatus("@&PING\n");
	}

	void tryReconnect() {
		MyAudioSource* connected_source = sourceConnected();
		if (NULL == connected_source && !audio_sources_.empty()) {
			LOG(INFO) << "Nothing connected for a while. Retry.";
			enumerateBluetoothDevices();
		}
	}

	void removeReconnect() {
		LOG(INFO) << "Removing reconnect timer proc";
		if (reconnect_token_)
			iqurius::RemoveTimerCallback(reconnect_token_);
		reconnect_token_ = 0;
	}

	void checkForUpdates() {
		if (updater_.CheckUpdateAvailable()) {
			LOG(INFO) << "Found firmware update";
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_UPDATE_IS_AVAILABLE));
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_ENTER_CODE_522));
		}
	}

	void removeUpdateChecker() {
		LOG(INFO) << "Removing update checker timer proc";
		if (update_checker_token_)
			iqurius::RemoveTimerCallback(update_checker_token_);
		update_checker_token_ = 0;
	}

	void updateScreenData() {
		MyAudioSource* connected_source = sourceConnected();
		if (connected_source &&
			connected_source->getState() == dbus::AudioSource::State::PLAYING) {
			auto* control = connected_source->getTargetControl();
			if (!control->getConnected()) {
				//LOG(INFO) << "No AVRCP connection";
				return;
			}
			control->updatePlayStatus();
			control->updateMetadata();
			int track_no = control->getTrackNo();
			int song_pos = control->getSongPos() / 1000;

			LOG(INFO) << "Track " << track_no << " pos " << song_pos;
			if (track_no > 0) {
				std::string track = googleapis::StringPrintf("@&TR%02d\n",
						track_no % 100);
				command_parser_.sendStatus(track.c_str());
			}
			std::string time = googleapis::StringPrintf("@&%02d%02d\n",
					(song_pos / 100) % 100,  // Minutes
					(song_pos) % 100);       // Seconds
			command_parser_.sendStatus(time.c_str());
		} else {
			//LOG(INFO) << "No audo source or not playing.";
		}
	}

	void connectToBluetoothAdapter() {
		if (adapter_) {
			LOG(WARNING) << "Bluetooth adapter already is connected";
			return;
		}

		dbus::ObjectPath adapter_path;
		if (!getAdapterPath("", &adapter_path)) {
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_UNABLE_TO_CONNECT_TO_BLUETOOTH_ADAPTER));
			return;
		}
		adapter_ = new dbus::BluezAdapter(&conn_, adapter_path);
		conn_.addObject(adapter_);

		adapter_->setName("iQuryus JSync V3 Dbg");
		stopDiscoverable();
		adapter_->setDeviceCreatedCallback(
			googleapis::NewPermanentCallback(this,
				&Application::onDeviceCreated));

		agent_ = new dbus::SimpleBluezAgent(&conn_, 2015);
		conn_.addObject(agent_);
		adapter_->registerAgent(agent_);

		adapter_media_interface_ = new dbus::BluezMedia(&conn_,
				adapter_path);
		sbc_media_endpoint_ = new dbus::SbcMediaEndpoint();
		conn_.addObject(sbc_media_endpoint_);
		if (!adapter_media_interface_->registerEndpoint(
				*sbc_media_endpoint_)) {
			LOG(ERROR) << "Unable to register the A2DP sync. Check if bluez \n"
				"has audio support and the configuration is enabled.";
			adapter_->unregisterAgent(agent_);
			conn_.removeObject(agent_);
			agent_ = NULL;
			conn_.removeObject(sbc_media_endpoint_);
			sbc_media_endpoint_ = NULL;
			delete adapter_media_interface_;
			adapter_media_interface_ = NULL;
			conn_.removeObject(adapter_);
			adapter_ = NULL;
			sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
					iqurius::SoundManager::SOUND_UNABLE_TO_CONNECT_TO_BLUETOOTH_ADAPTER));
			return;
		}

		enumerateBluetoothDevices();
		if (audio_sources_.empty()) {
			startDiscoverable();
		}

		if (FLAGS_autoconnect && !reconnect_token_) {
			reconnect_token_ = iqurius::PostTimerCallback(RECONNECT_TIME,
				googleapis::NewPermanentCallback(this,
					&Application::tryReconnect));
			iqurius::PostDelayedCallback(RECONNECT_TIMEOUT,
				googleapis::NewCallback(this, &Application::removeReconnect));
		}

		LOG(INFO) << "Connected to bluetooth adapter " << adapter_path;
		return;
	}

	void mainLoop() {
		uint32_t ping_proc_token;
		uint32_t discoverable_proc_token;

		mixer_.start();
		sound_queue_.start();
		command_parser_.setCommandCllaback(
				googleapis::NewPermanentCallback(this,
						&Application::onCommand));

		// Schedule periodic callbacks
		ping_proc_token = iqurius::PostTimerCallback(0, PING_TIME,
			googleapis::NewPermanentCallback(this,
				&Application::sendPing));

		//iqurius::PostTimerCallback(1000, googleapis::NewPermanentCallback(&text_screen_,
		//					&iqurius::TextScreen::tick));

		iqurius::PostTimerCallback(1000, googleapis::NewPermanentCallback(this,
							&Application::updateScreenData));

		connectToBluetoothAdapter();

		discoverable_proc_token = iqurius::PostTimerCallback(
				DISCOVERY_FLAG_RETRY_TIMEOUT,
			googleapis::NewPermanentCallback(this,
				&Application::startDiscoverable));

		update_checker_token_ = iqurius::PostTimerCallback(0, UPDATE_CHECK_TIME,
			googleapis::NewPermanentCallback(this,
				&Application::checkForUpdates));
		iqurius::PostDelayedCallback(UPDATE_CHECK_TIMEOUT,
			googleapis::NewCallback(this, &Application::removeUpdateChecker));
		shutdown_ = false;
		phone_connected_ = false;
		while (!shutdown_) {
			iqurius::ProcessDelayedCalls();
			command_parser_.process();
			conn_.process(100); // 100ms timeout
		}

		sound_queue_.scheduleFragment(sound_manager_.getSoundPath(
				iqurius::SoundManager::SOUND_RESTARTING));

		removeReconnect();
		removeUpdateChecker();
		iqurius::RemoveTimerCallback(ping_proc_token);
		iqurius::RemoveTimerCallback(discoverable_proc_token);

		sound_queue_.waitQueueEmpty();
		mixer_.getAudioChannel(1)->waitForIdle();
		int rc = system("/usr/sbin/shutdown -h now");
		if (rc) {
		    LOG(ERROR) << "shutdown returned " << rc;
		}
		rc = system("/usr/sbin/halt -p");
		if (rc) {
		    LOG(ERROR) << "halt returned " << rc;
		}
		uint32_t timer = timeGetTime();
		while (elapsedTime(timer) < SHUTDOWN_TIMEOUT) {
			updater_.SyncDisc();
			iqurius::ProcessDelayedCalls();
			conn_.process(100); // 100ms timeout
		}
		if (adapter_) {
			if (sbc_media_endpoint_) {
				adapter_media_interface_->unregisterEndpoint(*sbc_media_endpoint_);
			}
			delete adapter_media_interface_;
		}
		if (sbc_media_endpoint_) {
			conn_.removeObject(sbc_media_endpoint_);
			sbc_media_endpoint_ = NULL;
		}
		if (agent_) {
			adapter_->unregisterAgent(agent_);
			conn_.removeObject(agent_);
			agent_ = NULL;
		}
		if (adapter_) {
			conn_.removeObject(adapter_);
			adapter_ = NULL;
		}
		iqurius::DeletePendingCalls();
		sound_queue_.stop();
		mixer_.stop();
	}
private:
	static const uint32_t RECONNECT_TIME = 20000;
	static const uint32_t RECONNECT_TIMEOUT = 90000;
	static const uint32_t PING_TIME = 1000;
	static const uint32_t CONNECT_TIMEOUT = 10000;
	static const uint32_t PLAY_TIMEOUT = 15000;
	static const uint32_t DISCOVERY_FLAG_RETRY_TIMEOUT = 5000;
	static const uint32_t UPDATE_CHECK_TIME = 10000;
	static const uint32_t UPDATE_CHECK_TIMEOUT = 60000;
	static const uint32_t SHUTDOWN_TIMEOUT = 5000;
	static const uint32_t PLAYBACK_CHECK_TIME = 5000;

	dbus::Connection conn_;
	dbus::BluezAdapter* adapter_;
	dbus::BluezAgent* agent_;
	dbus::SbcMediaEndpoint* sbc_media_endpoint_;
	dbus::BluezMedia* adapter_media_interface_;
	dbus::PlaybackThread* playback_thread_;
	uint32_t reconnect_token_;
	uint32_t update_checker_token_;
	bool shutdown_;
	iqurius::FirmwareUpdater updater_;
	iqurius::AudioMixer mixer_;
	iqurius::SoundQueue sound_queue_;
	iqurius::SoundManager sound_manager_;
	std::list<MyAudioSource*> audio_sources_;
	CommandParser command_parser_;
	bool phone_connected_;
};

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	if (lzo_init() != LZO_E_OK) {
		LOG(ERROR) << "Error initializing the LZO library";
		return 1;
	}
	{  // Scope for the destructor execution.
		Application app;
		if (app.connectBus()) {
			LOG(ERROR) << "Can't connect to the system D-Bus.";
			return 2;
		}
		app.mainLoop();
	}
	LOG(INFO) << "Exiting audio daemon";
	return 0;
}
