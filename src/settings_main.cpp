/*
 * settings_main.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014, 2015
 * All rights reserved.
 */

#include "SettingsManager.h"
#include <gflags/gflags.h>
#include <glog/logging.h>

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Settings unit test");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting test";
	std::unique_ptr<iqurius::SettingsManager> sm;
	iqurius::SettingsManager::open(sm);
	LOG(INFO) << "FirstTime=" << sm->get("FirstTime", 1);
	sm->set("FirstTime", 0);
	sm->set("TestValue", 1234);
	sm->set("TestValue2", false);
	sm->set("TestValue3", "=some random text");
	sm->set("TestValue4", "other random text");
	sm->get("TestValue5", getpid());
	LOG(INFO) << "Exiting test";
	return 0;
}
