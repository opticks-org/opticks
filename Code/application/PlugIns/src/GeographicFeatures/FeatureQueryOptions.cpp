/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DynamicObject.h"
#include "FeatureQueryOptions.h"

const std::string FeatureQueryOptions::FORMAT_STRING = "Format string";

FeatureQueryOptions::FeatureQueryOptions() : QueryOptions(),
   mFormatString(std::string())
{}

FeatureQueryOptions::~FeatureQueryOptions()
{}

void FeatureQueryOptions::fromDynamicObject(const DynamicObject* pDynObj)
{
   VERIFYNRV(pDynObj != NULL);
   QueryOptions::fromDynamicObject(pDynObj);
   pDynObj->getAttribute(FORMAT_STRING).getValue(mFormatString);
}

DynamicObject* FeatureQueryOptions::toDynamicObject() const
{
   DynamicObject* pDynObj = QueryOptions::toDynamicObject();
   VERIFYRV(pDynObj != NULL, NULL);
   pDynObj->setAttribute(FORMAT_STRING, mFormatString);
   return pDynObj;
}

void FeatureQueryOptions::setFormatString(const std::string &formatString)
{
   mFormatString = formatString;
}

const std::string &FeatureQueryOptions::getFormatString() const
{
   return mFormatString;
}

const std::vector<std::string>& FeatureQueryOptions::getGraphicObjectIds() const
{
   return mGraphicObjectIds;
}

void FeatureQueryOptions::addGraphicObjectId(const std::string& id)
{
   mGraphicObjectIds.push_back(id);
}

void FeatureQueryOptions::clearGraphicObjectIds()
{
   mGraphicObjectIds.clear();
}