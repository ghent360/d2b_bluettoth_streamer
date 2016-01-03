/*
 * PlaybackThread.h
 *
 *  Created on: Nov 19, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef PLAYBACKTHREAD_H_
#define PLAYBACKTHREAD_H_

#include "MediaTransport.h"
#include "util.h"

#include <soxr.h>
#include <pthread.h>

namespace iqurius {
class AudioChannel;
class AudioBuffer;
}

namespace dbus {

class ObjectPath;
class PlaybackThread {
public:
  enum ECodecID {
    E_SBC,
	E_AAC
  };

  PlaybackThread(Connection* connection,
		  const ObjectPath&,
		  iqurius::AudioChannel* audio_channel,
		  int sampling_rate);
  virtual ~PlaybackThread() {
    stop();
    delete [] audo_channel_buffer_;
    if (resampler_) {
    	soxr_delete(resampler_);
    }
  };

  virtual void decode(const uint8_t* buffer, size_t size) = 0;
  virtual ECodecID codecId() const = 0;
  void playPcm(const uint8_t* buffer, size_t size);

  void start();
  void stop();

  bool ok() const { return running_ && decoding_ok_; }

private:
  iqurius::AudioBuffer* waitForFreeBuffer();
  void freeTransport();
  bool acqureTransport();

  bool running_;
  bool signal_stop_;
  bool decoding_ok_;
  MediaTransport transport_;
  pthread_t thread_;

  int fd_;
  int read_mtu_;
  int write_mtu_;
  int sampling_rate_;
  iqurius::AudioChannel* audio_channel_;
  uint8_t* audo_channel_buffer_;
  size_t audo_buffer_len_;
  soxr_t resampler_;

  static void* threadProc(void *);
  void run();

  DISALLOW_COPY_AND_ASSIGN(PlaybackThread);
};

} /* namespace dbus */

#endif /* PLAYBACKTHREAD_H_ */
