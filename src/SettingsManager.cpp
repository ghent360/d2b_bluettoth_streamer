/*
 * SettingsManager.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */
#include "SettingsManager.h"

#include <fstream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <googleapis/base/stringprintf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

DEFINE_string(settings_location, "/storage/.cache/bt-a2dp",
		"File or FIFO where to read commands and write status to.");

namespace iqurius {

static const char* SETTINGS_FILE_NAME = "bt-a2dp.settings";
static const char* CRC_KEY = "__crc__";

void SettingsManager::open(std::unique_ptr<SettingsManager>& result) {
	SettingsManager* sm = new SettingsManager();
	std::string settings_file_name = FLAGS_settings_location + "/" +
			SETTINGS_FILE_NAME;
	if (sm->read(settings_file_name.c_str())) {
		goto exit;
	}
	for (int sufix = 1; sufix < 10; sufix++) {
		std::string file_name = googleapis::StringPrintf(
				"%s.%d",
				settings_file_name.c_str(),
				sufix);
		if (sm->read(file_name.c_str())) {
			goto exit;
		}
	}
	sm->defaults();
exit:
	result.reset(sm);
}

bool SettingsManager::validate() const {
	uint32_t crc = calcCRC();
	uint32_t file_crc = 0;
	if (!get(CRC_KEY, &file_crc)) {
		return false;
	}
    return file_crc == crc;
}

uint32_t SettingsManager::calcCRC() const {
	uint32_t crc = 0;
	for (const auto& element : values_) {
		if (element.first != CRC_KEY) {
			std::string line;
			formatLine(element.first, element.second, &line);
			crc += crcLine(line);
		}
	}
	return crc;
}

void SettingsManager::formatLine(
		const std::string& key,
		const std::string& value,
		std::string* output) {
	output->clear();
	output->append(key);
	output->append("=");
	output->append(value);
}

uint32_t SettingsManager::crcLine(const std::string& line) {
	uint32_t crc = 0;
	for (auto c : line) {
		crc += c;
	}
	return crc;
}

bool SettingsManager::read(const char* file_name) {
	std::ifstream input_file;
	input_file.open(file_name);
	if (!input_file.good()) {
		LOG(WARNING) << "Unable to open " << file_name;
		return false;
	}
	values_.clear();
	while(!input_file.eof()) {
		std::string line;
		std::getline(input_file, line);
		if (line.empty()) continue;
		size_t eq_pos = line.find('=');
		if (eq_pos == std::string::npos) {
			continue;
		}
		std::string key = line.substr(0, eq_pos);
		std::string value = line.substr(eq_pos + 1);
		if (key.empty()) continue;
		values_[key] = value;
	}
	input_file.close();
	return validate();
}

void SettingsManager::defaults() {
	values_.clear();

	// Add default setting values bellow
	values_["FirstTime"] = "1";
}

bool SettingsManager::get(const std::string& key, std::string* value) const {
	const auto& element = values_.find(key);
	if (element == values_.end()) {
		return false;
	}
	value->assign(element->second);
	return true;
}

bool SettingsManager::get(const std::string& key, int32_t* value) const {
	std::string value_str;
	if (!get(key, &value_str)) {
		return false;
	}
	*value = atoi(value_str.c_str());
	return true;
}

bool SettingsManager::get(const std::string& key, uint32_t* value) const {
	std::string value_str;
	if (!get(key, &value_str)) {
		return false;
	}
	*value = strtoul(value_str.c_str(), NULL, 10);
	return true;
}

bool SettingsManager::get(const std::string& key, bool* value) const {
	int value_int;
	if (!get(key, &value_int)) {
		return false;
	}
	*value = value_int != 0;
	return true;
}

void SettingsManager::set(const std::string& key, const char* value) {
	const auto& element = values_.find(key);
	if (element == values_.end() || element->second != value) {
		values_[key] = value;
		update();
	}
}

void SettingsManager::set(const std::string& key, const std::string& value) {
	const auto& element = values_.find(key);
	if (element == values_.end() || element->second != value) {
		values_[key] = value;
		update();
	}
}

void SettingsManager::set(const std::string& key, int32_t value) {
	std::string value_str = googleapis::StringPrintf("%d", value);
	set(key, value_str);
}

void SettingsManager::set(const std::string& key, uint32_t value) {
	std::string value_str = googleapis::StringPrintf("%u", value);
	set(key, value_str);
}

void SettingsManager::set(const std::string& key, bool value) {
	set(key, value ? 1 : 0);
}

void SettingsManager::update() {
	std::string settings_file_name = FLAGS_settings_location + "/" +
			SETTINGS_FILE_NAME;

	mkdir(FLAGS_settings_location.c_str(), 0700);

	std::string last_file_name = googleapis::StringPrintf(
			"%s.9",
			settings_file_name.c_str());
	unlink(last_file_name.c_str());

	for (int sufix = 9; sufix > 1; sufix--) {
		std::string old_file_name = googleapis::StringPrintf(
				"%s.%d",
				settings_file_name.c_str(),
				sufix - 1);
		std::string new_file_name = googleapis::StringPrintf(
				"%s.%d",
				settings_file_name.c_str(),
				sufix);
		rename(old_file_name.c_str(), new_file_name.c_str());
	}
	std::string new_file_name = googleapis::StringPrintf(
			"%s.1",
			settings_file_name.c_str());
	rename(settings_file_name.c_str(), new_file_name.c_str());
	uint32_t crc = calcCRC();
	std::string crc_str = googleapis::StringPrintf("%u", crc);
	values_[CRC_KEY] = crc_str;
	write(settings_file_name.c_str());
}

bool SettingsManager::write(const char* file_name) {
	std::ofstream output_file;
	output_file.open(file_name);
	if (!output_file.good()) {
		LOG(WARNING) << "Unable to open " << file_name;
		return false;
	}
	for (const auto& element : values_) {
		std::string line;
		formatLine(element.first, element.second, &line);
		output_file << line << std::endl;
	}
	output_file.close();
	return true;
}

} /* namepsace iqurius */
