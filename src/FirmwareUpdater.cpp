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

#include <sys/mount.h>
#include <gcrypt.h>
#include <glog/logging.h>
#include <unistd.h>

namespace iqurius {

static const char g_FlashMountPoint[] = "/flash";
static const char g_MediaMountPoint[] = "/media";
static const int g_HashAlgo = GCRY_MD_SHA256;

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

static unsigned char hexDigitFromChar(char c) {
	switch (c) {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;

	case 'a':
	case 'A':
		return 10;
	case 'b':
	case 'B':
		return 11;
	case 'c':
	case 'C':
		return 12;
	case 'd':
	case 'D':
		return 13;
	case 'e':
	case 'E':
		return 14;
	case 'f':
	case 'F':
		return 15;
	}
	return -1;
}

static int byteFromHex(const char* str) {
	unsigned char d1 = hexDigitFromChar(str[0]);
	unsigned char d2 = hexDigitFromChar(str[1]);
	if (d1 == -1 || d2 == -1) {
		return -1;
	}
	return (d1 << 4) | d2;
}

bool FirmwareUpdater::checksumFile(const char* file_path,
		const char* checksum) {
	const size_t checksum_len = strlen(checksum);
	const unsigned int digest_len = gcry_md_get_algo_dlen(g_HashAlgo);
	unsigned char buffer[16384];
    unsigned char* input_digest = NULL;
	FILE* input = NULL;
	gcry_md_hd_t hd = NULL;
	gcry_error_t error;
	unsigned char *file_digest;
	int len;
	bool result = false;

	if (checksum_len != digest_len * 2) {
		LOG(ERROR) << "Invalid checksum length expected " << digest_len *2
				<< " got " << checksum_len;
		goto exit;
	}

	input_digest = new unsigned char[digest_len];
	if (NULL == input_digest) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
    for (int idx = 0; idx < digest_len; ++idx) {
    	int d = byteFromHex(checksum + 2*idx);
    	if (d < 0) {
    		LOG(ERROR) << "Invalid checksum: " << checksum;
    		goto exit;
    	}
    	input_digest[idx] = (unsigned char)d;
    }

	input = fopen(file_path, "rb");
	if (NULL == input) {
		LOG(ERROR) << "Can not open the input file: " << file_path;
		goto exit;
	}

	error = gcry_md_open(&hd, g_HashAlgo, 0);
	if (error != 0) {
		LOG(ERROR) << "Can not initialize the gcrypt hash library: " << error;
		goto exit;
	}

	while (true) {
		len = fread(buffer, 1, sizeof(buffer), input);
		if (len <= 0) break;
		gcry_md_write(hd, buffer, len);
	}
	file_digest = gcry_md_read(hd, g_HashAlgo);
	result = (0 == memcmp(input_digest, file_digest, digest_len));
exit:
    if (hd) gcry_md_close(hd);
    if (input) fclose(input);
    delete [] input_digest;
	return result;
}

} /* namespace iqurius */
