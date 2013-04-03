/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATUREQUERYOPTIONS_H
#define FEATUREQUERYOPTIONS_H

#include "QueryOptions.h"

#include <string>
#include <vector>

class DynamicObject;

class FeatureQueryOptions : public QueryOptions
{
public:
   FeatureQueryOptions();
   virtual ~FeatureQueryOptions();

   /**
    * Convert options to a DynamicObject.
    * 
    * @return A DynamicObject containing the import options.  The caller is
    *         responsible for deleting this DynamicObject.
    */
   virtual DynamicObject* toDynamicObject() const;
   virtual void fromDynamicObject(const DynamicObject* pDynObj);

   void setFormatString(const std::string& formatString);
   const std::string& getFormatString() const;

   const std::vector<std::string>& getGraphicObjectIds() const;
   void addGraphicObjectId(const std::string& id);
   void clearGraphicObjectIds();

private:
   static const std::string FORMAT_STRING;
   static const std::string DISPLAY_QUERIES_ACTIVATED;

   // feature
   std::string mFormatString;
   std::vector<std::string> mGraphicObjectIds;
};

#endif
