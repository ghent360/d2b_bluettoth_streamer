/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin EFremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#include "Connection.h"
#include "Message.h"
#include "MessageArgumentIterator.h"

#include <dbus/dbus.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

//DEFINE_int32(end, 1000, "The last record to read");

class Application {
public:
	Application() {}
	virtual ~Application() {}

	int connectBus() {
		return dbus::Connection::getSystemConnection(&conn_);
	}

	int getAdapterPath(const std::string& device, std::string* path) {
	    dbus::Message msg;
	    dbus::Message reply;
	    std::string s;
	    std::string methodName = FIND_ADAPTER;

	    LOG(INFO) << "Getting object path for adapter " << device;

	    // create a new method call msg
	    if (device.empty()) {
	    	methodName = DEFAULT_ADAPTER;
	    }
	    dbus::Message::forMethodCall("org.bluez",  // Destination
	    		"/",                               // object to call on
	    		"org.bluez.Manager",               // interface to call on
	    		methodName,                        // method name
	    		&msg);

	    // append arguments
	    dbus::MessageArgumentIterator iter = msg.argIterator();
	    if (!device.empty()) {
	    	iter.append(device);
	    }

	    // send message and wait for reply, -1 means wait forever
	    reply = conn_.sendWithReplyAndBlock(msg, -1);
	    if (!reply.msg()) {
	        LOG(ERROR) << "Reply Null\n";
	        return 1;
	    }

	    iter = reply.argIterator();
	    // read the parameters
	    if (iter.hasArgs() && DBUS_TYPE_OBJECT_PATH == iter.getArgumentType()) {
	        s = iter.getObjectPath();
	    } else {
	    	return 1;
	    }

	    // free reply and close connection
	    LOG (INFO) << "Object path for " << device << " is " << s;
	    *path = s;
	    return 0;
	}
private:
	const static std::string FIND_ADAPTER;
	const static std::string DEFAULT_ADAPTER;

	dbus::Connection conn_;
};

const std::string Application::FIND_ADAPTER("FindAdapter");
const std::string Application::DEFAULT_ADAPTER("DefaultAdapter");

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	Application app;
	app.connectBus();
	std::string path;
	app.getAdapterPath("", &path);

	LOG(INFO) << "Exiting audio daemon";
	return 0;
}

