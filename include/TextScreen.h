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

	void tick();
private:
	bool is_open() const { return fd_ >= 0; }

	int fd_;
	std::string port_name_;
	googleapis::Closure* error_cb_;
	DISALLOW_COPY_AND_ASSIGN(TextScreen);
};

} /* namespace iqurius */

#endif /* TEXTSCREEN_H_ */
