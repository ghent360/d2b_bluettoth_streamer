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
