/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef DTEDSHARED_H
#define DTEDSHARED_H

#include <stdio.h>
#include <stdlib.h>

// define helper functions for the data

bool readLatLongInterval(FILE* pInputFile, float &lli);
bool readLatLongCount(FILE* pInputFile, int &llc);
int hemValue(char c);
bool readLatLong(FILE* pInputFile, float &latLong, bool lat);

#endif

 
