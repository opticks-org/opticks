/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QString>

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "ConfigurationSettingsImp.h"
#include "DataDescriptor.h"
#include "DataDescriptorImp.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "ImportAgentImp.h"
#include "ImportDescriptor.h"
#include "ImportDlg.h"
#include "Importer.h"
#include "ImportOptionsDlg.h"
#include "ModelServices.h"
#include "MruFile.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"

#include <queue>
using namespace std;

void ImportAgentImp::instantiate(Progress* pProgress, bool batch)
{
   if (getInstantiated()) throw logic_error("ImportAgent can not be instantiated twice!");
   ExecutableAgentImp::instantiate(pProgress, batch);
   mpElement = NULL;
}

void ImportAgentImp::instantiate(const string& importerName, Progress* pProgress, bool batch)
{
   if (getInstantiated()) throw logic_error("ImportAgent can not be instantiated twice!");
   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
}

void ImportAgentImp::instantiate(const string& importerName, const string& filename, Progress* pProgress,
                                 bool batch)
{
   if (getInstantiated()) throw logic_error("ImportAgent can not be instantiated twice!");
   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
   setFilename(filename);
}

void ImportAgentImp::instantiate(const string& importerName, const vector<ImportDescriptor*>& descriptors,
                                 Progress* pProgress, bool batch)
{
   if (getInstantiated()) throw logic_error("ImportAgent can not be instantiated twice!");
   ExecutableAgentImp::instantiate(importerName, string(), pProgress, batch);
   mpElement = NULL;
   mDescriptors = descriptors;
}

void ImportAgentImp::instantiate(PlugIn* pPlugIn, const vector<ImportDescriptor*>& descriptors,
                                 Progress* pProgress, bool batch)
{
   if (getInstantiated()) throw logic_error("ImportAgent can not be instantiated twice!");
   ExecutableAgentImp::instantiate(pPlugIn, string(), pProgress, batch);
   mpElement = NULL;
   mDescriptors = descriptors;
}

ImportAgentImp::ImportAgentImp() :
   mEditType(ImportAgentExt1::NEVER_EDIT),
   mUpdateMruList(false),
   mpElement(NULL)
{
}

ImportAgentImp::~ImportAgentImp()
{
   Service<ModelServices> pModel;
   for (vector<ImportDescriptor*>::iterator iter = mDescriptors.begin(); iter != mDescriptors.end(); ++iter)
   {
      if (*iter != NULL)
      {
         pModel->destroyImportDescriptor(*iter);
      }
   }
}

void ImportAgentImp::setImporterSubtype(const string& subtype)
{
   checkInstantiate();

   Service<ApplicationServices> pApp;
   if (pApp->isInteractive() == true)
   {
      mImporterSubtype = subtype;
   }
}

string ImportAgentImp::getImporterSubtype() const
{
   checkInstantiate();
   return mImporterSubtype;
}

void ImportAgentImp::setFilename(const string& filename)
{
   checkInstantiate();
   mDescriptors.clear();

   QString strFilename = QString::fromStdString(filename);
   strFilename.replace(QRegExp("\\\\"), "/");

   mFilename = strFilename.toStdString();
}

void ImportAgentImp::setEditType(ImportAgentExt1::EditType editType)
{
   checkInstantiate();

   Service<ApplicationServices> pApp;
   if (pApp->isInteractive() == true)
   {
      mEditType = editType;
   }
}

ImportAgentExt1::EditType ImportAgentImp::getEditType() const
{
   checkInstantiate();
   return mEditType;
}

void ImportAgentImp::updateMruFileList(bool updateList)
{
   checkInstantiate();
   mUpdateMruList = updateList;
}

bool ImportAgentImp::isMruFileListUpdated() const
{
   checkInstantiate();
   return mUpdateMruList;
}

void ImportAgentImp::setImportDescriptors(const vector<ImportDescriptor*>& descriptors)
{
   checkInstantiate();
   vector<ImportDescriptor*>::const_iterator descIter, foundIter;
   Service<ModelServices> pModel;
   for (descIter = mDescriptors.begin(); descIter != mDescriptors.end(); ++descIter)
   {
      foundIter = find(descriptors.begin(), descriptors.end(), *descIter);
      if (foundIter == descriptors.end())
      {
         pModel->destroyImportDescriptor(*descIter);
      }
   }

   mDescriptors = descriptors;
}

