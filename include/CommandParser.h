/*
 * CommandParser.h
 *
 *  Created on: Feb 7, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef COMMAND_PARSER_H_
#define COMMAND_PARSER_H_

#include "util.h"

#include <googleapis/base/callback.h>
#include <string.h>

class CommandParser {
public:
	typedef googleapis::Callback1<const char*> OnCommandCallback;

	CommandParser(const std::string& file_name);
	~CommandParser();

	bool open();
	void close();
	bool isOpen() { return file_handle_ >= 0; }
	void process();
	void setCommandCllaback(OnCommandCallback* cb);
	void sendStatus(const char* status);

private:
	enum EParserStatus {
	    IDLE,
	    SIGN_1,
	    CMD,
	};

	static constexpr char CMD_PREFIX_1 = '@';
	static constexpr char CMD_PREFIX_2 = '&';
	static constexpr char CR = '\r';
	static constexpr char LF = '\n';
	static constexpr int  MAX_CMD_LEN = 10;

	void nextChar(char next_char);

	std::string file_name_;
	int file_handle_;
	EParserStatus parser_status_;
	int cmd_len_;
	char cmd_[MAX_CMD_LEN];
	char buffer_[MAX_CMD_LEN];
	OnCommandCallback* command_cb_;

	DISALLOW_COPY_AND_ASSIGN(CommandParser);
};

#endif /* COMMAND_PARSER_H_ */
