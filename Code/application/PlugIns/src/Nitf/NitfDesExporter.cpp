/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "Blob.h"
#include "DataElement.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "NitfConstants.h"
#include "NitfDesExporter.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "RasterElement.h"

#include <QtGui/QComboBox>
#include <QtGui/QFormLayout>
#include <QtGui/QWidget>
#include <errno.h>
#include <string.h>

REGISTER_PLUGIN(OpticksNitf, DesExporter, Nitf::DesExporter);

Nitf::DesExporter::DesExporter() : mpOptions(NULL), mpCombo(NULL)
{
   setName("NITF DES Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("DES data Files (*.des *.xml)");
   setDescription("Export DES Segments from NITF Raster Elements");
   setSubtype("RasterElement");
   setDescriptorId("{8cd4ffa6-726f-416b-8a8e-7467565e6e29}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::DesExporter::~DesExporter()
{
   delete mpOptions;
}

bool Nitf::DesExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   if (!ExporterShell::getInputSpecification(pArgList))
   {
      return false;
   }
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), "Raster element to export DES from."));
   VERIFY(pArgList->addArg<std::string>("DES Index", "Index of the DES to export. This value is the name "
               "of the metadata item underneath the \"DES\" item. For example: DES_000\n"
               "The default is to use the first DES which is not a TRE_OVERFLOW DES."));
   return true;
}

ValidationResultType Nitf::DesExporter::validate(const PlugInArgList* pArgList, std::string& errorMessage) const
{
   ValidationResultType res = ExporterShell::validate(pArgList, errorMessage);
   if (res != VALIDATE_FAILURE)
   {
      RasterElement* pElement = pArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
      const DynamicObject* pMetadata = (pElement == NULL) ? NULL : pElement->getMetadata();
      pMetadata = (pMetadata == NULL) ? NULL : dv_cast<DynamicObject>(
         &pMetadata->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::DES_METADATA));
      if (pMetadata == NULL)
      {
         errorMessage = "No valid DESs are contained in this file.";
         return VALIDATE_FAILURE;
      }
      std::vector<std::string> attrs;
      pMetadata->getAttributeNames(attrs);
      int cnt = 0;
      bool hasTreOverflow = false;
      for (std::vector<std::string>::const_iterator attr = attrs.begin(); attr != attrs.end(); ++attr)
      {
         const DynamicObject* pDes = dv_cast<DynamicObject>(&pMetadata->getAttribute(*attr));
         if (pDes != NULL)
         {
            if (pDes->getAttribute(Nitf::DesSubheaderFieldNames::DESID).toXmlString() != "TRE_OVERFLOW")
            {
               cnt++;
            }
            else
            {
               hasTreOverflow = true;
            }
         }
      }
      if (cnt == 1)
      {
         return res;
      }
      else if (cnt > 1)
      {
         std::string desIndex;
         pArgList->getPlugInArgValue<std::string>("DES Index", desIndex);
         if (desIndex.empty() && mpCombo == NULL)
         {
            errorMessage = "Please select a DES to export.";
            return VALIDATE_INPUT_REQUIRED;
         }
         return res;
      }
      if (hasTreOverflow)
      {
         errorMessage = "The only available DES is a TRE_OVERFLOW which can't be exported.";
      }
      else
      {
         errorMessage = "No valid DESs are contained in this file.";
      }
   }
   return VALIDATE_FAILURE;
}

