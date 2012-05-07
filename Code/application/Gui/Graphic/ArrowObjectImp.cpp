/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "glCommon.h"
#include "ArrowObjectImp.h"
#include "AppConfig.h"
#include "GraphicLayer.h"

using namespace std;

ArrowObjectImp::ArrowObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                               LocationType pixelCoord) :
   LineObjectImp(id, type, pLayer, pixelCoord)
{
   addProperty("Scale");
}

void ArrowObjectImp::draw(double zoomFactor) const
{
   LineObjectImp::draw(zoomFactor);

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   double lineWidth = getLineWidth();
   double scaleFactor = getScale();

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pLayer->translateDataToScreen(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
      pLayer->translateDataToScreen(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);
   }

   double arrowHeadSize = 10;
   double h = arrowHeadSize * sqrt(lineWidth) * scaleFactor;
   double theta = atan2(urCorner.mY - llCorner.mY, urCorner.mX - llCorner.mX);
   double hcTheta = h * cos(theta);
   double hsTheta = h * sin(theta);

   LocationType end1;
   LocationType end2;
   end1.mX = urCorner.mX - hcTheta - hsTheta;
   end1.mY = urCorner.mY - hsTheta + hcTheta;
   end2.mX = urCorner.mX - hcTheta + hsTheta;
   end2.mY = urCorner.mY - hsTheta - hcTheta;

   if (pLayer != NULL)
   {
      pLayer->translateScreenToData(end1.mX, end1.mY, end1.mX, end1.mY);
      pLayer->translateScreenToData(end2.mX, end2.mY, end2.mX, end2.mY);
      pLayer->translateScreenToData(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
      pLayer->translateScreenToData(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);
   }

   glLineWidth(lineWidth);
#if defined(WIN_API)
   glEnable(GL_LINE_SMOOTH);
#else
   if (lineWidth == 1.0)
   {
      glEnable(GL_LINE_SMOOTH);
   }
   else
   {
      glDisable(GL_LINE_SMOOTH);
   }
#endif
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glBegin(GL_LINE_STRIP);
   glVertex2d(end1.mX, end1.mY);
   glVertex2d(urCorner.mX, urCorner.mY);
   glVertex2d(end2.mX, end2.mY);
   glEnd();

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glLineWidth(1);
}

const string& ArrowObjectImp::getObjectType() const
{
   static string type("ArrowObjectImp");
   return type;
}

bool ArrowObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ArrowObject"))
   {
      return true;
   }

   return LineObjectImp::isKindOf(className);
}
