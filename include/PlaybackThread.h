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

#include <pthread.h>

namespace iqurius {
class AudioChannel;
class AudioBuffer;
}

namespace dbus {

class ObjectPath;
class PlaybackThread {
public:
  PlaybackThread(Connection* connection,
		  const ObjectPath&,
		  iqurius::AudioChannel* audio_channel);
  virtual ~PlaybackThread() {
    stop();
    delete [] audo_leftover_buffer_;
  };

  virtual void decode(const uint8_t* buffer, size_t size) = 0;
  void playPcm(const uint8_t* buffer, size_t size);

  void start();
  void stop();

private:
  iqurius::AudioBuffer* waitForFreeBuffer();

  bool running_;
  bool signal_stop_;
  MediaTransport transport_;
  pthread_t thread_;

  int fd_;
  int read_mtu_;
  int write_mtu_;
  iqurius::AudioChannel* audio_channel_;
  uint8_t* audo_leftover_buffer_;
  size_t audo_leftover_len_;

  static void* threadProc(void *);
  void run();

  DISALLOW_COPY_AND_ASSIGN(PlaybackThread);
};

} /* namespace dbus */

#endif /* PLAYBACKTHREAD_H_ */
