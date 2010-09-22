/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImporterShell.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"

using namespace std;

ImporterShell::ImporterShell()
{
   setType(PlugInManagerServices::ImporterType());
   setWizardSupported(false);
}

ImporterShell::~ImporterShell()
{
}

string ImporterShell::getDefaultExtensions() const
{
   return mExtension;
}

bool ImporterShell::isProcessingLocationSupported(ProcessingLocation location) const
{
   return (location == IN_MEMORY);
}

QWidget* ImporterShell::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   return NULL;
}

bool ImporterShell::validate(const DataDescriptor* pDescriptor, string& errorMessage) const
{
   if (pDescriptor == NULL)
   {
      errorMessage = "The data set is invalid!";
      return false;
   }

   const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      errorMessage = "The data set does not contain valid file information!";
      return false;
   }

   // Valid filename
   const string& filename = pFileDescriptor->getFilename();
   if (filename.empty() == true)
   {
      errorMessage = "The filename is invalid!";
      return false;
   }

   // Existing file
   LargeFileResource file(true);
   if (!file.open(filename.c_str(), O_RDONLY | O_BINARY, S_IREAD))
   {
      errorMessage = "The file: " + filename + " does not exist!";
      return false;
   }

   // Existing element
   Service<ModelServices> pModel;
   if (pModel.get() != NULL)
   {
      const string& name = pDescriptor->getName();
      const string& type = pDescriptor->getType();
      DataElement* pParent = pDescriptor->getParent();

      if (pModel->getElement(name, type, pParent) != NULL)
      {
         errorMessage = "The data set currently exists!  It may have already been imported.";
         return false;
      }
   }

   return true;
}

QWidget* ImporterShell::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
   return NULL;
}

void ImporterShell::polishDataDescriptor(DataDescriptor* pDescriptor)
{
   // Do nothing to the data descriptor
}

void ImporterShell::setExtensions(const string& extensions)
{
   mExtension = extensions;
}