vector<ImportDescriptor*> ImportAgentImp::getImportDescriptors()
{
   checkInstantiate();
   if(mDescriptors.empty())
   {
      Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
      if (pImporter != NULL)
      {
         mDescriptors = pImporter->getImportDescriptors(mFilename);
      }
   }

   return mDescriptors;
}

string ImportAgentImp::getDefaultExtensions() const
{
   checkInstantiate();
   string extensions;

   const Importer* pImporter = dynamic_cast<const Importer*>(getPlugIn());
   if (pImporter != NULL)
   {
      extensions = pImporter->getDefaultExtensions();
   }

   return extensions;
}

bool ImportAgentImp::execute()
{
   checkInstantiate();
   Service<ApplicationServices> pApp;
   Service<ModelServices> pModel;

   Importer* pImporter = dynamic_cast<Importer*>(getPlugIn());
   vector<ImportDescriptor*> descriptors = getImportDescriptors();

   if (pApp->isBatch() == true)
   {
      // Use the Auto Importer if an importer has not been set
      if (pImporter == NULL)
      {
         if (descriptors.empty() == false)
         {
            return false;
         }

         setPlugIn("Auto Importer");
         pImporter = dynamic_cast<Importer*>(getPlugIn());

         // Update the data sets to import if necessary
         if ((pImporter != NULL) && (mFilename.empty() == false))
         {
            descriptors = getImportDescriptors();
         }
      }
   }
   else if ((descriptors.empty() == true) && (mFilename.empty() == true))
   {
      Service<DesktopServices> pDesktop;

      // Get the import information from the user
      string importerName;

      const PlugIn* pPlugIn = getPlugIn();
      if (pPlugIn != NULL)
      {
         importerName = pPlugIn->getName();
      }

      ImportDlg importDlg(QString::fromStdString(mImporterSubtype), QString::fromStdString(importerName),
         pDesktop->getMainWidget());
      if (importDlg.exec() == QDialog::Rejected)
      {
         return false;
      }

      // Importer
      PlugIn* pSelectedImporter = importDlg.getSelectedImporter();
      pImporter = dynamic_cast<Importer*>(pSelectedImporter);
      setPlugIn(pSelectedImporter);

      // Data sets
      descriptors = importDlg.getImportDescriptors();
      mDescriptors = descriptors;

      // Filename
      QStringList filenames = importDlg.selectedFiles();
      if (filenames.empty() == false)
      {
         QString strFilename = filenames.front();
         if (strFilename.isEmpty() == false)
         {
            strFilename.replace(QRegExp("\\\\"), "/");
            mFilename = strFilename.toStdString();
         }
      }
   }
   else
   {
      // Use the Auto Importer if an importer has not been set
      if (pImporter == NULL)
      {
         if (descriptors.empty() == false)
         {
            return false;
         }

         setPlugIn("Auto Importer");
         pImporter = dynamic_cast<Importer*>(getPlugIn());

         // Update the data sets to import if necessary
         if ((pImporter != NULL) && (mFilename.empty() == false))
         {
            descriptors = getImportDescriptors();
         }
      }

      // Validate the descriptors
      bool bValidImport = validateImportDescriptors(descriptors, pImporter);

      // Display the options dialog if necessary
      if ((pImporter != NULL) && ((mEditType == ImportAgentExt1::ALWAYS_EDIT) ||
         ((mEditType == ImportAgentExt1::AS_NEEDED_EDIT) && (bValidImport == false))))
      {
         Service<DesktopServices> pDesktop;
         vector<ImportDescriptor*> importerDescriptors = descriptors;

         ImportOptionsDlg optionsDlg(pImporter, importerDescriptors, pDesktop->getMainWidget());
         if (optionsDlg.exec() == QDialog::Rejected)
         {
            return false;
         }

         // Update the import descriptors to load and destroy all others
         descriptors.clear();

         vector<ImportDescriptor*>::iterator iter;
         for (iter = importerDescriptors.begin(); iter != importerDescriptors.end(); ++iter)
         {
            ImportDescriptor* pImportDescriptor = *iter;
            if (pImportDescriptor != NULL)
            {
               if (pImportDescriptor->isImported() == true)
               {
                  descriptors.push_back(pImportDescriptor);
               }
               else
               {
                  pModel->destroyImportDescriptor(pImportDescriptor);
               }
            }
         }

         mDescriptors = descriptors;
      }

      // Ensure the descriptors are valid; they could be invalid if the user cancelled the options dialog
      if (validateImportDescriptors(descriptors, pImporter) == false)
      {
         return false;
      }
   }

   if (pImporter == NULL)
   {
      return false;
   }

   if (descriptors.empty() == true)
   {
      return false;
   }

   Progress* pProgress = getProgress();
   createProgressDialog();

   // Create the data elements for all the selected data sets
   bool success = true;
   vector<DataDescriptor*> selectedDescriptors;
   for (vector<ImportDescriptor*>::iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor == NULL || !pImportDescriptor->isImported())
      {
         continue;
      }
      DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
      if (pDescriptor == NULL)
      {
         continue;
      }
      // Allow the importer to modify the data descriptor if necessary
      pImporter->polishDataDescriptor(pDescriptor);

      string errorMessage;
      if (pImporter->validate(pDescriptor, errorMessage) == false)
      {
         if (pProgress != NULL)
         {
            if (errorMessage.empty() == true)
            {
               errorMessage = "The data set cannot be loaded by the importer.";
            }

            pProgress->updateProgress(errorMessage, 0, ERRORS);
         }

         success = false;
         continue;
      }
      else if(!errorMessage.empty())
      {
         // warning from validate
         if (pProgress != NULL)
         {
            int currentPercent;
            string currentText;
            ReportingLevel currentLevel;
            pProgress->getProgress(currentText, currentPercent, currentLevel);
            pProgress->updateProgress(errorMessage, currentPercent, WARNING);
         }
      }
      selectedDescriptors.push_back(pDescriptor);
   }
   vector<DataElement*> createdElementsVec = pModel->createElements(selectedDescriptors);
   if(createdElementsVec.size() < selectedDescriptors.size())
   {
      if(pProgress != NULL)
      {
         if(createdElementsVec.empty())
         {
            pProgress->updateProgress("Unable to create any data elements", 0, ERRORS);
            return false;
         }
         else
         {
            pProgress->updateProgress("Unable to create some of the data elements", 0, ERRORS);
         }
      }
      success = false;
   }
   list<DataElement*> createdElements;
   copy(createdElementsVec.begin(), createdElementsVec.end(), back_inserter(createdElements));

   // build a queue of elements to import
   queue<DataElement*> toVisit;
   for (list<DataElement*>::iterator elmnt = createdElements.begin(); elmnt != createdElements.end(); ++elmnt)
   {
      // does the element's parent need to be imported?
      if (find(createdElements.begin(), createdElements.end(), (*elmnt)->getParent()) == createdElements.end())
      {
         // if not, add it to the visit queue
         toVisit.push(*elmnt);
      }
   }

   // Load the data sets
   while(!toVisit.empty())
   {
      mpElement = toVisit.front();
      toVisit.pop();
      createdElements.remove(mpElement);

      if (!ExecutableAgentImp::executePlugIn())
      {
         success = false;
         if (pProgress != NULL)
         {
            // If there isn't already an error message, post one to progress
            string text;
            int percent;
            ReportingLevel gran;
            pProgress->getProgress(text, percent, gran);
            if (gran != ABORT && gran != ERRORS)
            {
               string message = "The '" + mpElement->getName() + "' data set failed to load!";
               pProgress->updateProgress(message, 0, ERRORS);
            }
         }
         pModel->destroyElement(mpElement);
      }
      else
      {
         DataDescriptorImp *pDdi = dynamic_cast<DataDescriptorImp*>(mpElement->getDataDescriptor());
         if (pDdi != NULL)
         {
            pDdi->setImporterName(getPlugIn()->getName());
         }
         mImportedElements.push_back(mpElement);
         for (list<DataElement*>::iterator elmnt = createdElements.begin(); elmnt != createdElements.end(); ++elmnt)
         {
            if((*elmnt)->getParent() == mpElement)
            {
               toVisit.push(*elmnt);
            }
         }
      }

      mpElement = NULL;
   }

   if ((success == true) && (mUpdateMruList == true))
   {
      updateMruFileList();
   }

   return success;
}

