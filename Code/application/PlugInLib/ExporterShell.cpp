/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ExporterShell.h"
#include "FileDescriptor.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

using namespace std;

ExporterShell::ExporterShell()
{
   setType(PlugInManagerServices::ExporterType());
}

ExporterShell::~ExporterShell()
{
}

bool ExporterShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

string ExporterShell::getDefaultExtensions() const
{
   return mExtension;
}

bool ExporterShell::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = true;
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);
   success = success && pArgList->addArg<FileDescriptor>(ExportDescriptorArg());
   success = success && pArgList->addArg<Progress>(ProgressArg());
   return success;
}

ValidationResultType ExporterShell::validate(const PlugInArgList* pArgList, string& errorMessage) const
{
   FileDescriptor* pFileDescriptor = pArgList->getPlugInArgValueUnsafe<FileDescriptor>(ExportDescriptorArg());
   if ((pFileDescriptor == NULL) || (pFileDescriptor->getFilename().getFullPathAndName().empty()))
   {
      errorMessage = "No output file specified.";
      return VALIDATE_FAILURE;
   }
   else if (pFileDescriptor->getFilename().isDirectory())
   {
      errorMessage = "Save location is a directory.";
      return VALIDATE_FAILURE;
   }
   return VALIDATE_SUCCESS;
}

QWidget* ExporterShell::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   return NULL;
}

void ExporterShell::setExtensions(const string& extensions)
{
   mExtension = extensions;
}
