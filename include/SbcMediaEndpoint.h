/*
 * SbcMediaEndpoint.h
 *
 *  Created on: Nov 23, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef SBCMEDIAENDPOINT_H_
#define SBCMEDIAENDPOINT_H_

#include "MediaEndpoint.h"

#include "a2dp-codecs.h"

namespace dbus {

class SbcMediaEndpoint : public MediaEndpoint {
public:
	SbcMediaEndpoint();
	SbcMediaEndpoint(const ObjectPath& path)
	    : MediaEndpoint(path),
		  sampling_rate_(44100) {};

	virtual const char* getUuid() const {
		return A2DP_SINK_UUID;
	}

	virtual uint8_t getCodecId() const {
		return A2DP_CODEC_SBC;
	}

	virtual bool getCapabilities(uint8_t* capabilities,
			size_t* capabilities_max_len) const;
	virtual int getSamplingRate() const {
		return sampling_rate_;
	}
protected:
	virtual bool selectConfiguration(void* capabilities,
			size_t capabilities_len,
			uint8_t** selected_capabilities,
			size_t* selected_capabilities_len);
	virtual void setConfiguration(const ObjectPath& transport,
			const MediaTransportProperties& properties);

private:
	static const a2dp_sbc_t CAPABILITIES;
	int sampling_rate_;
	DISALLOW_COPY_AND_ASSIGN(SbcMediaEndpoint);
};

} /* namespace dbus */

#endif /* SBCMEDIAENDPOINT_H_ */
