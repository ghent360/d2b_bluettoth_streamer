/*
 * DictionaryHelper.cpp
 *
 *  Created on: Jan 11, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014,2015
 * All rights reserved.
 */
#include "DictionaryHelper.h"

namespace dbus {

void DictionaryHelper::dump(const char* prefix) {
	for(auto& value : dict_) {
		switch (value.second.getArgumentType()) {
		case DBUS_TYPE_BYTE:
			LOG(INFO) << prefix << " " << value.first << " " << (int)value.second.getByte();
			break;
		case DBUS_TYPE_BOOLEAN:
			LOG(INFO) << prefix << " " << value.first << " " << value.second.getBool();
			break;
		case DBUS_TYPE_INT16:
		case DBUS_TYPE_INT32:
			LOG(INFO) << prefix << " " << value.first << " " << value.second.getInt32();
			break;
		case DBUS_TYPE_UINT16:
		case DBUS_TYPE_UINT32:
			LOG(INFO) << prefix << " " << value.first << " " << value.second.getUint32();
			break;
		case DBUS_TYPE_STRING:
			LOG(INFO) << prefix << " " << value.first << " " << value.second.getString();
			break;
		case DBUS_TYPE_ARRAY:
			{
				auto array_iterator = value.second.recurse();
				int idx = 0;
				LOG(INFO) << prefix << " " << value.first << " array:";
				do {
					switch (array_iterator.getArgumentType()) {
					case DBUS_TYPE_STRING:
						LOG(INFO) << prefix << " " << value.first << "[" << idx << "] " <<
							array_iterator.getString();
						break;
					case DBUS_TYPE_OBJECT_PATH:
						LOG(INFO) << prefix << " " << value.first << "[" << idx << "] " <<
							array_iterator.getObjectPath();
						break;
					default:
						LOG(INFO) << prefix << " " << value.first << "[" << idx << "] type: " <<
							array_iterator.getArgumentType();
						break;
					}
					idx++;
				} while (array_iterator.next());
			}
			break;
		default:
			LOG(INFO) << prefix << " " << value.first << " type:"
					<< value.second.getArgumentType();
			break;
		}
	}
}

} /* namepsace dbus */
