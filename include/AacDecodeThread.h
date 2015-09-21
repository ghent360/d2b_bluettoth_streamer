/*
 * AacDecodeThread.h
 *
 *  Created on: July 19, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef AACDECODETHREAD_H_
#define AACDECODETHREAD_H_

#include "PlaybackThread.h"

#undef IS_LITTLE_ENDIAN
#include <fdk-aac/aacdecoder_lib.h>

namespace dbus {

class AacDecodeThread : public PlaybackThread {
public:
	AacDecodeThread(Connection* connection,
			const ObjectPath& path,
			iqurius::AudioChannel* audio_channel,
			int sampling_rate);
	virtual ~AacDecodeThread();

	virtual void decode(const uint8_t* buffer, size_t size);

private:
	INT_PCM pcm_buffer_[2048 * 2];  // Max 2048 samples * 2 channels
	HANDLE_AACDECODER decoder_;

	DISALLOW_COPY_AND_ASSIGN(AacDecodeThread);
};

} /* namespace dbus */

#endif /* AACDECODETHREAD_H_ */
