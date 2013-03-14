/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CustomLayer.h"
#include "DrawObject.h"

DrawObject::DrawObject() :
   mpLayer(NULL)
{}

DrawObject::~DrawObject()
{
   setLayer(NULL);
   notify(SIGNAL_NAME(Subject, Deleted));
}

void DrawObject::setLayer(CustomLayer* pLayer)
{
   if (pLayer != mpLayer)
   {
      mpLayer = pLayer;
      notify(SIGNAL_NAME(DrawObject, LayerChanged), boost::any(mpLayer));
   }
}

CustomLayer* DrawObject::getLayer()
{
   return mpLayer;
}

const CustomLayer* DrawObject::getLayer() const
{
   return mpLayer;
}

bool DrawObject::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool DrawObject::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool DrawObject::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool DrawObject::processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool DrawObject::getExtents(std::vector<LocationType>& worldCoords)
{
   double minX = 0.0;
   double maxX = 0.0;
   double minY = 0.0;
   double maxY = 0.0;
   if (getExtents(minX, minY, maxX, maxY) == false)
   {
      return false;
   }

   worldCoords.clear();
   worldCoords.push_back(LocationType(minX, minY));
   worldCoords.push_back(LocationType(maxX, minY));
   worldCoords.push_back(LocationType(maxX, maxY));
   worldCoords.push_back(LocationType(minX, maxY));

   return true;
}
