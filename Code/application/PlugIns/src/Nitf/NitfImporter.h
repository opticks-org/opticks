/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFIMPORTER_H
#define NITFIMPORTER_H

#include "RasterElementImporterShell.h"

#include <ossim/base/ossimConstants.h>
#include <string>
#include <map>

namespace Nitf
{
   class NitfImporter : public RasterElementImporterShell
   {
   public:
      NitfImporter();
      ~NitfImporter();

      std::vector<ImportDescriptor*> getImportDescriptors(const std::string &filename);
      unsigned char getFileAffinity(const std::string& filename);
      bool execute(PlugInArgList* pInArgs, PlugInArgList* pOutArgs);
      bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
      SpatialDataView* createView() const;
      bool createRasterPager(RasterElement *pRaster) const;
      bool validateDefaultOnDiskReadOnly(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

      static EncodingType ossimEncodingToEncodingType(ossimScalarType scalar);

   private:
      std::map<std::string, std::string> mParseMessages;
   };
}
#endif
