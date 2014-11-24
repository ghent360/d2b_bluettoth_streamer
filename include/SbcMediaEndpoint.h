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
	SbcMediaEndpoint(const ObjectPath& path) : MediaEndpoint(path) {};

	virtual const char* getUuid() const {
		return A2DP_SINK_UUID;
	}

	virtual uint8_t getCodecId() const {
		return A2DP_CODEC_SBC;
	}

	virtual bool getCapabilities(uint8_t* capabilities,
			size_t* capabilities_max_len) const;
protected:
	virtual bool selectConfiguration(void* capabilities,
			size_t capabilities_len,
			uint8_t** selected_capabilities,
			size_t* selected_capabilities_len);

private:
	static const a2dp_sbc_t CAPABILITIES;
	DISALLOW_COPY_AND_ASSIGN(SbcMediaEndpoint);
};

} /* namespace dbus */

#endif /* SBCMEDIAENDPOINT_H_ */
