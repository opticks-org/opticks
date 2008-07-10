/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DTEDIMPORTER_H
#define DTEDIMPORTER_H

#include "AccHeader.h"
#include "DsiHeader.h"
#include "RasterElementImporterShell.h"
#include "UhlHeader.h"

class DtedImporter: public RasterElementImporterShell
{
public:
   DtedImporter();
   ~DtedImporter();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;

private:
   UhlHeader mUhl_h;
   DsiHeader mDsi_h;
   AccHeader mAcc_h;
};

#endif
