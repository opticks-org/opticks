/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5Data.h"
#include "Hdf5Utilities.h"

using namespace std;

Hdf5Data::Hdf5Data() :
 mTypeName(HdfUtilities::UNKNOWN_TYPE)
{
}

string Hdf5Data::getTypeName() const
{
   return mTypeName;
}

void Hdf5Data::setTypeName(const string& typeName)
{
   mTypeName = typeName;
}

const vector<hsize_t>& Hdf5Data::getDimensionSizes() const
{
   return mDimensionSizes;
}

void Hdf5Data::setDimensionSizes(const vector<hsize_t>& dimensionSizes)
{
   mDimensionSizes = dimensionSizes;
}
