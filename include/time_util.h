/*
 * time_util.h
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_

#include <stdint.h>

uint32_t timeGetTime();  // returns current time in milliseconds.
uint32_t elapsedTime(uint32_t time);

#endif /* TIME_UTIL_H_ */
