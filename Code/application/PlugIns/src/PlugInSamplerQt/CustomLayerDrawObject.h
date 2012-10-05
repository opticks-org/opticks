/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMLAYERDRAWOBJECT_H
#define CUSTOMLAYERDRAWOBJECT_H

#include "DrawObject.h"

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>

class CustomLayerDrawObject : public DrawObject
{
public:
   CustomLayerDrawObject();
   virtual ~CustomLayerDrawObject();

   virtual void draw();
   virtual bool getExtents(double& x1, double& y1, double& x4, double& y4);
   virtual void reset();

   virtual bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
};
#endif
