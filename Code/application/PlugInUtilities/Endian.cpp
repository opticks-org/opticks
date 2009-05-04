/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Endian.h"
#include "AppConfig.h"

Endian::Endian(EndianType endian) :
   mType(endian),
   mSystemType(getSystemEndian())
{
}

Endian::Endian() : 
   mType(getSystemEndian() == BIG_ENDIAN_ORDER ? LITTLE_ENDIAN_ORDER : BIG_ENDIAN_ORDER),
   mSystemType(getSystemEndian())
{
}

Endian::~Endian()
{
}

EndianType Endian::getEndian() const
{
   return mType;
}

bool Endian::isBigEndian() const
{
   return (mType == BIG_ENDIAN_ORDER);
}

bool Endian::isLittleEndian() const
{
   return (mType == LITTLE_ENDIAN_ORDER);
}

EndianType Endian::getSystemEndian()
{
   if (OPTICKS_BYTE_ORDER == LITTLE_ENDIAN_BYTE_ORDER)
   {
      return LITTLE_ENDIAN_ORDER;
   }

   return BIG_ENDIAN_ORDER;
}
