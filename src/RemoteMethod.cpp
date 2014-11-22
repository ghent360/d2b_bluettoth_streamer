/*
 * RemoteMethod.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include <RemoteMethod.h>

namespace dbus {

RemoteMethod::~RemoteMethod() {

}

bool RemoteMethod::prepareCall() {
  return Message::forMethodCall(*this, &call_);
}

} /* namespace dbus */
