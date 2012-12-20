/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIOIMPORTER_H
#define SIOIMPORTER_H

#include "RasterElementImporterShell.h"
#include "Testable.h"

class SioImporter : public RasterElementImporterShell, public Testable
{
public:
   SioImporter();
   ~SioImporter();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool validate(const DataDescriptor* pDescriptor, const std::vector<const DataDescriptor*>& importedDescriptors,
      std::string& errorMessage) const;
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool runOperationalTests(Progress *pProgress, std::ostream& failure);
   bool runAllTests(Progress *pProgress, std::ostream& failure);

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

private:
   bool ensureStatisticsReadProperly(Progress *pProgress, std::ostream& failure);
   bool mVersion9Sio;
};

#endif
