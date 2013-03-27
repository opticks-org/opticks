/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONLAYERIMP_H
#define ANNOTATIONLAYERIMP_H

#include "GraphicLayerImp.h"
#include "GraphicGroup.h"
#include "TypesFile.h"

#include <list>
#include <QtGui/QFont>

class AnnotationLayer;
class AnnotationObjectImp;

/**
  * This class displays an annotation layer. It contains a list of annotation
  * objects and manages all drawing, inserting, deleting and manipulation of
  * objects in that list.
  */
class AnnotationLayerImp : public GraphicLayerImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   AnnotationLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~AnnotationLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   AnnotationLayerImp &operator = (const AnnotationLayerImp &annotationLayer);

   LayerType getLayerType() const;

   QCursor getMouseCursor() const;

   /**
    * Correct the coordinate for whatever snapping is required.
    *
    * The implementation in AnnotationLayerImp will snap to grid as desired.
    *
    * @param coord
    *        Coordinate to correct.
    *
    * @return The corrected coordinate.
    */
   LocationType correctCoordinate(const LocationType &coord) const;

   static void setDefaultFont(const QFont& font);
   static QFont getDefaultFont();

   void completeInsertion(bool bValidObject = true);

   virtual QColor getLabelColor(const GraphicObjectImp *pObj);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void reset();
   void setSnapToGrid(bool snap);
   void objectWasAdded(GraphicObject *pObj);

signals:
   void snapToGridChanged(bool snap);

private:
   AnnotationLayerImp(const AnnotationLayerImp& rhs);

   bool mSnapToGrid;
};

#define ANNOTATIONLAYERADAPTEREXTENSION_CLASSES \
   GRAPHICLAYERADAPTEREXTENSION_CLASSES

#define ANNOTATIONLAYERADAPTER_METHODS(impClass) \
   GRAPHICLAYERADAPTER_METHODS(impClass)
#endif
