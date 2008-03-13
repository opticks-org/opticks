/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTGROUPIMP_H
#define PLOTGROUPIMP_H

#include "LocationType.h"
#include "PlotObjectImp.h"

#include <vector>

class PlotObject;

class PlotGroupImp : public PlotObjectImp
{
   Q_OBJECT

public:
   PlotGroupImp(PlotViewImp* pPlot, bool bPrimary);
   ~PlotGroupImp();

   PlotGroupImp& operator= (const PlotGroupImp& object);

   PlotObjectType getType() const;
   void draw();

   PlotObject* addObject(const PlotObjectType& eType);
   void insertObjects(const std::vector<PlotObject*>& objects);
   bool hasObject(PlotObject* pObject) const;
   const std::vector<PlotObject*>& getObjects() const;
   unsigned int getNumObjects() const;

   PlotObject* hitObject(LocationType point) const;
   bool hit(LocationType point) const;
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setVisible(bool bVisible);
   void setSelected(bool bSelect);

   virtual bool insertObject(PlotObject* pObject);
   virtual bool removeObject(PlotObject* pObject, bool bDelete = false);
   virtual void clear(bool bDelete = false);

signals:
   void objectAdded(PlotObject* pObject);
   void objectRemoved(PlotObject* pObject);

private:
   std::vector<PlotObject*> mObjects;
};

#define PLOTGROUPADAPTER_METHODS(impClass) \
   PLOTOBJECTADAPTER_METHODS(impClass) \
   virtual PlotObject* addObject(const PlotObjectType& eType) \
   { \
      return impClass::addObject(eType); \
   } \
   void insertObjects(const std::vector<PlotObject*>& objects) \
   { \
      return impClass::insertObjects(objects); \
   } \
   bool hasObject(PlotObject* pObject) const \
   { \
      return impClass::hasObject(pObject); \
   } \
   const std::vector<PlotObject*>& getObjects() const \
   { \
      return impClass::getObjects(); \
   } \
   unsigned int getNumObjects() const \
   { \
      return impClass::getNumObjects(); \
   } \
   PlotObject* hitObject(LocationType point) const \
   { \
      return impClass::hitObject(point); \
   } \
   bool hit(LocationType point) \
   { \
      return impClass::hit(point); \
   } \
   bool insertObject(PlotObject* pObject) \
   { \
      return impClass::insertObject(pObject); \
   } \
   bool removeObject(PlotObject* pObject, bool bDelete = false) \
   { \
      return impClass::removeObject(pObject, bDelete); \
   } \
   void clear(bool bDelete = false) \
   { \
      return impClass::clear(bDelete); \
   }

#endif
