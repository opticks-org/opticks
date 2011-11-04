/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
  * The documentation of this class is in GcpList.h
  */
#ifndef GCPLISTIMP_H
#define GCPLISTIMP_H

#include "DataElementImp.h"
#include "GcpList.h"

#include <iterator>
#include <list>

class GcpListImp : public DataElementImp
{
public:
   GcpListImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~GcpListImp();

   int getCount() const;
   const std::list<GcpPoint>& getSelectedPoints() const;
   void addPoints(const std::list<GcpPoint>& points);
   void addPoint(const GcpPoint& point);
   void removePoints(const std::list<GcpPoint>& points);
   void removePoint(const GcpPoint& point);
   void clearPoints();

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

   static bool gcpsToXml(std::list<GcpPoint>::const_iterator first, std::list<GcpPoint>::const_iterator last,
      XMLWriter* pXml);
   static bool xmlToGcps(std::back_insert_iterator<std::list<GcpPoint> > first, DOMNode* pDoc, unsigned int version);

private:
   GcpListImp(const GcpListImp& rhs);
   std::list<GcpPoint> mSelected;
};

#define GCPLISTADAPTEREXTENSION_CLASSES \
   DATAELEMENTADAPTEREXTENSION_CLASSES

#define GCPLISTADAPTER_METHODS(impClass) \
   DATAELEMENTADAPTER_METHODS(impClass) \
   int getCount() const \
   { \
      return impClass::getCount(); \
   } \
   const std::list<GcpPoint>& getSelectedPoints() const \
   { \
      return impClass::getSelectedPoints(); \
   } \
   void addPoints(const std::list<GcpPoint>& points) \
   { \
      impClass::addPoints(points); \
   } \
   void addPoint(const GcpPoint& point) \
   { \
      impClass::addPoint(point); \
   } \
   void removePoints(const std::list<GcpPoint>& points) \
   { \
      impClass::removePoints(points); \
   } \
   void removePoint(const GcpPoint& point) \
   { \
      impClass::removePoint(point); \
   } \
   void clearPoints() \
   { \
      impClass::clearPoints(); \
   }

#endif
