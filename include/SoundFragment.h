/*
 * SoundFragment.h
 *
 *  Created on: Apr 17, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef SOUNDFRAGMENT_H_
#define SOUNDFRAGMENT_H_

#include "util.h"

#include <glog/logging.h>

namespace iqurius {

class AudioChannel;
class AudioBuffer;
class SoundFragment {
public:
	virtual ~SoundFragment();

	virtual const uint8_t* getBuffer() const { return sample_buffer_; }
	virtual size_t getBufferSize() const = 0;
	virtual uint8_t getChannels() const = 0;
	virtual void playFragment(AudioChannel* audio_channel) = 0;
	void cancelPlayback() { cancel_playback_ = true; }

	static SoundFragment* fromVorbisFile(const char* path);

protected:
	SoundFragment() : sample_buffer_(nullptr), cancel_playback_(false) {}
	AudioBuffer* waitForFreeBuffer(AudioChannel* audio_channel);

	uint8_t* sample_buffer_;
	bool cancel_playback_;
private:
	DISALLOW_COPY_AND_ASSIGN(SoundFragment);
};

} /* namespace iqurius */

#endif /* SOUNDFRAGMENT_H_ */
