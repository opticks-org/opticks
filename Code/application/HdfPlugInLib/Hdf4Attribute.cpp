/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <string>
#include <vector>

#include "Hdf4Attribute.h"
#include "AppVerify.h"

using namespace std;

Hdf4Attribute::Hdf4Attribute(const string& name, const DataVariant& value) :
   mName(name), mValue(value)
{
}


Hdf4Attribute::~Hdf4Attribute()
{
}

const string& Hdf4Attribute::getName() const
{
   return mName;
}

void Hdf4Attribute::setVariant(const DataVariant &var)
{
   mValue = var;
}

const DataVariant &Hdf4Attribute::getVariant() const
{
   return mValue;
}

DataVariant &Hdf4Attribute::getVariant()
{
   return mValue;
}
