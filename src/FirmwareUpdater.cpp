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
#include "sha256.h"

#include <sys/mount.h>
#include <unistd.h>
#include <glog/logging.h>

namespace iqurius {

static const char g_FlashMountPoint[] = "/flash";

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

bool FirmwareUpdater::checksumFile(const char* file_path,
		const char* checksum) {
	FILE* input = fopen(file_path, "rb");
	if (NULL == input) {
		return false;
	}
	SHA256 sha;
	unsigned char buffer[8192];
    unsigned char digest[SHA256::DIGEST_SIZE];
    memset(digest,0,SHA256::DIGEST_SIZE);
	int len;
	sha.init();
	do
	{
		len = fread(buffer, 1, sizeof(buffer), input);
		if (len > 0) {
			sha.update(buffer, len);
		}
	} while (len > 0);
	sha.final(digest);
	fclose(input);
	return false;
}

} /* namespace iqurius */
