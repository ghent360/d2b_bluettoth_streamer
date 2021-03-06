/*
 * SerringsManager.h
 *
 *  Created on: Aug 30, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef SETTINGSMANAGER_H_
#define SETTINGSMANAGER_H_

#include "util.h"

#include <map>
#include <memory>

namespace iqurius {

class SettingsManager {
public:
	virtual ~SettingsManager() {}

	static void open(std::unique_ptr<SettingsManager>& result);

	bool has(const std::string& key) const;
	const char* get(const std::string& key, const char* def_value);
	int32_t get(const std::string& key, int32_t def_value);
	uint32_t get(const std::string& key, uint32_t def_value);
	bool get(const std::string& key, bool def_value);

	void set(const std::string& key, const char* value);
	void set(const std::string& key, const std::string& value);
	void set(const std::string& key, int32_t value);
	void set(const std::string& key, uint32_t value);
	void set(const std::string& key, bool value);
private:
	SettingsManager() {}

	void update();
	bool validate();
	uint32_t calcCRC() const;
	static void formatLine(const std::string& key,
			const std::string& value,
			std::string* output);
	static uint32_t crcLine(const std::string& line);
	bool read(const char* file_name);
	bool write(const char* file_name);

	std::map<std::string, std::string> values_;

	DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};

}  /* namespace iquryus */

#endif /* SETTINGSMANAGER_H_ */

