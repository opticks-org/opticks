/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DISPLAYQUERYOPTIONS_H
#define DISPLAYQUERYOPTIONS_H

#include "QueryOptions.h"

#include <string>

class DynamicObject;

class DisplayQueryOptions : public QueryOptions
{
public:
   DisplayQueryOptions();
   virtual ~DisplayQueryOptions();

   DisplayQueryOptions(const QueryOptions& rhs);
   /**
    * Convert options to a DynamicObject.
    * 
    * @return A DynamicObject containing the import options.  The caller is
    *         responsible for deleting this DynamicObject.
    */
   virtual DynamicObject* toDynamicObject() const;
   virtual void fromDynamicObject(const DynamicObject* pDynObj);

   void setQueryActive(bool bActive);
   bool getQueryActive() const;

   void setUniqueName(const std::string& nameString);
   const std::string& getUniqueName() const;

   void setOrder(int order);
   int getOrder() const;

   bool operator< ( const DisplayQueryOptions& rhs) const;

private:
   static const std::string UNIQUE_NAME;
   static const std::string CHECKED;
   static const std::string ORDER;

   // feature
   std::string mUniqueName;
   int mOrder;
   bool mbQueryActive;
};

#endif
