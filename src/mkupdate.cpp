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

#include <dirent.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <stdint.h>

DEFINE_string(update_version, "", "Version string for this update package.");
DEFINE_string(output_path, "iqjs.fwu", "Output filename.");

using gflags::ParseCommandLineFlags;
using gflags::SetUsageMessage;
using google::InitGoogleLogging;
using iqurius::FirmwareContainerWriter;
using std::string;

void addFolder(const string& path,
		const string& dest_dir,
		FirmwareContainerWriter* writer) {
  DIR* dir = opendir(path.c_str());

  if (!dir) {
	  LOG(ERROR) << "Can not open " << path << " errno=" << errno;
	  return;
  }
  for (struct dirent *d = readdir(dir); d; d = readdir(dir)) {
	  if (d->d_type != DT_REG) continue;
	  string src_path(path);
	  src_path.append("/");
	  src_path.append(d->d_name);

	  string dest_path(dest_dir);
	  dest_path.append(d->d_name);
      LOG(INFO) << "Adding " << src_path << " as " << dest_path;
      writer->addFile(src_path.c_str(), dest_path.c_str(), false);
  }
}
int main(int argc, char *argv[]) {
	SetUsageMessage("Create a firmware update container file");
	ParseCommandLineFlags(&argc, &argv, true);
	InitGoogleLogging(argv[0]);
	if (lzo_init() != LZO_E_OK) {
		LOG(ERROR) << "Error initializing the LZO library";
		return 1;
	}
	FirmwareContainerWriter writer(FLAGS_update_version.c_str());
	writer.addFile("target/KERNEL", "kernel.img", false);
	writer.addFile("target/SYSTEM", "SYSTEM", false);
	addFolder("3rdparty/bootloader", "", &writer);
	addFolder("3rdparty/bootloader/overlays", "overlays/", &writer);
	writer.writeContainer(FLAGS_output_path.c_str());
	return 0;
}
