/*
 * SbcDecodeThread.h
 *
 *  Created on: Nov 26, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef SBCDECODETHREAD_H_
#define SBCDECODETHREAD_H_

#include "PlaybackThread.h"
#include "sbc.h"

namespace dbus {

class SbcDecodeThread : public PlaybackThread {
public:
	SbcDecodeThread(Connection* connection,
			const ObjectPath& path,
			iqurius::AudioChannel* audio_channel,
			int sampling_rate);
	virtual ~SbcDecodeThread();

	virtual void decode(const uint8_t* buffer, size_t size);

private:
	sbc_t codec_;
	uint8_t pcm_buffer_[8192];

	DISALLOW_COPY_AND_ASSIGN(SbcDecodeThread);
};

} /* namespace dbus */

#endif /* SBCDECODETHREAD_H_ */
