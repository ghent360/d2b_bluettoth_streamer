/*
 * DelayedProcessing.cpp
 *
 *  Created on: Apr 12, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#include "DelayedProcessing.h"
#include "time_util.h"

#include <glog/logging.h>
#include <list>

namespace iqurius {

static uint32_t g_TokenGenerator = 1;

class DelayedProcessingEntity {
public:
	DelayedProcessingEntity(uint32_t period, googleapis::Closure* callback, bool repeat)
		: period_(0),
		  callback_(callback),
		  token_(g_TokenGenerator++) {
		CHECK(callback != NULL);
		next_exec_time_ = timeGetTime() + period;
		if (repeat) {
			period_ = period;
			CHECK(callback->IsRepeatable());
		} else {
			CHECK(!callback->IsRepeatable());
		}
	}
	~DelayedProcessingEntity() {
		delete callback_;
	}

	bool callCheck(uint32_t time) {
		if (time > next_exec_time_) {
			callback_->Run();
			if (period_ > 0) {
				next_exec_time_ = time + period_;
			} else {
				callback_ = NULL;
				return true;
			}
		}
		return false;
	}

	uint32_t token() const { return token_; }
private:
	uint32_t period_;
	uint32_t next_exec_time_;
	googleapis::Closure* callback_;
	uint32_t token_;

	DISALLOW_COPY_AND_ASSIGN(DelayedProcessingEntity);
};

static std::list<DelayedProcessingEntity*> g_DelayedProcs;

void ProcessDelayedCalls() {
	uint32_t time_now = timeGetTime();
	static std::list<uint32_t> cleanup_list;

	for (auto* proc : g_DelayedProcs) {
		if (proc->callCheck(time_now)) {
			cleanup_list.push_back(proc->token());
		}
	}
	for (uint32_t token : cleanup_list) {
		RemoveTimerCallback(token);
	}
}

bool RemoveTimerCallback(uint32_t token) {
	for (std::list<DelayedProcessingEntity*>::iterator it = g_DelayedProcs.begin();
			it != g_DelayedProcs.end(); ++it) {
		if ((*it)->token() == token) {
			delete (*it);
			g_DelayedProcs.erase(it);
			return true;
		}
	}
	return false;
}

uint32_t PostDelayedCallback(uint32_t delay_ms, googleapis::Closure* callback) {
	DelayedProcessingEntity* proc = new DelayedProcessingEntity(delay_ms, callback, false);
	if (proc == NULL) {
		LOG(ERROR) << "Out of memory";
		return 0;
	}
	g_DelayedProcs.push_back(proc);
	return proc->token();
}

uint32_t PostTimerCallback(uint32_t delay_ms, googleapis::Closure* callback) {
	DelayedProcessingEntity* proc = new DelayedProcessingEntity(delay_ms, callback, true);
	if (proc == NULL) {
		LOG(ERROR) << "Out of memory";
		return 0;
	}
	g_DelayedProcs.push_back(proc);
	return proc->token();
}
} /* namespace iqurius */
