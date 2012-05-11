/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICEIMPORTERSHELL_H
#define ICEIMPORTERSHELL_H

#include "Hdf5ImporterShell.h"
#include "IceUtilities.h"

class DataDescriptor;
class ImportDescriptor;

class IceImporterShell : public Hdf5ImporterShell
{
public:
   IceImporterShell(IceUtilities::FileType fileType);
   ~IceImporterShell();

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   bool performImport() const;

protected:
   const IceUtilities::FileType mFileType;

private:
   IceImporterShell& operator=(const IceImporterShell& rhs);

   std::vector<std::string> mWarnings;
   std::vector<std::string> mErrors;
};

#endif