vector<DataElement*> ImportAgentImp::getImportedElements() const
{
   checkInstantiate();
   return mImportedElements;
}

bool ImportAgentImp::validateImportDescriptors(const vector<ImportDescriptor*>& descriptors, Importer* pImporter)
{
   if ((descriptors.empty() == true) || (pImporter == NULL))
   {
      return false;
   }

   for (vector<ImportDescriptor*>::const_iterator iter = descriptors.begin(); iter != descriptors.end(); ++iter)
   {
      ImportDescriptor* pImportDescriptor = *iter;
      if (pImportDescriptor != NULL)
      {
         // At least one data set must be selected for import
         if (pImportDescriptor->isImported() == true)
         {
            DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();

            string errorMessage;
            if (pImporter->validate(pDescriptor, errorMessage) == true)
            {
               // The import is valid if at least one data set validates successfully
               return true;
            }
         }
      }
   }

   return false;
}

void ImportAgentImp::populateArgValues(PlugInArgList *pArgList)
{
   checkInstantiate();
   if (pArgList != NULL)
   {
      ExecutableAgentImp::populateArgValues(pArgList);

      // Data element
      PlugInArg* pImportElementArg = NULL;
      pArgList->getArg(Importer::ImportElementArg(), pImportElementArg);
      if (pImportElementArg != NULL)
      {
         if (pImportElementArg->isActualSet() == false)
         {
            pImportElementArg->setPlugInArgValueLoose(mpElement);
         }
      }
   }
}

