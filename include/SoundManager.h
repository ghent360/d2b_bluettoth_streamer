/*
 * SoundManager.h
 *
 *  Created on: Apr 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef SOUNDMANAGER_H_
#define SOUNDMANAGER_H_

#include "util.h"

namespace iqurius {

class SoundManager {
public:
	enum ESoundID {
		SOUND_CORRECT,
		SOUND_INCORRECT,
		SOUND_DISABLED,
		SOUND_RESTARTING,
		SOUND_SONAR,
		SOUND_ENTER_CODE_522,
		SOUND_UNABLE_TO_CONNECT_TO_BLUETOOTH_ADAPTER,
		SOUND_UPDATE_COMPLETED,
		SOUND_NO_PHONE_CONNECTED,
		SOUND_UPDATE_COMPLETE,
		SOUND_PLEASE_DONT_TURH_THE_POWER,
		SOUND_UPDATE_IS_AVAILABLE,
		SOUND_PREPARING_THE_UPDATE,
		SOUND_UPDATING,
		SOUND_READY_TO_PAIR,
	};

	SoundManager();
	~SoundManager() {};

	const char* getSoundPath(ESoundID sound_id) const;
private:
	DISALLOW_COPY_AND_ASSIGN(SoundManager);
};

} /* namespace iqurius */

#endif /* SOUNDMANAGER_H_ */
