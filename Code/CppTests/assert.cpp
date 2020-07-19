/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"

unsigned int AssertionCounter::smSuccesses = 0;
unsigned int AssertionCounter::smFailures = 0;

std::vector<std::string> AssertionFailureLog::smLog;

void AssertionCounter::initialize()
{ 
   smSuccesses = 0;
   smFailures = 0;
}

