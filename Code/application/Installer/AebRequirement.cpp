/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebRequirement.h"

AebRequirement::AebRequirement()
{
}

AebRequirement::AebRequirement(AebId id, AebVersion min, AebVersion max) :
	 mId(id), mMin(min), mMax(max)
{
   if (!isValid())
   {
      mId = AebId();
      mMin = AebVersion();
      mMax = AebVersion();
   }
}

AebRequirement::AebRequirement(const AebRequirement& other) :
	 mId(other.mId), mMin(other.mMin), mMax(other.mMax)
{
}

bool AebRequirement::isValid() const
{
   return mId.isValid() && mMin.isValid() && mMax.isValid();
}

AebId AebRequirement::getId() const
{
   return mId;
}

AebVersion AebRequirement::getMin() const
{
   return mMin;
}

AebVersion AebRequirement::getMax() const
{
   return mMax;
}

bool AebRequirement::meets(AebVersion version) const
{
   if (!isValid())
   {
      return false;
   }
   return mMin <= version && version <= mMax;
}
