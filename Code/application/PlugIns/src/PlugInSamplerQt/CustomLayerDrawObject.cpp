/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AppVerify.h"
#include "CustomLayer.h"
#include "CustomLayerDrawObject.h"
#include "DataVariant.h"
#include "DataVariantAnyData.h"
#include "LocationType.h"

#include <GL/glew.h>
#include <vector>

CustomLayerDrawObject::CustomLayerDrawObject()
{}

CustomLayerDrawObject::~CustomLayerDrawObject()
{}

void CustomLayerDrawObject::draw()
{
   // Get the corners of the rectangle to be drawn from the layer's data element.
   // In this example plug-in, method getExtents() returns the corners of the rectangle to be drawn.
   LocationType llCorner;
   LocationType urCorner;
   if (getExtents(llCorner.mX, llCorner.mY, urCorner.mX, urCorner.mY) == false)
   {
      return;
   }

   // now draw the transparent red rectangle with black outline
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor4ub(255, 0, 0, 100);
   glBegin(GL_POLYGON);
   glVertex2d(llCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glVertex2d(llCorner.mX, urCorner.mY);
   glEnd();
   glDisable(GL_BLEND);

   glColor3ub(0, 0, 0);
   glBegin(GL_LINE_LOOP);
   glVertex2d(llCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, llCorner.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glVertex2d(llCorner.mX, urCorner.mY);
   glEnd();
}

bool CustomLayerDrawObject::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   // For this example we're going to return the extents of the drawn rectangle to insure zoomExtents() will
   // display the entire rectangle.
   CustomLayer* pLayer = getLayer();
   if (pLayer == NULL)
   {
      return false;
   }

   Any* pAny = dynamic_cast<Any*>(pLayer->getDataElement());
   if (pAny == NULL)
   {
      return false;
   }

   DataVariantAnyData* pAnyData = dynamic_cast<DataVariantAnyData*>(pAny->getData());
   if (pAnyData == NULL)
   {
      return false;
   }

   DataVariant& customData = pAnyData->getAttribute();
   VERIFY(customData.isValid());
   std::vector<double> rectangle = dv_cast<std::vector<double> >(customData);
   VERIFY(rectangle.size() == 4);
   x1 = rectangle[0];
   y1 = rectangle[1];
   x4 = rectangle[2];
   y4 = rectangle[3];
   return true;
}

void CustomLayerDrawObject::reset()
{
   // No properties to reset
   return;
}

bool CustomLayerDrawObject::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (button != Qt::LeftButton)
   {
      return false;
   }

   LocationType pixel;
   Layer* pLayer = getLayer();
   if (pLayer == NULL)
   {
      return false;
   }
   pLayer->translateScreenToData(screenCoord.x(), screenCoord.y(), pixel.mX, pixel.mY);

   // Define the opposite corners of a rectangle as the custom element data
   std::vector<double> corners;
   double centerToSide(20.0);
   double centerToTop(40.0);
   corners.push_back(pixel.mX - centerToSide);      // left edge
   corners.push_back(pixel.mY - centerToTop);       // top edge
   corners.push_back(pixel.mX + centerToSide);      // right edge
   corners.push_back(pixel.mY + centerToTop);       // bottom edge

   Any* pElement = dynamic_cast<Any*>(pLayer->getDataElement());
   if (pElement == NULL)
   {
      return false;
   }

   DataVariantAnyData* pAnyData = dynamic_cast<DataVariantAnyData*>(pElement->getData());
   if (pAnyData == NULL)  // first time so need to create the rectangle data
   {
      FactoryResource<DataVariantAnyData> pNewAnyData;
      pNewAnyData->setAttribute(DataVariant(corners));

      // Set the custom data into the element
      pElement->setData(pNewAnyData.release());
   }
   else
   {
      DataVariant& customData = pAnyData->getAttribute();
      if (customData.isValid() == false)
      {
         return false;
      }
      pAnyData->setAttribute(DataVariant(corners));
   }

   return true;
}