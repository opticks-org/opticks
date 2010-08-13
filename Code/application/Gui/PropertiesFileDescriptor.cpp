/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "FileDescriptor.h"
#include "FileDescriptorWidget.h"
#include "PropertiesFileDescriptor.h"

PropertiesFileDescriptor::PropertiesFileDescriptor()
{
   setName("File Descriptor Properties");
   setPropertiesName("File");
   setDescription("General setting properties of a file descriptor");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{C27E4657-635C-47C9-B70C-F5505AB071A6}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PropertiesFileDescriptor::~PropertiesFileDescriptor()
{
}

bool PropertiesFileDescriptor::initialize(SessionItem* pSessionItem)
{
   FileDescriptorWidget* pDescriptorPage = dynamic_cast<FileDescriptorWidget*>(getWidget());
   if (pDescriptorPage == NULL)
   {
      return false;
   }

   DataElement* pElement = dynamic_cast<DataElement*>(pSessionItem);
   if (pElement != NULL)
   {
      DataDescriptor* pDescriptor = pElement->getDataDescriptor();
      if (pDescriptor != NULL)
      {
         const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            pDescriptorPage->setFileDescriptor(pFileDescriptor);
            return true;
         }
      }
   }

   return false;
}

bool PropertiesFileDescriptor::applyChanges()
{
   FileDescriptorWidget* pDescriptorPage = dynamic_cast<FileDescriptorWidget*>(getWidget());
   VERIFY(pDescriptorPage != NULL);

   return pDescriptorPage->applyChanges();
}

QWidget* PropertiesFileDescriptor::createWidget()
{
   QWidget* pWidget = new FileDescriptorWidget();
   return pWidget;
}
