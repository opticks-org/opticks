/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICGROUPIMP_H
#define GRAPHICGROUPIMP_H

#include "GraphicObjectImp.h"
#include "GraphicProperty.h"
#include "TypesFile.h"
#include "xmlwriter.h"

#include <string>
#include <list>

class GraphicLayer;

class GraphicGroupImp : public GraphicObjectImp
{
   Q_OBJECT

public:
   GraphicGroupImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~GraphicGroupImp();

   GraphicGroupImp& operator= (const GraphicGroupImp& graphicGroup);

   void draw(double zoomFactor) const;
   bool setProperty(const GraphicProperty* pProperty);
   void updateBoundingBox();
   void updateLayout();
   void updateHandles();
   bool hit(LocationType pixelCoord) const;
   GraphicObject* hitObject(const LocationType& pixelCoord) const;
   bool getExtents(std::vector<LocationType>& dataCoords) const;
   bool getRotatedExtents(std::vector<LocationType>& dataCoords) const;
   virtual void updateGeo();
   virtual void enableGeo();

   GraphicObject* createObject(GraphicObjectType eType, LocationType pixelCoord = LocationType());
   virtual GraphicObject* addObject(GraphicObjectType eType,
      LocationType point = LocationType());
   void insertObject(GraphicObject* pObject);
   void insertObjects(std::list<GraphicObject*>& objects);
   void insertObjects(const std::list<GraphicObject*>& objects);
   bool hasObject(GraphicObject* pObject) const;
   const std::list<GraphicObject*>& getObjects() const;
   bool moveObjectToBack(GraphicObject* pObject);
   bool moveObjectToFront(GraphicObject* pObject);
   int getObjectStackingIndex(GraphicObject* pObject) const;
   void setObjectStackingIndex(GraphicObject* pObject, int index);
   virtual bool removeObject(GraphicObject* pObject, bool bDelete);
   virtual void removeAllObjects(bool bDelete);

   bool replicateObject(const GraphicObject* pObject);
   CgmObject* convertToCgm();

   bool setFillState(bool bFill);
   bool setFont(const QFont& textFont);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual void setLayer(GraphicLayer *pLayer);

signals:
   void abortedAdd(GraphicObject *pObj);

protected:
   std::list<GraphicObject*> mObjects;

protected slots:
   void updateFromProperty(GraphicProperty* pProperty);

private:
   GraphicGroupImp(const GraphicGroupImp& rhs);
   bool mbNeedsLayout;
   LocationType mLlCorner;
   LocationType mUrCorner;

   template<typename T, typename U>
   bool propagateProperty(T method, U value);
};

#define GRAPHICGROUPADAPTEREXTENSION_CLASSES \
   GRAPHICOBJECTADAPTEREXTENSION_CLASSES

#define GRAPHICGROUPADAPTER_METHODS(impClass) \
   GRAPHICOBJECTADAPTER_METHODS(impClass) \
   GraphicObject *addObject(GraphicObjectType type, \
      LocationType point = LocationType()) \
   { \
      return impClass::addObject(type, point); \
   } \
   const std::list<GraphicObject*> &getObjects() const \
   { \
      return impClass::getObjects(); \
   } \
   void insertObject(GraphicObject *pObject) \
   { \
      return impClass::insertObject(pObject); \
   } \
   bool removeObject(GraphicObject *pObject, bool bDelete) \
   { \
      return impClass::removeObject(pObject, bDelete); \
   }\
   void removeAllObjects(bool bDelete) \
   { \
      return impClass::removeAllObjects(bDelete); \
   } \
   bool hasObject(GraphicObject *pObject) \
   { \
      return impClass::hasObject(pObject); \
   }

#endif   // GRAPHICGROUP_H
