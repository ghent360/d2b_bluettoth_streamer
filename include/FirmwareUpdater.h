/*
 * FirmwareUpdater.h
 *
 *  Created on: Apr 5, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef FIRMWAREUPDATER_H_
#define FIRMWAREUPDATER_H_

#include "util.h"

#include <string>

namespace iqurius {

class FirmwareUpdater {
public:
	FirmwareUpdater() {}
	bool checkUpdateAvailable();
private:
	bool remountFlash(bool read_only);
	void sync();

	std::string update_file_name_;
	DISALLOW_COPY_AND_ASSIGN(FirmwareUpdater);
};

} /* namespace iqurius */

#endif /* FIRMWAREUPDATER_H_ */
