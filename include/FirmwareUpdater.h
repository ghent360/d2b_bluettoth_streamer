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

class FirmwareContainerReader;
class FirmwareUpdater {
public:
	FirmwareUpdater() : update_(nullptr) {}
	~FirmwareUpdater();
	bool checkUpdateAvailable();
	bool updateValid();
	bool update();
	void SyncDisc();
private:
	bool remountFlash(bool read_only);

	FirmwareContainerReader* update_;
	DISALLOW_COPY_AND_ASSIGN(FirmwareUpdater);
};

} /* namespace iqurius */

#endif /* FIRMWAREUPDATER_H_ */
