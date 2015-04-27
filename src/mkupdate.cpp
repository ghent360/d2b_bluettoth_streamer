/*
 * mkupdate.cc
 *
 *  Created on: Apr 27, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "FirmwareContainer.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <stdint.h>

DEFINE_string(update_version, "", "Version string for this update package.");
DEFINE_string(file_prefix, "OpenBT-RPi.arm-6.0-dev", "Prefix for the file names.");
DEFINE_string(output_path, "iqfw_update.fwu", "Output filename.");

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Create a firmware update container file");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	if (lzo_init() != LZO_E_OK) {
		LOG(ERROR) << "Error initializing the LZO library";
		return 1;
	}
	iqurius::FirmwareContainerWriter writer(FLAGS_update_version.c_str());
	std::string file_name;
	file_name = FLAGS_file_prefix;
	file_name.append(".kernel");
	writer.addFile(file_name.c_str(), "kernel.img", false);
	file_name = FLAGS_file_prefix;
	file_name.append(".system");
	writer.addFile(file_name.c_str(), "SYSTEM", false);
	writer.writeContainer(FLAGS_output_path.c_str());
	return 0;
}
