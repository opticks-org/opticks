/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODISL1BIMPORTER_H
#define MODISL1BIMPORTER_H

#include "ConfigurationSettings.h"
#include "Hdf4Element.h"
#include "Hdf4ImporterShell.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

class DynamicObject;
class ModisL1bImportOptionsWidget;

class ModisL1bImporter : public Hdf4ImporterShell
{
public:
   ModisL1bImporter();
   virtual ~ModisL1bImporter();

   SETTING(RasterConversion, ModisL1bImporter, std::string, "NoConversion");

   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);
   virtual bool validate(const DataDescriptor* pDescriptor,
      const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const;
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;
   virtual void performGeoreference() const;
   virtual SpatialDataView* createView() const;
   virtual bool createRasterPager(RasterElement* pRaster) const;

   void populateAttributes(const Hdf4Element::AttributeContainer& attributes, DynamicObject* pMetadata) const;

private:
   std::auto_ptr<ModisL1bImportOptionsWidget> mpOptionsWidget;
   std::map<std::string, std::pair<bool, bool> > mLatLonImported;
};

#endif
