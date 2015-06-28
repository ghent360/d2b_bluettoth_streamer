/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014, 2015
 * All rights reserved.
 */

#include "BluezAdapter.h"
#include "BluezAgent.h"
#include "BluezDevice.h"
#include "BluezManager.h"
#include "BluezNames.h"
#include "Connection.h"
#include "DelayedProcessing.h"
#include "DictionaryHelper.h"
#include "ObjectPath.h"
#include "Serial.h"
#include "TextScreen.h"
#include "time_util.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <googleapis/base/callback.h>
#include <stdint.h>

static const char* SPP_UUID  = "00001101-0000-1000-8000-00805f9b34fb";

class Application {
public:
	Application()
        : adapter_(NULL),
		  agent_(NULL),
		  shutdown_(false),
		  serial_(NULL) {
	}

	virtual ~Application() {
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

	void enumerateBluetoothDevices() {
		if (adapter_) {
			adapter_->refreshProperties();
			for (dbus::ObjectPath device_path : adapter_->getDevices()) {
				dbus::BluezDevice device(&conn_, device_path);
				dbus::DictionaryHelper* properties = NULL;
				device.GetProperties(&properties);
				if (properties) {
					auto services = properties->getArray("UUIDs");
					if (supports(services, SPP_UUID) && serial_ == NULL) {
						if (supports(services, SPP_UUID)) {
							connectSerialPort(device_path);
						}
					}
				}
				delete properties;
			}
		}
	}

	void startDiscoverable() {
		if (adapter_) {
			adapter_->refreshProperties();
			if (!adapter_->getDiscoverable()) {
				LOG(INFO) << "Set discoverable.";
				adapter_->setDiscoverable(true);
			}
			if (!adapter_->getPairable()) {
				LOG(INFO) << "Set pairable.";
				adapter_->setPairable(true);
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
		dbus::BluezDevice device(&conn_, device_path);
		dbus::DictionaryHelper* properties = NULL;
		device.GetProperties(&properties);
		if (properties) {
			auto services = properties->getArray("UUIDs");
			if (supports(services, SPP_UUID)) {
				connectSerialPort(device_path);
			}
		}
		delete properties;
	}

	void onDeviceFound(const char* device_address,
			dbus::BaseMessageIterator* properties) {
		dbus::DictionaryHelper dict(properties);
		bool is_paired = dict.getBool("Paired");
		if (!is_paired) {
			dict.dump("device properties:");
			const char* device_name = dict.getString("Name");
			uint32_t device_class = dict.getUint32("Class");
			if (device_class == 0x1F00 &&
				strncmp(device_name, "iQurius Screen", 14) == 0) {
				LOG(INFO) << "Found screen " << device_name
						<< "(" << device_address << ")";
				dbus::SimpleBluezAgent* agent = new dbus::SimpleBluezAgent(&conn_, 2015);
				conn_.addObject(agent);

				adapter_->createPairedDevice(device_address, agent);
				adapter_->stopDiscovery();
			}
		}
	}

	void onSerialConnectResult(dbus::Message* msg) {
		if (msg && msg->getType() == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
		  dbus::MessageArgumentIterator iter = msg->argIterator();
		  if (iter.hasArgs()) {
			serial_port_path_.assign(iter.getString());
			LOG(INFO) << "Connected serial " << serial_port_path_;
		  }
		}
	}

	void connectSerialPort(const dbus::ObjectPath& device_path) {
		if (serial_) {
			conn_.removeObject(serial_);
			serial_ = NULL;
			serial_port_path_.clear();
		}
		serial_ = new dbus::Serial(&conn_, device_path);
		conn_.addObject(serial_);
		serial_->connectAsync("spp", -1, googleapis::NewCallback(this,
				&Application::onSerialConnectResult));
	}

	void connectToBluetoothAdapter() {
		if (adapter_) {
			LOG(WARNING) << "Bluetooth adapter already is connected";
			return;
		}

		dbus::ObjectPath adapter_path;
		if (!getAdapterPath("", &adapter_path)) {
			return;
		}
		adapter_ = new dbus::BluezAdapter(&conn_, adapter_path);
		conn_.addObject(adapter_);

		adapter_->setName("iQurius JSync V3");

		agent_ = new dbus::SimpleBluezAgent(&conn_, 2015);
		conn_.addObject(agent_);

		adapter_->registerAgent(agent_);

		adapter_->setDeviceCreatedCallback(
			googleapis::NewPermanentCallback(this,
				&Application::onDeviceCreated));

		adapter_->setDeviceFoundCallback(
			googleapis::NewPermanentCallback(this,
				&Application::onDeviceFound));

		enumerateBluetoothDevices();

		LOG(INFO) << "Connected to bluetooth adapter " << adapter_path;
		return;
	}

	void mainLoop() {
		uint32_t discoverable_proc_token;

		connectToBluetoothAdapter();

		discoverable_proc_token = iqurius::PostTimerCallback(
				DISCOVERY_FLAG_RETRY_TIMEOUT,
			googleapis::NewPermanentCallback(this,
				&Application::startDiscoverable));

		shutdown_ = false;
		while (!shutdown_) {
			iqurius::ProcessDelayedCalls();
			conn_.process(100); // 100ms timeout
		}

		iqurius::RemoveTimerCallback(discoverable_proc_token);

		iqurius::DeletePendingCalls();
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
	bool shutdown_;
	dbus::Serial* serial_;
	std::string serial_port_path_;
};

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
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