QWidget* Nitf::DesExporter::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptions == NULL)
   {
      mpOptions = new QWidget(NULL);
      mpCombo = new QComboBox(mpOptions);
      QFormLayout* pLayout = new QFormLayout(mpOptions);
      pLayout->addRow("Select the DES to export.", mpCombo);
   }
   RasterElement* pElement = pInArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   const DynamicObject* pMetadata = (pElement == NULL) ? NULL : pElement->getMetadata();
   pMetadata = (pMetadata == NULL) ? NULL : dv_cast<DynamicObject>(
      &pMetadata->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::DES_METADATA));
   if (pMetadata == NULL)
   {
      return NULL;
   }
   std::vector<std::string> attrs;
   pMetadata->getAttributeNames(attrs);
   QStringList items;
   for (std::vector<std::string>::const_iterator attr = attrs.begin(); attr != attrs.end(); ++attr)
   {
      const DynamicObject* pDes = dv_cast<DynamicObject>(&pMetadata->getAttribute(*attr));
      if (pDes != NULL)
      {
         QString desid = QString::fromStdString(pDes->getAttribute(Nitf::DesSubheaderFieldNames::DESID).toXmlString());
         if (desid != "TRE_OVERFLOW")
         {
            items.push_back(QString::fromStdString(*attr));
         }
      }
   }
   mpCombo->clear();
   mpCombo->addItems(items);
   return mpOptions;
}

bool Nitf::DesExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Export DES.", "app", "{23202426-cd07-43be-940b-4369d2ca643f}");
   FileDescriptor* pDesc = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pDesc == NULL)
   {
      progress.report("No export file descriptor specified.", 0, ERRORS, true);
      return false;
   }
   RasterElement* pElement = pInArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   const DynamicObject* pMetadata = (pElement == NULL) ? NULL : pElement->getMetadata();
   pMetadata = (pMetadata == NULL) ? NULL : dv_cast<DynamicObject>(
      &pMetadata->getAttributeByPath(Nitf::NITF_METADATA + "/" + Nitf::DES_METADATA));
   if (pMetadata == NULL)
   {
      progress.report("Invalid or no raster element specified.", 0, ERRORS, true);
      return false;
   }
   std::string desIndex;
   if (mpCombo != NULL)
   {
      desIndex = mpCombo->currentText().toStdString();
   }
   else
   {
      pInArgList->getPlugInArgValue<std::string>("DES Index", desIndex);
   }
   const DynamicObject* pDes = NULL;
   if (!desIndex.empty())
   {
      pDes = dv_cast<DynamicObject>(&pMetadata->getAttribute(desIndex));
   }
   else
   {
      // grab the first non TRE_OVERFLOW DES
      std::vector<std::string> attrs;
      pMetadata->getAttributeNames(attrs);
      for (std::vector<std::string>::const_iterator attr = attrs.begin(); attr != attrs.end(); ++attr)
      {
         const DynamicObject* pTmp = dv_cast<DynamicObject>(&pMetadata->getAttribute(*attr));
         if (pTmp != NULL)
         {
            if (pTmp->getAttribute(Nitf::DesSubheaderFieldNames::DESID).toXmlString() != "TRE_OVERFLOW")
            {
               pDes = pTmp;
               break;
            }
         }
      }
   }
   if (pDes == NULL)
   {
      progress.report("Invalid DES specified.", 0, ERRORS, true);
   }
   progress.report("Exporting DES.", 10, NORMAL);
   const Blob* pData = dv_cast<Blob>(&pDes->getAttribute(Nitf::DesSubheaderFieldNames::DESDATA));
   if (pData == NULL)
   {
      progress.report("Invalid or missing DESDATA field.", 0, ERRORS, true);
      return false;
   }
   const std::vector<unsigned char>& data(pData->get());
   FileResource pOutFile(pDesc->getFilename().getFullPathAndName().c_str(), "wb");
   if (pOutFile.get() == NULL)
   {
      progress.report("Unable to open export file. Make sure you have write permissions.", 0, ERRORS, true);
      return false;
   }
   if (fwrite(&data.front(), sizeof(unsigned char), data.size(), pOutFile) != data.size())
   {
      progress.report(std::string("Unable to save DES: ") + strerror(errno), 0, ERRORS, true);
      return false;
   }

   progress.report("Export complete.", 100, NORMAL);
   progress.upALevel();
   return true;
}
