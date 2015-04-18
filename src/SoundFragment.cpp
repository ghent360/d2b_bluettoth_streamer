/*
 * SoundFragment.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include <AudioMixer.h>
#include "SoundFragment.h"
#include <glog/logging.h>
#include <sys/select.h>
#include <unistd.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace iqurius {

class MonoSoundFragment : public SoundFragment {
public:
	MonoSoundFragment(size_t num_samples);
	virtual ~MonoSoundFragment();

	virtual size_t getBufferSize() const { return num_samples_ * 2; }
	virtual uint8_t getChannels() const { return 1; }
	virtual void playFragment(AudioChannel* audio_channel);
private:
	size_t num_samples_;
	uint8_t* conversion_buffer_;
	DISALLOW_COPY_AND_ASSIGN(MonoSoundFragment);
};

class StereoSoundFragment : public SoundFragment {
public:
	StereoSoundFragment(size_t num_samples);

	virtual size_t getBufferSize() const { return num_samples_ * 4; }
	virtual uint8_t getChannels() const { return 2; }
	virtual void playFragment(AudioChannel* audio_channel);
private:
	size_t num_samples_;
	DISALLOW_COPY_AND_ASSIGN(StereoSoundFragment);
};

MonoSoundFragment::MonoSoundFragment(size_t num_samples) {
  num_samples_ = num_samples;
  sample_buffer_ = new uint8_t[num_samples * 2];
  conversion_buffer_ = new uint8_t[AudioMixer::AUDIO_BUFFER_SIZE * 4];
}

StereoSoundFragment::StereoSoundFragment(size_t num_samples) {
  num_samples_ = num_samples;
  sample_buffer_ = new uint8_t[num_samples * 4];
}

SoundFragment::~SoundFragment() {
  delete [] sample_buffer_;
}

MonoSoundFragment::~MonoSoundFragment() {
  delete [] conversion_buffer_;
}

SoundFragment* SoundFragment::fromVorbisFile(const char* path) {
  SoundFragment* result = nullptr;
  OggVorbis_File vf;
  vorbis_info *vi;
  ogg_int64_t num_samples;
  char* buffer;
  long len;
  size_t buffer_len;
  int current_section;

  if(ov_fopen(path, &vf) < 0) {
	LOG(ERROR) << "File " << path << " is not a valid Ogg bitstream.";
	return nullptr;
  }
  vi = ov_info(&vf, -1);
  CHECK(vi->rate == 44100);
  CHECK(vi->channels <= 2);
  num_samples = ov_pcm_total(&vf, -1);
  if (vi->channels == 1) {
	  result = new MonoSoundFragment(num_samples);
  } else {
	  result = new StereoSoundFragment(num_samples);
  }

  buffer = (char*)result->sample_buffer_;
  buffer_len = (int)result->getBufferSize();
  while (buffer_len > 0) {
	len = ov_read(&vf, buffer, buffer_len, 0, 2, 1, &current_section);
	if (len < 0) {
	  LOG(ERROR) << "Error decoding the stream.";
	  delete result;
	  result = nullptr;
	  goto exit;
	}
	buffer += len;
	buffer_len -= len;
  }
exit:
  ov_clear(&vf);
  return result;
}

AudioBuffer* SoundFragment::waitForFreeBuffer(AudioChannel* audio_channel) {
  AudioBuffer* audio_buffer;

  do {
	audio_buffer = audio_channel->getFreeBuffer();
	if (audio_buffer) {
		return audio_buffer;
	}
    usleep(10000);
  } while (!cancel_playback_);
  return nullptr;
}

void MonoSoundFragment::playFragment(AudioChannel* audio_channel) {
  const size_t frame_size = 2;
  const uint8_t* fragment_buffer = getBuffer();
  size_t fragment_len = getBufferSize() / frame_size;

  while (fragment_len > 0) {
    AudioBuffer* audio_buffer = waitForFreeBuffer(audio_channel);
    if (nullptr == audio_buffer) {
      break;
    }

    size_t buffer_len = audio_buffer->getSize() / 4;
    if (fragment_len < buffer_len) {
  	  buffer_len = fragment_len;
    }
    CHECK(buffer_len <= AudioMixer::AUDIO_BUFFER_SIZE / 4);
    for (int idx = 0; idx < buffer_len; ++idx) {
      conversion_buffer_[idx*4] = fragment_buffer[idx*2];
      conversion_buffer_[idx*4 + 1] = fragment_buffer[idx*2 + 1];
      conversion_buffer_[idx*4 + 2] = fragment_buffer[idx*2];
      conversion_buffer_[idx*4 + 3] = fragment_buffer[idx*2 + 1];
    }
    audio_buffer->write(conversion_buffer_, buffer_len * 4);
	audio_channel->postBuffer(audio_buffer);
    fragment_buffer += buffer_len * 2;
	fragment_len -= buffer_len;
  }
}

void StereoSoundFragment::playFragment(AudioChannel* audio_channel) {
  const size_t frame_size = 4;
  const uint8_t* fragment_buffer = getBuffer();
  size_t frafment_len = getBufferSize() / frame_size;

  while (frafment_len > 0) {
    AudioBuffer* audio_buffer = waitForFreeBuffer(audio_channel);
    if (nullptr == audio_buffer) {
      break;
    }
    size_t buffer_len = audio_buffer->getSize() / 4;
    if (frafment_len < buffer_len) {
  	  buffer_len = frafment_len;
    }
  	audio_buffer->write(fragment_buffer, buffer_len * 4);
	audio_channel->postBuffer(audio_buffer);
 	fragment_buffer += buffer_len * 4;
	frafment_len -= buffer_len;
  }
}

} /* namespace dbus */
