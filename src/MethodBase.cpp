/*
 * Method.cpp
 *
 *  Created on: Aug 31, 2014
 *      Author: Venelin Efremov
 *
 *  Copyright (C) Venelin Efremov 2014
 *  All rights reserved.
 */

#include "MethodBase.h"

namespace dbus {

MethodBase::~MethodBase() {

}

bool MethodBase::prepareCall() {
  return Message::forMethodCall(*this, &call_);
}

} /* namespace dbus */
