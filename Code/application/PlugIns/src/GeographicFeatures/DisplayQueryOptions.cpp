/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DisplayQueryOptions.h"
#include "DynamicObject.h"

const std::string DisplayQueryOptions::UNIQUE_NAME = "Unique Name";
const std::string DisplayQueryOptions::CHECKED = "Checked";
const std::string DisplayQueryOptions::ORDER = "Order";

DisplayQueryOptions::DisplayQueryOptions() : QueryOptions(),
   mUniqueName(std::string()),
   mOrder(0),
   mbQueryActive(true)
{}

DisplayQueryOptions::DisplayQueryOptions(const QueryOptions& rhs) : QueryOptions(rhs),
   mUniqueName(std::string()),
   mOrder(0),
   mbQueryActive(true)
{}

DisplayQueryOptions::~DisplayQueryOptions()
{}

void DisplayQueryOptions::fromDynamicObject(const DynamicObject* pDynObj)
{
   VERIFYNRV(pDynObj != NULL);
   QueryOptions::fromDynamicObject(pDynObj);
   pDynObj->getAttribute(UNIQUE_NAME).getValue(mUniqueName);
   pDynObj->getAttribute(CHECKED).getValue(mbQueryActive);
   pDynObj->getAttribute(ORDER).getValue(mOrder);
}

DynamicObject* DisplayQueryOptions::toDynamicObject() const
{
   DynamicObject* pDynObj = QueryOptions::toDynamicObject();
   VERIFYRV(pDynObj != NULL, NULL);

   pDynObj->setAttribute(UNIQUE_NAME, mUniqueName);
   pDynObj->setAttribute(CHECKED, mbQueryActive);
   pDynObj->setAttribute(ORDER, mOrder);

   return pDynObj;
}

void DisplayQueryOptions::setQueryActive(bool bActive)
{
   mbQueryActive = bActive;
}

bool DisplayQueryOptions::getQueryActive() const
{
   return mbQueryActive;
}

void DisplayQueryOptions::setUniqueName(const std::string& nameString)
{
   mUniqueName = nameString;
}

const std::string& DisplayQueryOptions::getUniqueName() const
{
   return mUniqueName;
}

void DisplayQueryOptions::setOrder(int order)
{
   mOrder = order;
}

int DisplayQueryOptions::getOrder() const
{
   return mOrder;
}

bool DisplayQueryOptions::operator< ( const DisplayQueryOptions& rhs) const
{
   bool bReturn = false;
   if (getOrder() < rhs.getOrder())
   {
      bReturn = true;
   }
   return bReturn;
}