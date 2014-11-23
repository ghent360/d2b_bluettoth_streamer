/*
 * MessageArgumentIterator.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MessageArgumentIterator.h"

#include <glog/logging.h>

namespace dbus {

const char* BaseMessageIterator::getStringForType(int type) {
	const char* str;
	int arg_type = getArgumentType();
	if (arg_type != type) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected " << type;
		return "";
	}
	dbus_message_iter_get_basic(&iter_, &str);
	return str;
}

uint8_t BaseMessageIterator::getByte() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_BYTE) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_BYTE";
		return 0;
	}
	dbus_int32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return (uint8_t)value;
}

uint16_t BaseMessageIterator::getWord() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_UINT16) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_UINT16";
		return 0;
	}
	dbus_int32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return (uint16_t)value;
}

bool BaseMessageIterator::getBool() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_BOOLEAN) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_BOOLEAN";
		return false;
	}
	dbus_int32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value == TRUE;
}

uint32_t BaseMessageIterator::getUint32() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_UINT32) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_UINT32";
		return false;
	}
	dbus_uint32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

uint64_t BaseMessageIterator::getUint64() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_UINT64) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_UINT64";
		return false;
	}
	dbus_uint64_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

int64_t BaseMessageIterator::getInt64() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_INT64) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_INT64";
		return false;
	}
	dbus_int64_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

int32_t BaseMessageIterator::getInt32() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_INT32) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_INT32";
		return false;
	}
	dbus_int32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

int16_t BaseMessageIterator::getShort() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_INT16) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_INT16";
		return false;
	}
	dbus_int32_t value;
	dbus_message_iter_get_basic(&iter_, &value);
	return (int16_t)value;
}

double BaseMessageIterator::getDouble() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_DOUBLE) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_DOUBLE";
		return false;
	}
	double value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

int BaseMessageIterator::getFileDescriptor() {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_UNIX_FD) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_UNIX_FD";
		return 0;
	}
	int value;
	dbus_message_iter_get_basic(&iter_, &value);
	return value;
}

bool BaseMessageIterator::getByteArray(uint8_t** buffer, size_t* len) {
	int arg_type = getArgumentType();
	if (arg_type != DBUS_TYPE_ARRAY) {
		LOG(ERROR) << "Invalid argument type got " << arg_type <<
				" expected DBUS_TYPE_ARRAY";
		return false;
	}
	BaseMessageIterator itarray = recurse();
	uint8_t* tmp_buffer;
	int size;
	dbus_message_iter_get_fixed_array(&itarray.iter_, &tmp_buffer, &size);
	*buffer = new uint8_t[size];
	memcpy(*buffer, tmp_buffer, size);
	*len = size;
	return true;
}

ContainerIterator BaseMessageIterator::openContainer(int type,
		const char* signature) {
	return ContainerIterator(*this, type, signature);
}

ContainerIterator BaseMessageIterator::openContainer(int type, int signature) {
	return ContainerIterator(*this, type, signature);
}

ContainerIterator BaseMessageIterator::openContainer(int type) {
	return openContainer(type, (const char*)NULL);
}

BaseMessageIterator BaseMessageIterator::recurse() {
	BaseMessageIterator result;
	dbus_message_iter_recurse(&iter_, &result.iter_);
	return result;
}

bool BaseMessageIterator::appendDictEntry(const char* key, const char* value) {
	ContainerIterator dict = openContainer(DBUS_TYPE_DICT_ENTRY);
	if (!dict.isValid()) {
		return false;
	}
	dict.append(key);
	ContainerIterator variant = dict.openContainer(DBUS_TYPE_VARIANT,
			DBUS_TYPE_STRING);
	if (!variant.isValid()) {
		return false;
	}
	return variant.append(value);
}

bool BaseMessageIterator::appendDictEntry(const char* key,
		unsigned char value) {
	ContainerIterator dict = openContainer(DBUS_TYPE_DICT_ENTRY);
	if (!dict.isValid()) {
		return false;
	}
	dict.append(key);
	ContainerIterator variant = dict.openContainer(DBUS_TYPE_VARIANT,
			DBUS_TYPE_BYTE);
	if (!variant.isValid()) {
		return false;
	}
	return variant.append(value);
}

bool BaseMessageIterator::appendDictEntry(const char* key,
		const void* value, size_t len) {
	ContainerIterator dict = openContainer(DBUS_TYPE_DICT_ENTRY);
	if (!dict.isValid()) {
		return false;
	}
	dict.append(key);
	char signature[3];
	signature[0] = 'a';
	signature[1] = DBUS_TYPE_BYTE;
	signature[2] = 0;
	ContainerIterator variant = dict.openContainer(DBUS_TYPE_VARIANT,
			signature);
	if (!variant.isValid()) {
		return false;
	}
	ContainerIterator array = variant.openContainer(DBUS_TYPE_ARRAY,
			DBUS_TYPE_BYTE);
	if (!array.isValid()) {
		return false;
	}
	return array.append(value, len);
}

ContainerIterator::ContainerIterator(BaseMessageIterator& parent,
		int type,
		const char* signature)
    : parent_(parent),
	  is_valid_(false) {
	is_valid_ = dbus_message_iter_open_container(&parent.iter_,
			type,
			signature,
			&iter_) == TRUE;
}

ContainerIterator::ContainerIterator(BaseMessageIterator& parent, int type,
		int signature)
    : parent_(parent),
	  is_valid_(false) {
	const char* sig_string = (const char*)&signature;
	is_valid_ = dbus_message_iter_open_container(&parent.iter_,
			type,
			sig_string,
			&iter_) == TRUE;
}

ContainerIterator::~ContainerIterator() {
	close();
}

void ContainerIterator::close() {
	if (is_valid_) {
		dbus_message_iter_close_container(&parent_.iter_, &iter_);
		is_valid_ = false;
	}
}

} /* namespace dbus */
