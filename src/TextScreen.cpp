/*
 * TextScreen.cpp
 *
 *  Created on: Jun 27, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "TextScreen.h"
#include "time_util.h"

#include <glog/logging.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace iqurius {

TextScreen::TextScreen(googleapis::Closure* error_cb)
    : fd_(-1),
	  error_cb_(error_cb) {
  CHECK(!error_cb || error_cb->IsRepeatable()) <<
		"Error callback should be repeatable";
  //artist_ = "System of a Down";
  //album_ = "Toxicity";
  //track_ = "Psycho";
  switch_state_ = EARTIST;
  top_start_ = 0;
  track_no_ = -1;
  play_time_ = -1;
  last_switch_time_ = 0;
  switch_time_ = MIN_SWITCH_TIME;
}

TextScreen::~TextScreen() {
  close();
  delete error_cb_;
}

void TextScreen::close() {
  if (fd_ >= 0) {
	::close(fd_);
	fd_ = -1;
  }
}

void TextScreen::open(const char* port_name) {
  close();
  port_name_ = port_name;
  fd_ = ::open(port_name_.c_str(), O_RDWR);
  if (fd_ < 0) {
	LOG(ERROR) << "Error opening " << port_name_ << " errno=" << errno;
	if (error_cb_) {
	  error_cb_->Run();
	}
  } else {
	last_switch_time_ = timeGetTime() - switch_time_ - 1;
  }
}

TextScreen::ESwitchState TextScreen::nextSwitchState(
		TextScreen::ESwitchState state) {
  switch (state) {
	case EARTIST: return EALBUM;
	case EALBUM: return ETRACK;
	case ETRACK: return EARTIST;
  }
  return EARTIST;
}

void TextScreen::switchTopLine() {
  top_line_ = "No informaton";
  switch (switch_state_) {
	case EARTIST:
	  if (!artist_.empty()) {
	    top_line_ = "Artist: ";
	    top_line_ += artist_;
  	    break;
	  }
	case EALBUM:
	  if (!album_.empty()) {
	    top_line_ = "Album: ";
	    top_line_ += album_;
	    switch_state_ = EALBUM;
	    break;
	  }
	case ETRACK:
	  if (!title_.empty()) {
	    top_line_ = "Title: ";
	    top_line_ += title_;
	    switch_state_ = ETRACK;
	  } else if (!artist_.empty()) {
	    top_line_ = "Artist: ";
	    top_line_ += artist_;
	    switch_state_ = EARTIST;
	  } else if (!album_.empty()) {
	    top_line_ = "Album: ";
	    top_line_ += album_;
	    switch_state_ = EALBUM;
	  }
	  break;
  }
  top_start_ = 0;
  switch_time_ = MIN_SWITCH_TIME;
  if (top_line_.length() > 16) {
	top_line_.append("    ");
	switch_time_ += 2000 * (top_line_.length() - 16);
  }
}

void TextScreen::tick() {
  if (is_open()) {
	char bfr[20];
	int idx;
	int result;

	if (elapsedTime(last_switch_time_) > switch_time_) {
	  last_switch_time_ = timeGetTime();
	  switch_state_ = nextSwitchState(switch_state_);
	  switchTopLine();
	}
    size_t top_len = top_line_.length();

	bfr[0] = '\f';
    for (idx = 0; idx < 16 && idx < top_len; idx++) {
      bfr[idx + 1] = top_line_.at((top_start_ + idx) % top_len);
    }
    idx++;
    if (idx < 17) {
      bfr[idx++] = 9;
    }
    bfr[idx++] = '\n';
    result = write(fd_, bfr, idx);
    if (result < 0) {
      int err = errno;
      LOG(ERROR) << "Error writing to text screen errno=" << err;
      if (err == ENOTCONN || err == EIO) {
   		if (error_cb_) {
   		  error_cb_->Run();
   		}
    	close();
    	return;
      }
    }

    if (track_no_ > 0) {
      snprintf(bfr, sizeof(bfr) - 1, "Track %-4d ", track_no_ % 10000);
    } else if (play_time_ >= 0) {
      strcpy(bfr, "Play time  ");
    } else {
      bfr[0] = 9;
      bfr[1] = 0;
    }
    if (play_time_ >= 0) {
      int play_time_min = (play_time_ / 60) % 60;
      int play_time_sec = play_time_ % 60;
      snprintf(bfr + 11, sizeof(bfr) - 12, "%02d:%02d",
  			play_time_min,
      		play_time_sec);
    }
    result = write(fd_, bfr, strlen(bfr));
    if (result < 0) {
      int err = errno;
      LOG(ERROR) << "Error writing to text screen errno=" << err;
      if (err == ENOTCONN || err == EIO) {
   		if (error_cb_) {
   		  error_cb_->Run();
   		}
      	close();
      	return;
      }
    }

    if (top_len > 16) {
	  top_start_++;
	  top_start_ %= top_len;
    }
  }
}
} /* namespace iqurius */
