/*
 * time_util.cpp
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "time_util.h"

#include <time.h>

uint32_t timeGetTime() {
	struct timespec time;

	if (clock_gettime(CLOCK_MONOTONIC_COARSE, &time) < 0) {
		if (clock_gettime(CLOCK_BOOTTIME, &time) < 0) {
			clock_gettime(CLOCK_REALTIME, &time);
		}
	}
	return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

uint32_t elapsedTime(uint32_t time) {
	return timeGetTime() - time;
}
