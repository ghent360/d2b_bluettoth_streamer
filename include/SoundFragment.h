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
class SoundFragment {
public:
	~SoundFragment();

	const uint8_t* getBuffer() const { return samples_; }
	size_t getBufferSize() const { return num_samples_ * channels_ * 2; }
	uint8_t getChannels() const { return channels_; }

	void playFragment(AudioChannel* audio_channel);

	static SoundFragment* fromVorbisFile(const char* path);
private:
	SoundFragment(size_t num_samples, uint8_t num_channels);

	size_t num_samples_;
	uint8_t* samples_;
	uint8_t channels_;
	uint8_t* conversion_buffer_;
	DISALLOW_COPY_AND_ASSIGN(SoundFragment);
};

} /* namespace iqurius */

#endif /* SOUNDFRAGMENT_H_ */
