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
#include "Testable.h"

#include <ossim/base/ossimConstants.h>
#include <string>
#include <map>

class ossimNitfImageHeaderV2_X;

namespace Nitf
{
   class NitfImporter : public RasterElementImporterShell, public Testable
   {
   public:
      NitfImporter();
      ~NitfImporter();

      std::vector<ImportDescriptor*> getImportDescriptors(const std::string &filename);
      unsigned char getFileAffinity(const std::string& filename);
      bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
      SpatialDataView* createView() const;
      PlugIn* getGeoreferencePlugIn() const;
      bool createRasterPager(RasterElement *pRaster) const;

      static EncodingType ossimImageHeaderToEncodingType(ossimNitfImageHeaderV2_X* pImgHeader);

      virtual bool runOperationalTests(Progress* pProgress, std::ostream& failure);
      virtual bool runAllTests(Progress* pProgress, std::ostream& failure);

   protected:
      virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

   private:
      std::map<std::string, std::string> mParseMessages;
   };
}
#endif
