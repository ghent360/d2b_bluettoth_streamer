/*
 * FirmwareContainer.h
 *
 *  Created on: Apr 7, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef FIRMWARECONTAINER_H_
#define FIRMWARECONTAINER_H_

#include "util.h"

#include <list>
#include <stdio.h>
#include <string>

namespace iqurius {

#define CONTAINER_MAGIC_NUMBER 0x57465149

constexpr int MAX_DIGEST_SIZE = 256 / 8;

/*
 * Container structure:
 *
 * MAGIC_NUMBER(4 bytes)
 * SHA-MANIFEST
 *
 * [Manifest compressed data]
 *      Manifest-size (4 bytes)
 *      compressed_block_size(4 bytes)
 *      compressed data (variable size)
 *
 * [File compressed data 1]
 *    [Compressed Block 1]
 *      uncompressed_block_size(4 bytes)
 *      compressed_block_size(4 bytes)
 *      compressed data (variable size)
 *    [Compressed Block 2]
 *    ...
 *    [Compressed Block N]
 *    [EOF]
 *      uncompressed_block_size = 0
 *
 * [File compressed data 2]
 *
 * [File compressed data N]
 */
class FirmwareContainerWriter {
public:
	FirmwareContainerWriter(const char* version)
		: version_(version) {
	}

	bool addFile(const char* path,
			size_t prefix_len_ = 0,
			bool to_storage = false);
	bool addFile(const char* path,
			const char* archive_path,
			bool to_storage = false);
	bool writeContainer(const char* path);
private:
	struct FileInfo {
		std::string path_;
		std::string archive_path_;
		uint64_t file_size_;
		bool to_storage_;
		uint8_t digest_[MAX_DIGEST_SIZE];
	};

	bool serializeManifest(uint8_t* buffer, size_t* buffer_len);
	bool calculateFileDigest(const char* path, uint8_t* digest);
	bool compressFile(const FileInfo& file_info,
			FILE* output,
			uint8_t* input_buffer,
			size_t input_buffer_len,
			uint8_t* output_buffer,
			size_t output_buffer_len,
			uint8_t* work_mem);

	std::string version_;
	std::list<FileInfo> manifest_;
	DISALLOW_COPY_AND_ASSIGN(FirmwareContainerWriter);
};

class FirmwareContainerReader {
public:
	FirmwareContainerReader(const char* path)
        : container_path_(path) {
	}

	bool verifyFiles();
	bool loadManifest();
	bool performUpdate(const char* flash_path, const char* storage_path);
private:
	bool deserializeManifest(uint8_t* buffer, size_t buffer_len);
	void revertUpdate(const char* flash_path, const char* storage_path);
	bool checksumFile(const std::string& file_path, const uint8_t* checksum);

	struct FileInfo {
		std::string archive_path_;
		uint64_t file_size_;
		bool to_storage_;
		uint8_t digest_[MAX_DIGEST_SIZE];
	};

	std::string container_path_;
	std::string version_;
	std::list<FileInfo> manifest_;
	DISALLOW_COPY_AND_ASSIGN(FirmwareContainerReader);
};

} /* namespace iqurius */

#endif /* FIRMWARECONTAINER_H_ */