void ImportAgentImp::updateMruFileList()
{
   if (mFilename.empty() == true)
   {
      return;
   }

   // Determine if the filename really is a file
   QString strFilename = QString::fromStdString(mFilename);
   strFilename.replace(QRegExp("\\\\"), "/");

   if (QFile::exists(strFilename) == false)
   {
      return;
   }

   // Remove the current file if it exists in the list
   ConfigurationSettingsImp* pSettings = ConfigurationSettingsImp::instance();
   Service<ModelServices> pModel;

   vector<MruFile> mruFiles = pSettings->getMruFiles();
   for (vector<MruFile>::iterator iter = mruFiles.begin(); iter != mruFiles.end(); ++iter)
   {
      MruFile mruFile = *iter;

      QString strName = QString::fromStdString(mruFile.mName).toLower();
      QString strCurrentName = strFilename.toLower();
      if (strName == strCurrentName)
      {
         // Destroy the import descriptors in this MRU file
         vector<ImportDescriptor*>::iterator descriptorIter;
         for (descriptorIter = mruFile.mDescriptors.begin();
              descriptorIter != mruFile.mDescriptors.end();
              ++descriptorIter)
         {
            ImportDescriptor* pImportDescriptor = *descriptorIter;
            if (pImportDescriptor != NULL)
            {
               pModel->destroyImportDescriptor(pImportDescriptor);
            }
         }

         mruFiles.erase(iter);
         break;
      }
   }

   // Get the number of MRU files from the Options and subtract 1
   unsigned int maxNumFiles = ConfigurationSettings::getSettingNumberOfMruFiles() - 1;
   while (mruFiles.size() > maxNumFiles)
   {
      // Destroy the existing import descriptors in the MRU file
      MruFile mruFile = mruFiles.back();

      vector<ImportDescriptor*>::iterator iter;
      for (iter = mruFile.mDescriptors.begin(); iter != mruFile.mDescriptors.end(); ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         if (pImportDescriptor != NULL)
         {
            pModel->destroyImportDescriptor(pImportDescriptor);
         }
      }

      // Remove the MRU file from the vector
      mruFiles.pop_back();
   }

   // Add only the descriptors that are imported to the MRU file
   vector<ImportDescriptor*> mruDescriptors;

   vector<ImportDescriptor*>::iterator descriptorIter;
   for (descriptorIter = mDescriptors.begin(); descriptorIter != mDescriptors.end(); ++descriptorIter)
   {
      ImportDescriptor* pImportDescriptor = *descriptorIter;
      if (pImportDescriptor != NULL)
      {
         if (pImportDescriptor->isImported() == true)
         {
            // Copy the import descriptor to set in the MRU file since they are destroyed in the destructor
            DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               DataDescriptor* pMruDescriptor = pDescriptor->copy();
               if (pMruDescriptor != NULL)
               {
                  ImportDescriptor* pMruImportDescriptor = pModel->createImportDescriptor(pMruDescriptor, true);
                  if (pMruImportDescriptor != NULL)
                  {
                     mruDescriptors.push_back(pMruImportDescriptor);
                  }
               }
            }
         }
      }
   }

   // Add the file to the MRU file vector
   string importer;

   const PlugIn* pPlugIn = getPlugIn();
   if (pPlugIn != NULL)
   {
      importer = pPlugIn->getName();
   }

   mruFiles.insert(mruFiles.begin(), MruFile(strFilename.toStdString(), importer, mruDescriptors));

   // Update the MRU file list in the configuration settings
   pSettings->setMruFiles(mruFiles);
}
