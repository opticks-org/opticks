/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GENERICIMPORTER_H
#define GENERICIMPORTER_H

#include "RasterElementImporterShell.h"

class GenericImporter : public RasterElementImporterShell
{
public:
   GenericImporter();
   virtual ~GenericImporter();

   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;
};

#endif
