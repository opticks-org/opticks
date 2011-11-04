/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICEPSEUDOCOLORLAYERIMPORTER_H
#define ICEPSEUDOCOLORLAYERIMPORTER_H

#include "IceImporterShell.h"

class IcePseudocolorLayerImporter : public IceImporterShell
{
public:
   IcePseudocolorLayerImporter();
   ~IcePseudocolorLayerImporter();

   QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);
   bool getInputSpecification(PlugInArgList*& pArgList);

protected:
   bool parseInputArgList(PlugInArgList* pInArgList);
   SpatialDataView* createView() const;

private:
   IcePseudocolorLayerImporter& operator=(const IcePseudocolorLayerImporter& rhs);

   SpatialDataView* mpView;
};

#endif
