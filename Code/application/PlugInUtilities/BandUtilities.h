/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef BANDUTILITIES_H
#define BANDUTILITIES_H

#include <vector>

template <class T>
int* deriveBadBands(const std::vector<T>& bandsToSave, const unsigned int totalBands,
                    unsigned int& numBad);

#endif
