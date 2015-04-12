/*
 * FirmwareUpdater.cpp
 *
 *  Created on: Apr 5, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "FirmwareUpdater.h"

#include <dirent.h>
#include <glog/logging.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace iqurius {

static const char g_FlashMountPoint[] = "/flash";
static const char g_StorageMountPoint[] = "/storage";
static const char g_MediaMountPoint[] = "/media/vne";
static const char g_UpdateFileName[] = "iqjs.fwu";

bool FirmwareUpdater::remountFlash(bool read_only) {
	int result;
	unsigned long flags = MS_MGC_VAL | MS_REMOUNT | MS_SYNCHRONOUS;

	if (read_only) {
		flags |= MS_RDONLY;
	}
	result = mount(NULL, g_FlashMountPoint, NULL, flags, NULL);
	if (result < 0) {
		LOG(ERROR) << "Unable to remount the flash file system: "
				<< errno;
		return false;
	}
	return true;
}

void FirmwareUpdater::sync() {
	sync();
}

bool FirmwareUpdater::checkUpdateAvailable() {
	DIR* media_folder = opendir(g_MediaMountPoint);
	bool result = false;
	struct dirent* entry;

	update_file_name_.clear();
	if (NULL == media_folder) {
		LOG(ERROR) << "Can not access the " << g_MediaMountPoint
				<< " folder errno=" << errno;
		return false;
	}
	while (NULL != (entry = readdir(media_folder))) {
		if (entry->d_type == DT_DIR) {
		    struct stat stat_buf;
			std::string update_file_path(g_MediaMountPoint);

			if (update_file_path == "." || update_file_path == "..") {
				continue;
			}
			update_file_path.append("/");
			update_file_path.append(entry->d_name);
			update_file_path.append("/");
			update_file_path.append(g_UpdateFileName);
			if (0 == stat(update_file_path.c_str(), &stat_buf) &&
				stat_buf.st_size != 0) {
				update_file_name_ = update_file_path;
				result = true;
				break;
			}
		}
	}
	closedir(media_folder);
	return result;
}
} /* namespace iqurius */
