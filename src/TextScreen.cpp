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

#include <glog/logging.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace iqurius {

TextScreen::TextScreen(const char* port_name,
		googleapis::Closure* error_cb)
    : fd_(-1),
	  port_name_(port_name),
	  error_cb_(error_cb) {
  CHECK(error_cb && !error_cb->IsRepeatable()) <<
		"Error callback can not be repeatable";
  open();
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

void TextScreen::open() {
  close();
  fd_ = ::open(port_name_.c_str(), O_RDWR);
  if (fd_ < 0) {
	LOG(ERROR) << "Error opening " << port_name_ << " errno=" << errno;
	if (error_cb_) {
	  error_cb_->Run();
	  error_cb_ = NULL;
	}
  }
}

} /* namespace iqurius */
