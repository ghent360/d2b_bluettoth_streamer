/*
 * DelayedProcessing.h
 *
 *  Created on: Apr 12, 2015
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2015
 * All rights reserved.
 */

#ifndef DELAYEDPROCESING_H_
#define DELAYEDPROCESING_H_

#include "util.h"

#include <googleapis/base/callback.h>

namespace iqurius {

uint32_t PostDelayedCallback(uint32_t delay_ms, googleapis::Closure* callback);
uint32_t PostTimerCallback(uint32_t delay_ms, googleapis::Closure* callback);
bool RemoveTimerCallback(uint32_t token);

void ProcessDelayedCalls();

} /* namespace iqurius */

#endif /* DELAYEDPROCESING_H_ */
