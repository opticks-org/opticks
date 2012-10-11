/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMLAYERPLUGIN_H
#define CUSTOMLAYERPLUGIN_H

#include "AttachmentPtr.h"
#include "CustomLayer.h"
#include "ViewerShell.h"

class PlugInArgList;

class CustomLayerPlugIn : public ViewerShell
{
public:
   CustomLayerPlugIn();
   virtual ~CustomLayerPlugIn();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   virtual QWidget* getWidget() const;
   void setLayerDrawObject();

   void layerDeleted(Subject& subject, const std::string& signal, const boost::any& value);

private:
   AttachmentPtr<CustomLayer> mpLayer;
};

#endif
