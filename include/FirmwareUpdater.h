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

namespace iqurius {

class FirmwareUpdater {
public:
	FirmwareUpdater() {}
	bool checkUpdateAvailable();
	bool virifyUpdateChecksum();
	bool remountFlash(bool read_only);
	void sync();
	bool checksumFile(const char* file_path, const char* checksum);
private:
	DISALLOW_COPY_AND_ASSIGN(FirmwareUpdater);
};

} /* namespace iqurius */

#endif /* FIRMWAREUPDATER_H_ */
