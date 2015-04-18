/*
 * main.cc
 *
 *  Created on: Aug 30, 2014
 *      Author: Venelin Efremov
 *
 * Copyright (C) Venelin Efremov 2014, 2015
 * All rights reserved.
 */


#include "AudioMixer.h"
#include "SoundFragment.h"

#include <glog/logging.h>
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <stdint.h>

struct ctx {
	iqurius::SoundFragment* sf;
	iqurius::AudioChannel* ac;
	int num_loops;
	int sleep;
};

void *playFragment(void* arg) {
	ctx* p_ctx = (ctx*)arg;
	for (int i = 0; i < p_ctx->num_loops; ++i) {
		p_ctx->sf->playFragment(p_ctx->ac);
		sleep(p_ctx->sleep);
	}
    return NULL;
}

void testAudioMix() {
	pthread_t t1;
	pthread_t t2;
	pthread_t t3;

	iqurius::SoundFragment* sf1 = iqurius::SoundFragment::fromVorbisFile(
			DATADIR "/Updating2.ogg");
	iqurius::SoundFragment* sf2 = iqurius::SoundFragment::fromVorbisFile(
			DATADIR "/Please_dont_turh_the_power.ogg");
	iqurius::SoundFragment* sf3 = iqurius::SoundFragment::fromVorbisFile(
			DATADIR "/Update_is_available.ogg");

	iqurius::AudioMixer mt(3);
	//mt.getAudioChannel(0)->setVolume(0.3f);
	//mt.getAudioChannel(1)->setVolume(0.3f);
	//mt.getAudioChannel(2)->setVolume(0.3f);
	mt.start();
	usleep(10000);

	ctx c1;
	ctx c2;
	ctx c3;

	c1.sf = sf1;
	c2.sf = sf2;
	c3.sf = sf3;
	c1.ac = mt.getAudioChannel(0);
	c2.ac = mt.getAudioChannel(1);
	c3.ac = mt.getAudioChannel(2);
	c1.num_loops = 3;
	c2.num_loops = 3;
	c3.num_loops = 5;
	c1.sleep = 2;
	c2.sleep = 5;
	c3.sleep = 3;

	pthread_create(&t1, NULL, playFragment, &c1);
	pthread_create(&t2, NULL, playFragment, &c2);
	pthread_create(&t3, NULL, playFragment, &c3);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	//sleep(5);
	mt.stop();

	delete sf1;
	delete sf2;
	delete sf3;
}

int main(int argc, char *argv[]) {
	gflags::SetUsageMessage("Bluetooth audio streaming daemon");
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Starting audio daemon";
	dbus_threads_init_default();
	if (lzo_init() != LZO_E_OK) {
		LOG(ERROR) << "Error initializing the LZO library";
		return 1;
	}
	testAudioMix();
	LOG(INFO) << "Exiting audio daemon";
	return 0;
}
