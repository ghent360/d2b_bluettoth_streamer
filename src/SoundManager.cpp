/*
 * SoundManager.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "SoundManager.h"

#include <glog/logging.h>
#include <map>

namespace iqurius {

static const std::map<SoundManager::ESoundID, std::string> g_SoundPaths = {
		{ SoundManager::SOUND_CORRECT, DATADIR "/Correct.ogg" },
		{ SoundManager::SOUND_INCORRECT, DATADIR "/Incorrect.ogg" },
		{ SoundManager::SOUND_DISABLED, DATADIR "/Disabled.ogg" },
		{ SoundManager::SOUND_RESTARTING, DATADIR "/Restarting.ogg" },
		{ SoundManager::SOUND_SONAR, DATADIR "/Sonar.ogg" },
		{ SoundManager::SOUND_ENTER_CODE_522, DATADIR "/Enter_code_522.ogg" },
		{ SoundManager::SOUND_UNABLE_TO_CONNECT_TO_BLUETOOTH_ADAPTER,
				DATADIR "/Unable_to_connect_to_bluetooth_adapter.ogg" },
		{ SoundManager::SOUND_UPDATE_COMPLETED,
				DATADIR "/Update_completed.ogg" },
		{ SoundManager::SOUND_NO_PHONE_CONNECTED,
				DATADIR "/No_phone_connected.ogg" },
		{ SoundManager::SOUND_UPDATE_COMPLETE,
				DATADIR "/Update_complete.ogg" },
		{ SoundManager::SOUND_PLEASE_DONT_TURH_THE_POWER,
				DATADIR "/Please_dont_turh_the_power.ogg" },
		{ SoundManager::SOUND_UPDATE_IS_AVAILABLE,
				DATADIR "/Update_is_available.ogg" },
		{ SoundManager::SOUND_PREPARING_THE_UPDATE,
				DATADIR "/Preparing_the_update.ogg" },
		{ SoundManager::SOUND_UPDATING, DATADIR "/Updating2.ogg" },
		{ SoundManager::SOUND_READY_TO_PAIR, DATADIR "/Ready_to_pair1.ogg" }
};

SoundManager::SoundManager() {
}

const char* SoundManager::getSoundPath(ESoundID sound_id) const {
	const auto element = g_SoundPaths.find(sound_id);
	if (element == g_SoundPaths.end()) {
		return nullptr;
	}
	return element->second.c_str();
}
} /* namespace dbus */
