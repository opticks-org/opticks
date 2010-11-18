/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXELASPECTRATIO_H
#define PIXELASPECTRATIO_H

#include "ViewerShell.h"
#include "PixelAspectRatioGui.h"

class PixelAspectRatio : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   PixelAspectRatio();
   ~PixelAspectRatio();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   bool showGui();
   PixelAspectRatioGui* mpGui;
};

#endif
