/*
 * CommandParser.cpp
 *
 *  Created on: Feb 7, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */
#include "CommandParser.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <glog/logging.h>

CommandParser::CommandParser(const std::string& file_name)
	: file_name_(file_name),
	  file_handle_(-1),
	  parser_status_(IDLE),
	  cmd_len_(0),
	  command_cb_(NULL) {
}

CommandParser::~CommandParser() {
	close();
	delete command_cb_;
}

bool CommandParser::open() {
	close();
	if (!file_name_.empty()) {
		file_handle_ = ::open(file_name_.c_str(),
				O_RDWR | O_NOATIME | O_NOCTTY);
		parser_status_ = IDLE;
	}
	return isOpen();
}

void CommandParser::close() {
	if (file_handle_ >= 0) {
		::close(file_handle_);
		file_handle_ = -1;
	}
}

void CommandParser::setCommandCllaback(OnCommandCallback* cb) {
	delete command_cb_;
	command_cb_ = cb;
}

void CommandParser::sendStatus(const char* status) {
	if (!isOpen()) {
		if (!open()) return;
	}
	int written = write(file_handle_, status, strlen(status));
	if (written < 0) {
		LOG(WARNING) << "Error writing command status " << status
				<< " errno=" << errno;
	}
}

void CommandParser::process() {
	if (!isOpen()) {
		if (!open()) return;
	}

	fd_set read_fds;
	struct timeval timeout;

	FD_ZERO(&read_fds);
	FD_SET(file_handle_, &read_fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int select_result = select(file_handle_ + 1, &read_fds, NULL, NULL, &timeout);
	if (select_result < 0 && errno == EBADF) {
		LOG(WARNING) << "Error during select, re-opening command descriptor.";
		open();
		return;
	}
	if (select_result > 0) {
		int len = read(file_handle_, buffer_, sizeof(buffer_));
		if (len == 0 || (len < 0 && errno == EBADF)) {
			LOG(WARNING) << "Error reading, re-opening command descriptor.";
			open();
		}
		for (int idx = 0; idx < len; idx++) {
			nextChar(buffer_[idx]);
		}
	}
}

void CommandParser::nextChar(char next_char)
{
    switch (parser_status_)
    {
        case IDLE:
            if (next_char == CMD_PREFIX_1)
            	parser_status_ = SIGN_1;
            break;
        case SIGN_1:
            if (next_char == CMD_PREFIX_2) {
            	parser_status_ = CMD;
                cmd_len_ = 0;
            } else {
            	parser_status_ = IDLE;
            }
            break;
        case CMD:
            if (next_char == CR || next_char == LF) {
                cmd_[cmd_len_] = 0;
                if (command_cb_) {
                	bool once = !command_cb_->IsRepeatable();
                	command_cb_->Run(cmd_);
                	if (once) {
                		command_cb_ = NULL;
                	}
                } else {
                	LOG(INFO) << "Unhandled command: " << cmd_;
                }
                parser_status_ = IDLE;
            } else {
                if (cmd_len_ < MAX_CMD_LEN)
                    cmd_[cmd_len_++] = next_char;
                else
                	parser_status_ = IDLE;
            }
            break;
    }
}
