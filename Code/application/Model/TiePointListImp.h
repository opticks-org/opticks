/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTLISTIMP_H
#define TIEPOINTLISTIMP_H

#include "DataElementImp.h"
#include "TiePointList.h"

#include <string>
#include <vector>

class TiePointListImp : public DataElementImp
{
public:
   TiePointListImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~TiePointListImp();

   void setMissionDatasetName(std::string missionName);
   const std::string &getMissionDatasetName() const;
   LocationType toMission(LocationType refPixel) const;
   const std::vector<TiePoint>& getTiePoints() const;
   void adoptTiePoints(std::vector<TiePoint>& points);

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   // base class obligations
   bool toXml(XMLWriter* pWriter) const;
   bool fromXml(DOMNode* pNode, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

private:
   std::string mMissionName;
   std::vector<TiePoint> mTiePoints;
};

#define TIEPOINTLISTADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define TIEPOINTLISTADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   void setMissionDatasetName(std::string missionName) \
   { \
      impClass::setMissionDatasetName(missionName); \
   } \
   const std::string& getMissionDatasetName() const \
   { \
      return impClass::getMissionDatasetName(); \
   } \
   LocationType toMission(LocationType refPixel) const \
   { \
      return impClass::toMission(refPixel); \
   } \
   const std::vector<TiePoint>& getTiePoints() const \
   { \
      return impClass::getTiePoints(); \
   } \
   void adoptTiePoints(std::vector<TiePoint>& points) \
   { \
      impClass::adoptTiePoints(points); \
   }

#endif
