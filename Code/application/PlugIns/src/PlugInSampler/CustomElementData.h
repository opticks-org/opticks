/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMELEMENTDATA_H
#define CUSTOMELEMENTDATA_H

#include "AnyData.h"

class CustomElementData : public AnyData
{
public:
   CustomElementData()
   {
   }

   CustomElementData(int value) :
   mValue(value)
   {
   }

   ~CustomElementData()
   {
   }

   AnyData* copy() const
   {
      CustomElementData* pData = new CustomElementData(mValue);
      return pData;
   }

   void setValue(int value)
   {
      mValue = value;
   }

   int getValue() const
   {
      return mValue;
   }

private:
   int mValue;
};

#endif
