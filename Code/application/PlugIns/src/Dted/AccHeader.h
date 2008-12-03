/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ACCHEADER_H
#define ACCHEADER_H

#include <stdio.h>

const int ACC_SIZE = 2700;
const int ACC_PROPER_TOTAL = 2700;

class AccHeader
{
public:
   AccHeader();

   bool readHeader(FILE* pInputFile);
   size_t getTotalHeaderSize();

private:
   char mAcc[ACC_SIZE]; // ACC
   size_t mTotalHeaderSize;
};

#endif
