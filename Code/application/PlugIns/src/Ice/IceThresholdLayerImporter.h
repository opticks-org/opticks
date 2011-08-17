/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICETHRESHOLDLAYERIMPORTER_H
#define ICETHRESHOLDLAYERIMPORTER_H

#include "IceImporterShell.h"

class SpatialDataView;

class IceThresholdLayerImporter : public IceImporterShell
{
public:
   IceThresholdLayerImporter();
   virtual ~IceThresholdLayerImporter();

   virtual QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

protected:
   virtual bool parseInputArgList(PlugInArgList* pInArgList);
   virtual SpatialDataView* createView() const;

private:
   SpatialDataView* mpView;
};

#endif
