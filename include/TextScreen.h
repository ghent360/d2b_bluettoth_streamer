/*
 * TextScreen.h
 *
 *  Created on: Jun 27, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef TEXTSCREEN_H_
#define TEXTSCREEN_H_

#include "util.h"

#include <googleapis/base/callback.h>
#include <string>

namespace iqurius {

class TextScreen {
public:
	TextScreen(googleapis::Closure* error_cb = NULL);
	~TextScreen();

	void open(const char* port_name);
	void close();

	void setTrackNo(int track_no) {
	  track_no_ = track_no;
	}

	void setPlayTime(int play_time) {
	  play_time_ = play_time;
	}

	void setArtist(const char* artist) {
	  artist_ = artist;
	}

	void setAlbum(const char* album) {
	  album_ = album;
	}

	void setTitle(const char* title) {
	  title_ = title;
	}

	void tick();
private:
	enum ESwitchState {
	  EALBUM,
	  EARTIST,
	  ETRACK
	};
	bool is_open() const { return fd_ >= 0; }
	static ESwitchState nextSwitchState(ESwitchState state);
	void switchTopLine();

	constexpr static uint32_t MIN_SWITCH_TIME = 3500;

	int fd_;
	int top_start_;
	int track_no_;
	int play_time_;
	std::string artist_;
	std::string album_;
	std::string title_;
	std::string top_line_;
	std::string port_name_;
	uint32_t last_switch_time_;
	uint32_t switch_time_;
	ESwitchState switch_state_;
	googleapis::Closure* error_cb_;
	DISALLOW_COPY_AND_ASSIGN(TextScreen);
};

} /* namespace iqurius */

#endif /* TEXTSCREEN_H_ */
