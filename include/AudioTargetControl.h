/*
 * AudioTargetControl.h
 *
 *  Created on: Jan 18, 2015
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2015
 *  All rights reserved.
 */

#ifndef AUDIOTARGETCONTROL_H_
#define AUDIOTARGETCONTROL_H_

#include "InterfaceImplementation.h"
#include "ObjectBase.h"
#include "ObjectPath.h"
#include "StringWithHash.h"
#include "util.h"

#include <googleapis/base/callback.h>

namespace dbus {

class Connection;
class AudioTargetControl : public SimpleObjectBase {
public:
	typedef googleapis::Callback2<bool, AudioTargetControl*> OnConnectedChangeCallback;

	enum EButtonID {
		BUTTON_ID_PLAY         = 0x44,
		BUTTON_ID_STOP         = 0x45,
		BUTTON_ID_PAUSE        = 0x46,
		BUTTON_ID_NEXT         = 0x4B,
		BUTTON_ID_PREV         = 0x4C,
		BUTTON_ID_REWIND       = 0x48,
		BUTTON_ID_FAST_FORWARD = 0x49
	};

	enum EPlayerStatus {
		STATUS_STOPPED         = 0,
		STATUS_PLAYING         = 1,
		STATUS_PAUSED          = 2,
		STATUS_FFORWARD        = 3,
		STATUS_REWIND          = 4,
		STATUS_ERROR           = 0xFF,
	};

	AudioTargetControl(Connection* connection, const ObjectPath& path);
	virtual ~AudioTargetControl();

	void setOnStateChangeCallback(OnConnectedChangeCallback* cb) {
	   		delete on_state_change_cb_;
	   		on_state_change_cb_ = cb;
	}

	bool getConnected() const {
		return state_;
	}

	void refreshProperties();
	void getEventCapabilities();
	void volumeUp();
	void sendButton(uint8_t button_id);
	void volumeDown();
	void updatePlayStatus();
	void updateMetadata();

	uint32_t getSongPos() const { return song_pos_; }
	uint32_t getSongLen() const { return song_len_; }
	EPlayerStatus getStatus() const { return status_; }

protected:
	virtual void onStateChanged(bool new_state);

	bool state_;
private:
	static void handle_stateChanged(bool new_state, ObjectBase* ctx);

	Connection* connection_;
	OnConnectedChangeCallback* on_state_change_cb_;
	uint32_t song_pos_;
	uint32_t song_len_;
	EPlayerStatus status_;

	// DBus metadata
	static const StringWithHash INTERFACE;
	static const StringWithHash GETPROPERTIES_METHOD;
	static const StringWithHash VOLUME_UP_METHOD;
	static const StringWithHash SEND_BUTTON_METHOD;
	static const StringWithHash VOLUME_DOWN_METHOD;
	static const StringWithHash GET_EVENT_CAPABILITIES_METHOD;
	static const StringWithHash GET_PLAY_STATUS_METHOD;
	static const StringWithHash GET_METADATA_METHOD;

	static const StringWithHash PROPERTYCHANGED_SIGNAL;
	static const StringWithHash CONNECTED_PROPERTY;

	static const MethodDescriptor interfaceMethods_[];
	static const MethodDescriptor interfaceSignals_[];
	static const PropertyDescriptor interfaceProperties_[];
	static const InterfaceImplementation implementation_;

	DISALLOW_COPY_AND_ASSIGN(AudioTargetControl);
};

} /* namespace dbus */

#endif /* AUDIOTARGETCONTROL_H_ */
