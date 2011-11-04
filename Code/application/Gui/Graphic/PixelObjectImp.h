/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXELOBJECTIMP_H
#define PIXELOBJECTIMP_H

#include "GraphicObjectImp.h"

/**
 * Abstract superclass for GraphicObjects which need to draw
 * when sufficiently zoomed.
 *
 * Implement drawPixels() and drawVector() and draw accordingly.
 * The results of drawPixels is kept in a display list, so drawPixels()
 * is only called when there is a change.  Indicate a change by signaling 
 * modified() or propertyModified().
 */
class PixelObjectImp : public GraphicObjectImp
{
public:
   void draw(double zoomFactor) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   PixelObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~PixelObjectImp();

   virtual void drawPixels(double zoomFactor) const = 0;
   virtual void drawVector(double zoomFactor) const = 0;

private:
   PixelObjectImp(const PixelObjectImp& rhs);
   PixelObjectImp& operator=(const PixelObjectImp& rhs);
   mutable int mDisplayList;
};

#define PIXELOBJECTADAPTEREXTENSION_CLASSES \
   GRAPHICOBJECTADAPTEREXTENSION_CLASSES

#define PIXELOBJECTADAPTER_METHODS \
  GRAPHICOBJECTADAPTER_METHODS

#endif
