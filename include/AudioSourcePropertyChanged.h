/*
 * AudioSourcePropertyChanged.h
 *
 *  Created on: Sep 1, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#ifndef AUDIOSOURCEPROPERTYCHANGED_H_
#define AUDIOSOURCEPROPERTYCHANGED_H_

#include "MethodLocator.h"

namespace dbus {

class AudioSourcePropertyChanged : public MethodLocator {
public:
	AudioSourcePropertyChanged();

	virtual Message handle(Message&, void* ctx);

private:
};

} /* namespace dbus */

#endif /* AUDIOSOURCEPROPERTYCHANGED_H_ */
