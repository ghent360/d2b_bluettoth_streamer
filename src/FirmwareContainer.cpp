/*
 * FirmwareContainer.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "FirmwareContainer.h"

#include <lzo/lzo1x.h>
#include <gcrypt.h>
#include <glog/logging.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace iqurius {

static const int g_HashAlgo = GCRY_MD_SHA256;
static constexpr uint32_t g_BlockSize = 1024*1024;

static void writeUint64(uint64_t value, uint8_t* buffer) {
	buffer[0] = value & 0xff;
	buffer[1] = (value >> 8) & 0xff;
	buffer[2] = (value >> 16) & 0xff;
	buffer[3] = (value >> 24) & 0xff;
	buffer[4] = (value >> 32) & 0xff;
	buffer[5] = (value >> 40) & 0xff;
	buffer[6] = (value >> 48) & 0xff;
	buffer[7] = (value >> 56) & 0xff;
}

static void writeUint32(uint32_t value, uint8_t* buffer) {
	buffer[0] = value & 0xff;
	buffer[1] = (value >> 8) & 0xff;
	buffer[2] = (value >> 16) & 0xff;
	buffer[3] = (value >> 24) & 0xff;
}

static void writeUint16(uint16_t value, uint8_t* buffer) {
	buffer[0] = value & 0xff;
	buffer[1] = (value >> 8) & 0xff;
}

static bool writeString(const std::string& value, uint8_t** buffer,
		size_t* buffer_len) {
	size_t len = value.length();
    if (len > 255 || *buffer_len < (len + 1)) {
    	return false;
    }
    const char* str = value.c_str();

    **buffer = (uint8_t)len;
    (*buffer)++;
    (*buffer_len) -= len + 1;
    for (;len; --len) {
    	**buffer = *str++;
    	(*buffer)++;
    }
    return true;
}

bool FirmwareContainerWriter::calculateFileDigest(const char* path,
		uint8_t* digest) {
	const unsigned int digest_len = gcry_md_get_algo_dlen(g_HashAlgo);
	unsigned char buffer[16384];
	FILE* input = NULL;
	gcry_md_hd_t hd = NULL;
	gcry_error_t error;
	unsigned char *file_digest;
	int len;
	bool result = false;

	input = fopen(path, "rb");
	if (NULL == input) {
		LOG(ERROR) << "Can not open the input file: " << path;
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
	memcpy(digest, file_digest, digest_len);
	result = true;
exit:
    if (hd) gcry_md_close(hd);
    if (input) fclose(input);
	return result;
}

bool FirmwareContainerWriter::addFile(const char* path,
		size_t prefix_len,
		bool to_storage) {
	std::string archive_path(path);
	if (prefix_len > 0) {
		if (prefix_len > archive_path.length()) {
			LOG(ERROR) << "Invalid prefix length for " << path;
			return false;
		}
		archive_path = archive_path.substr(prefix_len);
	}
	return addFile(path, archive_path.c_str(), to_storage);
}

bool FirmwareContainerWriter::addFile(const char* path,
		const char* archive_path,
		bool to_storage) {
    struct stat stat_buf;
    FileInfo fi;
    int rc;

    rc = stat(path, &stat_buf);
    if (rc != 0) {
    	LOG(ERROR) << "Can not stat " << path << " errno=" << errno;
    	return false;
    }
    if (0 == archive_path) {
    	archive_path = path;
    }
    fi.to_storage_ = to_storage;
    fi.path_ = path;
    fi.archive_path_ = archive_path;
    fi.file_size_ = stat_buf.st_size;
    memset(fi.digest_, 0, sizeof(fi.digest_));
    if (!calculateFileDigest(path, fi.digest_)) {
    	return false;
    }
    manifest_.push_back(fi);
    return true;
}

bool FirmwareContainerWriter::serializeManifest(uint8_t* buffer,
		size_t* buffer_len) {
    if (!writeString(version_, &buffer, buffer_len)) return false;

    if (*buffer_len < 2 || manifest_.size() > 0xffff) return false;
    writeUint16((uint16_t)manifest_.size(), buffer);
    (*buffer_len) -= 2;
    buffer += 2;

    for (const auto& file_info : manifest_) {
        if (!writeString(file_info.archive_path_, &buffer, buffer_len)) {
        	return false;
        }
        if (*buffer_len < MAX_DIGEST_SIZE + 9) return false;
        writeUint64(file_info.file_size_, buffer);
        (*buffer_len) -= 8;
        buffer += 8;
        *buffer = file_info.to_storage_ ? 1 : 0;
        (*buffer_len)--;
        buffer++;
        memcpy(buffer, file_info.digest_, MAX_DIGEST_SIZE);
        (*buffer_len) -= MAX_DIGEST_SIZE;
        buffer += MAX_DIGEST_SIZE;
    }
	return true;
}

static bool fwriteUint32(uint32_t value, FILE* output) {
    uint8_t buffer[4];
    size_t write_len;

    buffer[0] = value & 0xff;
    buffer[1] = (value >> 8) & 0xff;
    buffer[2] = (value >> 16) & 0xff;
    buffer[3] = (value >> 24) & 0xff;
    write_len = fwrite(buffer, 1, 4, output);
    if (write_len != 4) {
    	LOG(ERROR) << "Error writing to the output file";
    	return false;
    }
    return true;
}

bool FirmwareContainerWriter::writeContainer(const char* path) {
	FILE* output = NULL;
	bool result = false;
	uint8_t* input_buffer = NULL;
	size_t input_buffer_len;
	lzo_uint output_buffer_len;
	size_t write_len;
	uint8_t* compression_buffer = NULL;
	uint8_t* work_memory = NULL;
	uint8_t  manifest_digest[MAX_DIGEST_SIZE];
	int rc;

	output = fopen(path, "wb");
	if (NULL == output) {
		LOG(ERROR) << "Error opening the output file: " << output;
		goto exit;
	}
	input_buffer = new uint8_t[g_BlockSize];
	if (NULL == input_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	compression_buffer = new uint8_t[g_BlockSize + g_BlockSize / 16 + 64 + 3];
	if (NULL == compression_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	work_memory = new uint8_t[LZO1X_999_MEM_COMPRESS];
	if (NULL == work_memory) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}

	if (!fwriteUint32(CONTAINER_MAGIC_NUMBER, output)) {
		goto exit;
	}

	input_buffer_len = g_BlockSize;
	if (!serializeManifest(input_buffer, &input_buffer_len)) {
		LOG(ERROR) << "Can not serialize the manifest.";
		goto exit;
	}
	input_buffer_len = g_BlockSize - input_buffer_len;
	gcry_md_hash_buffer(g_HashAlgo, manifest_digest, input_buffer,
			input_buffer_len);
	write_len = fwrite(manifest_digest, 1, sizeof(manifest_digest), output);
	if (write_len != sizeof(manifest_digest)) {
		LOG(ERROR) << "Error writing to the output file";
		goto exit;
	}

	output_buffer_len = g_BlockSize + g_BlockSize / 16 + 64 + 3;
	rc = lzo1x_999_compress(input_buffer, input_buffer_len,
			compression_buffer, &output_buffer_len, work_memory);
    if (rc != LZO_E_OK) {
		LOG(ERROR) << "Error compressing the manifest buffer";
		goto exit;
    }

    if (!fwriteUint32(input_buffer_len, output)) {
		goto exit;
	}
	if (output_buffer_len < input_buffer_len) {
	    if (!fwriteUint32(output_buffer_len, output)) {
			goto exit;
		}
		write_len = fwrite(compression_buffer, 1, output_buffer_len, output);
		if (write_len != output_buffer_len) {
			LOG(ERROR) << "Error writing to the output file";
			goto exit;
		}
	} else {
	    if (!fwriteUint32(input_buffer_len, output)) {
			goto exit;
		}
		write_len = fwrite(input_buffer, 1, input_buffer_len, output);
		if (write_len != input_buffer_len) {
			LOG(ERROR) << "Error writing to the output file";
			goto exit;
		}
	}
    for (const auto& file_info : manifest_) {
    	if (!compressFile(file_info,
    			output,
				input_buffer,
				g_BlockSize,
				compression_buffer,
				g_BlockSize + g_BlockSize/16 + 64 + 3,
				work_memory)) {
    		goto exit;
    	}
    }
    result = true;
exit:
    delete [] work_memory;
    delete [] compression_buffer;
    delete [] input_buffer;
    if (output) fclose(output);
	return result;
}

bool FirmwareContainerWriter::compressFile(const FileInfo& file_info,
		FILE* output,
		uint8_t* input_buffer,
		size_t input_buffer_len,
		uint8_t* output_buffer,
		size_t output_buffer_len,
		uint8_t* work_memory) {
	FILE* input = NULL;
	size_t len;
	lzo_uint output_len;
	size_t write_len;
	int rc;
	bool result = false;

	input = fopen(file_info.path_.c_str(), "rb");
	if (NULL == input) {
		LOG(ERROR) << "Can not open " << file_info.path_;
		goto exit;
	}
	while (true) {
		len = fread(input_buffer, 1, input_buffer_len, input);
		if (len <= 0) break;

		if (!fwriteUint32(len, output)) {
			goto exit;
		}
		output_len = output_buffer_len;
		rc = lzo1x_999_compress(input_buffer, len,
				output_buffer, &output_len, work_memory);
	    if (rc != LZO_E_OK) {
			LOG(ERROR) << "Error compressing file buffer";
			goto exit;
	    }
		if (output_len < len) {
			if (!fwriteUint32(output_len, output)) {
				goto exit;
			}
			write_len = fwrite(output_buffer, 1, output_len, output);
			if (write_len != output_len) {
				LOG(ERROR) << "Error writing to the output file";
				goto exit;
			}
		} else {
			if (!fwriteUint32(len, output)) {
				goto exit;
			}
			write_len = fwrite(input_buffer, 1, len, output);
			if (write_len != len) {
				LOG(ERROR) << "Error writing to the output file";
				goto exit;
			}
		}
	}
	if (!fwriteUint32(0, output)) {
		goto exit;
	}
	result = true;
exit:
    if (input) fclose(input);
	return result;
}

static bool freadUint32(uint32_t* value, FILE* input) {
    uint8_t buffer[4];
    size_t read_len;

    read_len = fread(buffer, 1, 4, input);
    if (read_len != 4) {
    	LOG(ERROR) << "Error reading from the input file";
    	return false;
    }
    *value = buffer[0] |
    		(buffer[1] << 8) |
			(buffer[2] << 16) |
			(buffer[3] << 24);
    return true;
}

bool FirmwareContainerReader::loadManifest() {
	FILE* input = NULL;
	bool result = false;
	uint8_t* compressed_buffer = NULL;
	uint8_t* output_buffer = NULL;
	uint8_t  manifest_digest[MAX_DIGEST_SIZE];
	uint8_t  buffer_digest[MAX_DIGEST_SIZE];
	const unsigned int digest_len = gcry_md_get_algo_dlen(g_HashAlgo);
	uint32_t magic;
	uint32_t input_len;
	uint32_t compressed_len;
	lzo_uint output_len;
	size_t read_len;
	int rc;

	input = fopen(container_path_.c_str(), "rb");
	if (NULL == input) {
		LOG(ERROR) << "Unable to open container " << container_path_;
		goto exit;
	}
	if (!freadUint32(&magic, input)) {
		goto exit;
	}
	if (magic != CONTAINER_MAGIC_NUMBER) {
		LOG(ERROR) << "Invalid signature";
		goto exit;
	}
	output_buffer = new uint8_t[g_BlockSize];
	if (NULL == output_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	compressed_buffer = new uint8_t[g_BlockSize];
	if (NULL == compressed_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	read_len = fread(manifest_digest, 1, sizeof(manifest_digest), input);
	if (read_len != sizeof(manifest_digest)) {
		LOG(ERROR) << "Error reading input file.";
		goto exit;
	}
	if (!freadUint32(&input_len, input)) {
		goto exit;
	}
	if (!freadUint32(&compressed_len, input)) {
		goto exit;
	}
	read_len = fread(compressed_buffer, 1, compressed_len, input);
	if (read_len != compressed_len) {
		LOG(ERROR) << "Error reading input file.";
		goto exit;
	}
	if (compressed_len < input_len) {
		output_len = g_BlockSize;
		rc = lzo1x_decompress_safe(compressed_buffer,
				compressed_len,
				output_buffer,
				&output_len,
				NULL);
		if (rc != LZO_E_OK || output_len != input_len) {
			LOG(ERROR) << "Error decompressin ght manifest";
			goto exit;
		}
	} else {
		memcpy(output_buffer, compressed_buffer, input_len);
	}
	gcry_md_hash_buffer(g_HashAlgo, buffer_digest, output_buffer,
				input_len);
	if (0 != memcmp(buffer_digest, manifest_digest, digest_len)) {
		LOG(ERROR) << "Manifest signature does not match";
		goto exit;
	}
	if (!deserializeManifest(output_buffer, input_len)) {
		goto exit;
	}
	result = true;
exit:
    delete [] compressed_buffer;
    delete [] output_buffer;
    if (input) fclose(input);
	return result;
}

static void readUint64(uint64_t* value, uint8_t* buffer) {
	*value = buffer[0] |
			((uint64_t)buffer[1] << 8) |
			((uint64_t)buffer[2] << 16) |
			((uint64_t)buffer[3] << 24) |
			((uint64_t)buffer[4] << 32) |
			((uint64_t)buffer[5] << 40) |
			((uint64_t)buffer[6] << 48) |
			((uint64_t)buffer[7] << 56);
}

static void readUint32(uint32_t* value, uint8_t* buffer) {
	*value = buffer[0] |
			((uint64_t)buffer[1] << 8) |
			((uint64_t)buffer[2] << 16) |
			((uint64_t)buffer[3] << 24);
}

static void readUint16(uint16_t* value, uint8_t* buffer) {
	*value = buffer[0] | ((uint64_t)buffer[1] << 8);
}

static bool readString(std::string* value, uint8_t** buffer,
		size_t* buffer_len) {
    if (*buffer_len < 1) {
    	return false;
    }
	size_t len = **buffer;
    (*buffer)++;
    (*buffer_len)--;
    if (*buffer_len < len) {
    	return false;
    }
   	value->append((char*)*buffer, len);
    (*buffer_len) -= len;
    (*buffer )+= len;
    return true;
}

bool FirmwareContainerReader::deserializeManifest(uint8_t* buffer,
		size_t buffer_len) {
	uint16_t file_cnt;
	readString(&version_, &buffer, &buffer_len);
	if (buffer_len < 2) return false;

	readUint16(&file_cnt, buffer);
	buffer += 2;
	buffer_len -= 2;

	manifest_.clear();
	for (;file_cnt; --file_cnt) {
		FileInfo fi;

		readString(&fi.archive_path_, &buffer, &buffer_len);
		if (buffer_len < MAX_DIGEST_SIZE + 9) return false;
		readUint64(&fi.file_size_, buffer);
		buffer += 8;
		fi.to_storage_ = *buffer != 0;
		buffer++;
		memcpy(fi.digest_, buffer, MAX_DIGEST_SIZE);
		buffer += MAX_DIGEST_SIZE;
		buffer_len -= MAX_DIGEST_SIZE + 9;
		manifest_.push_back(fi);
	}
	return buffer_len == 0;
}

bool FirmwareContainerReader::verifyFiles() {
	FILE* input = NULL;
	uint32_t input_len;
	uint32_t compressed_len;
	gcry_md_hd_t hd = NULL;
	gcry_error_t error;
	uint8_t* compressed_buffer = NULL;
	uint8_t* output_buffer = NULL;
	const unsigned int digest_len = gcry_md_get_algo_dlen(g_HashAlgo);
	unsigned char *file_digest;
	size_t read_len;
	lzo_uint output_len;
	int rc;
	bool result = false;

	if (manifest_.size() == 0) {
		LOG(ERROR) << "Please call loadManifest first";
		goto exit;
	}
	input = fopen(container_path_.c_str(), "rb");
	if (NULL == input) {
		LOG(ERROR) << "Unable to open container " << container_path_;
		goto exit;
	}
	if (0 != fseek(input, MAX_DIGEST_SIZE + 8, SEEK_SET)) {
		LOG(ERROR) << "Can't seek in the container errno=" << errno;
		goto exit;
	}
	if (!freadUint32(&compressed_len, input)) {
		goto exit;
	}
	if (0 != fseek(input, compressed_len, SEEK_CUR)) {
		LOG(ERROR) << "Can't seek in the container errno=" << errno;
		goto exit;
	}

	output_buffer = new uint8_t[g_BlockSize];
	if (NULL == output_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	compressed_buffer = new uint8_t[g_BlockSize];
	if (NULL == compressed_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	for (FileInfo fi : manifest_) {
		if (hd) {
			gcry_md_close(hd);
			hd = NULL;
		}
		error = gcry_md_open(&hd, g_HashAlgo, 0);
		if (error != 0) {
			LOG(ERROR) << "Can not initialize the gcrypt hash library: "
					<< error;
			goto exit;
		}
		do {
			if (!freadUint32(&input_len, input)) {
				goto exit;
			}
			if (0 == input_len) break;
			if (!freadUint32(&compressed_len, input)) {
				goto exit;
			}
			read_len = fread(compressed_buffer, 1, compressed_len, input);
			if (read_len != compressed_len) {
				LOG(ERROR) << "Error reading input file.";
				goto exit;
			}
			if (compressed_len < input_len) {
				output_len = g_BlockSize;
				rc = lzo1x_decompress_safe(compressed_buffer,
						compressed_len,
						output_buffer,
						&output_len,
						NULL);
				if (rc != LZO_E_OK || output_len != input_len) {
					LOG(ERROR) << "Error decompressin ght manifest";
					goto exit;
				}
			} else {
				memcpy(output_buffer, compressed_buffer, input_len);
			}
			gcry_md_write(hd, output_buffer, input_len);
		} while (true);
		file_digest = gcry_md_read(hd, g_HashAlgo);
		if (0 != memcmp(fi.digest_, file_digest, digest_len)) {
			LOG(ERROR) << "File checksum mismatch.";
			goto exit;
		}
	}
	result = true;
exit:
	delete [] compressed_buffer;
	delete [] output_buffer;
	if (hd) gcry_md_close(hd);
    if (input) fclose(input);
	return result;
}

bool FirmwareContainerReader::performUpdate(const char* flash_path,
		const char* storage_path) {
	FILE* input = NULL;
	FILE* output = NULL;
	uint32_t input_len;
	uint32_t compressed_len;
	uint8_t* compressed_buffer = NULL;
	uint8_t* output_buffer = NULL;
	size_t read_len;
	lzo_uint output_len;
	int rc;
	bool result = false;
    struct stat stat_buf;

	if (manifest_.size() == 0) {
		LOG(ERROR) << "Please call loadManifest first";
		goto exit;
	}
	input = fopen(container_path_.c_str(), "rb");
	if (NULL == input) {
		LOG(ERROR) << "Unable to open container " << container_path_;
		goto exit;
	}
	if (0 != fseek(input, MAX_DIGEST_SIZE + 8, SEEK_SET)) {
		LOG(ERROR) << "Can't seek in the container errno=" << errno;
		goto exit;
	}
	if (!freadUint32(&compressed_len, input)) {
		goto exit;
	}
	if (0 != fseek(input, compressed_len, SEEK_CUR)) {
		LOG(ERROR) << "Can't seek in the container errno=" << errno;
		goto exit;
	}

	output_buffer = new uint8_t[g_BlockSize];
	if (NULL == output_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	compressed_buffer = new uint8_t[g_BlockSize];
	if (NULL == compressed_buffer) {
		LOG(ERROR) << "Out of memory";
		goto exit;
	}
	for (FileInfo fi : manifest_) {
		std::string output_file_name;
		if (fi.to_storage_) {
			output_file_name = storage_path;
		} else {
			output_file_name = flash_path;
		}
		output_file_name.append(fi.archive_path_);
		output_file_name.append(".updated");

		output = fopen(output_file_name.c_str(), "wb");
		if (NULL == output) {
			LOG(ERROR) << "Can't open output file: " << output_file_name;
			goto exit;
		}
		do {
			if (!freadUint32(&input_len, input)) {
				goto exit;
			}
			if (0 == input_len) break;
			if (!freadUint32(&compressed_len, input)) {
				goto exit;
			}
			read_len = fread(compressed_buffer, 1, compressed_len, input);
			if (read_len != compressed_len) {
				LOG(ERROR) << "Error reading input file.";
				goto exit;
			}
			if (compressed_len < input_len) {
				output_len = g_BlockSize;
				rc = lzo1x_decompress_safe(compressed_buffer,
						compressed_len,
						output_buffer,
						&output_len,
						NULL);
				if (rc != LZO_E_OK || output_len != input_len) {
					LOG(ERROR) << "Error decompressin ght manifest";
					goto exit;
				}
			} else {
				memcpy(output_buffer, compressed_buffer, input_len);
			}
			output_len = fwrite(output_buffer, 1, input_len, output);
			if (output_len != input_len) {
				LOG(ERROR) << "Can't write to output file: "
						<< output_file_name;
				goto exit;
			}
		} while (true);
		fclose(output);
		output = NULL;
		if (!checksumFile(output_file_name, fi.digest_)) {
			LOG(ERROR) << "File checksum does not match: " << output_file_name;
			goto exit;
		}
	}
	// Delete old save files.
	for (FileInfo fi : manifest_) {
		std::string saved_file_name;
		if (fi.to_storage_) {
			saved_file_name = storage_path;
		} else {
			saved_file_name = flash_path;
		}
		saved_file_name.append(fi.archive_path_);
		saved_file_name.append(".old");
		if (0 == stat(saved_file_name.c_str(), & stat_buf)) {
			if (0 != unlink(saved_file_name.c_str())) {
				LOG(ERROR) << "Can not remove " << saved_file_name;
				goto exit;
			}
			if (0 == stat(saved_file_name.c_str(), & stat_buf)) {
				LOG(ERROR) << "File refuses to die " << saved_file_name;
				goto exit;
			}
		}
	}
	sync();
	sync();
	sync();
	// Rename the updates.
	for (FileInfo fi : manifest_) {
		std::string destination_file_name;
		std::string saved_file_name;
		std::string updated_file_name;
		if (fi.to_storage_) {
			destination_file_name = storage_path;
		} else {
			destination_file_name = flash_path;
		}
		destination_file_name.append(fi.archive_path_);
		saved_file_name = destination_file_name;
		updated_file_name = destination_file_name;
		saved_file_name.append(".old");
		updated_file_name.append(".updated");

		if (0 == stat(destination_file_name.c_str(), & stat_buf) &&
		    0 != rename(destination_file_name.c_str(),
				saved_file_name.c_str())) {
			LOG(ERROR) << "Can not rename " << destination_file_name;
			goto exit;
		}
		if (0 != rename(updated_file_name.c_str(),
				destination_file_name.c_str())) {
			LOG(ERROR) << "Can not rename " << updated_file_name;
			goto exit;
		}
	}
	result = true;
exit:
	if (output) fclose(output);
	delete [] compressed_buffer;
	delete [] output_buffer;
    if (input) fclose(input);
    if (!result) {
    	revertUpdate(flash_path, storage_path);
    }
	return result;
}

bool FirmwareContainerReader::checksumFile(const std::string& file_path,
		const uint8_t* input_digest) {
	const unsigned int digest_len = gcry_md_get_algo_dlen(g_HashAlgo);
	unsigned char buffer[16384];
	FILE* input = NULL;
	gcry_md_hd_t hd = NULL;
	gcry_error_t error;
	unsigned char *file_digest;
	int len;
	bool result = false;

	input = fopen(file_path.c_str(), "rb");
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
	return result;
}

void FirmwareContainerReader::revertUpdate(const char* flash_path,
		const char* storage_path) {
	struct stat stat_buf;

	for (FileInfo fi : manifest_) {
		std::string destination_file_name;
		std::string saved_file_name;
		std::string updated_file_name;
		if (fi.to_storage_) {
			destination_file_name = storage_path;
		} else {
			destination_file_name = flash_path;
		}
		destination_file_name.append(fi.archive_path_);
		saved_file_name = destination_file_name;
		updated_file_name = destination_file_name;
		saved_file_name.append(".old");
		updated_file_name.append(".updated");

		if (0 == stat(destination_file_name.c_str(), &stat_buf) &&
		    0 != stat_buf.st_size) {
			// Original file is there and non zero length
			unlink(updated_file_name.c_str());  // Try to remove the tmp file.
			continue;
		}
		if (0 == stat(saved_file_name.c_str(), &stat_buf) &&
		    0 != stat_buf.st_size) {
			// Original file is MIA but save file exist.
			if (0 != rename(saved_file_name.c_str(),
					destination_file_name.c_str())) {
				LOG(ERROR) << "Unable to restore saved file: " << saved_file_name;
			}
			unlink(updated_file_name.c_str());  // Try to remove the tmp file.
			continue;
		}
		// Both the saved tile and original file are MIA.
		if (0 != rename(updated_file_name.c_str(), destination_file_name.c_str())) {
			LOG(ERROR) << "Can not remove " << updated_file_name;
			unlink(updated_file_name.c_str());  // Try to remove the tmp file.
		}
	}
}

} /* namespace iqurius */
