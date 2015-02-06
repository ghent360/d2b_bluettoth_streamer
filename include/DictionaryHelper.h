/*
 * DictionaryHelper.h
 *
 *  Created on: Nov 22, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014
 * All rights reserved.
 */

#ifndef DICTIONARYHELPER_H_
#define DICTIONARYHELPER_H_

#include "MessageArgumentIterator.h"

#include <glog/logging.h>
#include <map>

namespace dbus {

class DictionaryHelper {
public:
	DictionaryHelper(BaseMessageIterator* dict) {
		parse(dict);
	}


	BaseMessageIterator findProperty(const char* name) {
		return dict_[name];
	}

	const char* getString(const char* name) {
		BaseMessageIterator value = dict_[name];
		return value.getString();
	}

	bool getBool(const char* name) {
		BaseMessageIterator value = dict_[name];
		return value.getBool();
	}

	uint32_t getUint32(const char* name) {
		BaseMessageIterator value = dict_[name];
		return value.getUint32();
	}

	uint8_t getByte(const char* name) {
		BaseMessageIterator value = dict_[name];
		return value.getByte();
	}

	BaseMessageIterator getArray(const char* name) {
		BaseMessageIterator value = dict_[name];
		int type = value.getArgumentType();
		if (DBUS_TYPE_ARRAY != type) {
			LOG(ERROR) << "Invalid argument type got " << type <<
					" expected " << DBUS_TYPE_ARRAY;
		}
		return value;
	}

	void dump(const char* prefix);
private:
	void parse(BaseMessageIterator* dict) {
		if (DBUS_TYPE_ARRAY == dict->getArgumentType()) {
			auto dict_elem = dict->recurse();
			while (DBUS_TYPE_DICT_ENTRY == dict_elem.getArgumentType()) {
				auto dict_pair = dict_elem.recurse();
				if (DBUS_TYPE_STRING == dict_pair.getArgumentType()) {
					const char* key = dict_pair.getString();
					dict_pair.next();
					BaseMessageIterator value = dict_pair.recurse();
					dict_[key] = value;
				}
				dict_elem.next();
			}
		}
	}

	std::map<std::string, BaseMessageIterator> dict_;
};

}  /* namespace dbus */

#endif /* DICTIONARYHELPER_H_ */

