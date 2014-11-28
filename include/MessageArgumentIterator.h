/*
 * MessageArgumentIterator.h
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef MESSAGEARGUMENTITERATOR_H_
#define MESSAGEARGUMENTITERATOR_H_

#include "Message.h"
#include "ObjectPath.h"

#include <dbus/dbus.h>

namespace dbus {

class ContainerIterator;

class BaseMessageIterator {
public:
	virtual ~BaseMessageIterator() {}

	bool next() {
		return dbus_message_iter_next(&iter_) == TRUE;
	}

	bool hasNext() {
		return dbus_message_iter_has_next(&iter_) == TRUE;
	}

	bool append(const char* str) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_STRING,
				&str) == TRUE;
	}

	bool append(const ObjectPath& path) {
		const char* str_value = path.path();
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_OBJECT_PATH,
				&str_value) == TRUE;
	}

	bool append(uint8_t value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_BYTE,
				&value) == TRUE;
	}

	bool append(uint32_t value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_UINT32,
				&value) == TRUE;
	}

	bool append(uint16_t value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_UINT16,
				&value) == TRUE;
	}

	bool append(int32_t value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_INT32,
				&value) == TRUE;
	}

	bool append(int16_t value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_INT16,
				&value) == TRUE;
	}

	bool append(double value) {
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_DOUBLE,
				&value) == TRUE;
	}

	bool append(bool value) {
		dbus_uint32_t val = (value) ? TRUE : FALSE;
		return dbus_message_iter_append_basic(&iter_,
				DBUS_TYPE_BOOLEAN,
				&val) == TRUE;
	}

	bool append(const uint8_t* value, size_t len) {
		return dbus_message_iter_append_fixed_array(&iter_,
				DBUS_TYPE_BYTE, &value, len) == TRUE;
	}

	bool appendVariant(const char* str);
	bool appendVariant(const ObjectPath& path);
	bool appendVariant(uint8_t value);
	bool appendVariant(uint32_t value);
	bool appendVariant(uint16_t value);
	bool appendVariant(int32_t value);
	bool appendVariant(int16_t value);
	bool appendVariant(double value);
	bool appendVariant(bool value);
	bool appendVariant(const uint8_t* value, size_t len);

	bool appendDictEntry(const char* key,
			const char* value);

	bool appendDictEntry(const char* key,
			unsigned char value);

	bool appendDictEntry(const char* key,
			const uint8_t* value, size_t len);

	int getArgumentType() {
		return dbus_message_iter_get_arg_type(&iter_);
	}

	bool getBool();
	uint8_t getByte();
	uint16_t getWord();
	uint32_t getUint32();
	uint64_t getUint64();
	int16_t getShort();
	int32_t getInt32();
	int64_t getInt64();
	double getDouble();

	const char* getString() {
		return getStringForType(DBUS_TYPE_STRING);
	}

	ObjectPath getObjectPath() {
		return ObjectPath(getStringForType(DBUS_TYPE_OBJECT_PATH));
	}

	bool getByteArray(uint8_t** buffer, size_t* len);
	int getFileDescriptor();

	ContainerIterator openDictionary();
	ContainerIterator openContainer(int type, const char* signature);
	ContainerIterator openContainer(int type, int signature);
	ContainerIterator openContainer(int type);

	BaseMessageIterator recurse();
protected:
	const char* getStringForType(int type);

	friend class ContainerIterator;
	DBusMessageIter iter_;
};

class MessageArgumentIterator : public BaseMessageIterator {
public:
	MessageArgumentIterator(Message& msg) {
		has_args_ = dbus_message_iter_init(msg.msg(), &iter_) == TRUE;
	}

	bool hasArgs() const {
		return has_args_;
	}

private:
	bool has_args_;
};

class MessageArgumentBuilder : public BaseMessageIterator {
public:
	MessageArgumentBuilder(Message& msg) {
		dbus_message_iter_init_append(msg.msg(), &iter_);
	}
};

class ContainerIterator : public BaseMessageIterator {
public:
	ContainerIterator(BaseMessageIterator& parent,
			int type,
			const char* signature);
	ContainerIterator(BaseMessageIterator& parent, int type, int signature);
	virtual ~ContainerIterator();

	bool isValid() const {
		return is_valid_;
	}

	void close();
private:
	BaseMessageIterator& parent_;
	bool is_valid_;
};
} /* namespace dbus */

#endif /* MESSAGEARGUMENTITERATOR_H_ */
