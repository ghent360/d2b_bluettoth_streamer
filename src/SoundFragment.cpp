/*
 * SoundFragment.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#include "SoundFragment.h"

#include <glog/logging.h>
#include <sys/select.h>
#include <unistd.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace iqurius {

SoundFragment::SoundFragment(size_t num_samples, uint8_t num_channels) {
  num_samples_ = num_samples;
  samples_ = new uint8_t[num_samples * num_channels * 2];
  channels_ = num_channels;
}

SoundFragment::~SoundFragment() {
  delete [] samples_;
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
  result = new SoundFragment(num_samples, vi->channels);
  buffer = (char*)result->samples_;
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

void SoundFragment::playFragment(AudioChannel* audio_channel) {
  const size_t frame_size = getChannels() * 2;
  const uint8_t* fragment_buffer = getBuffer();
  size_t buffer_len_frames = getBufferSize() / frame_size;
  uint8_t* conversion_buffer = new uint8_t[MixerThread::AUDIO_BUFFER_SIZE * 4];

  while (buffer_len_frames > 0) {
    iqurius::AudioBuffer* audio_buffer = audio_channel->getFreeBuffer();
    if (nullptr == audio_buffer) {
  	  sleep(0);
   	  continue;
    }
    size_t buffer_size_frames = audio_buffer->getSize() / 4;

    if (buffer_len_frames < buffer_size_frames) {
  	  buffer_size_frames = buffer_len_frames;
    }
    if (channels_ == 2) {
  	  audio_buffer->write(fragment_buffer, buffer_size_frames * 4);
   	  fragment_buffer += buffer_size_frames * 4;
    } else {
   	  CHECK(buffer_size_frames <= MixerThread::AUDIO_BUFFER_SIZE);
      for (int idx = 0; idx < buffer_size_frames; ++idx) {
       	conversion_buffer[idx*4] = fragment_buffer[idx*2];
        conversion_buffer[idx*4 + 1] = fragment_buffer[idx*2 + 1];
        conversion_buffer[idx*4 + 2] = fragment_buffer[idx*2];
        conversion_buffer[idx*4 + 3] = fragment_buffer[idx*2 + 1];
      }
      audio_buffer->write(conversion_buffer, buffer_size_frames * 4);
      fragment_buffer += buffer_size_frames * 2;
    }
	buffer_len_frames -= buffer_size_frames;
	audio_channel->postBuffer(audio_buffer);
  }
}

} /* namespace dbus */
