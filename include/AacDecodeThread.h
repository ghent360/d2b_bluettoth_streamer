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

namespace dbus {

class AacDecodeThread : public PlaybackThread {
public:
	AacDecodeThread(Connection* connection,
			const ObjectPath& path,
			iqurius::AudioChannel* audio_channel);
	virtual ~AacDecodeThread();

	virtual void decode(const uint8_t* buffer, size_t size);

private:
	uint8_t pcm_buffer_[8192];

	DISALLOW_COPY_AND_ASSIGN(AacDecodeThread);
};

} /* namespace dbus */

#endif /* AACDECODETHREAD_H_ */
