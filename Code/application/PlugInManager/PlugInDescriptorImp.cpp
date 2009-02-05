/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QBuffer>
#include <QtGui/QIcon>

#include "ApplicationServices.h"
#include "PlugInDescriptorImp.h"
#include "Executable.h"
#include "Exporter.h"
#include "Icons.h"
#include "Importer.h"
#include "Interpreter.h"
#include "PlugIn.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "ProgressAdapter.h"
#include "PropertiesPlugInDescriptor.h"
#include "Service.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "xmlreader.h"
#include "xmlwriter.h"
#include "Testable.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

PlugInDescriptorImp::PlugInDescriptorImp(const string& id, PlugIn* pPlugIn) :
   SessionItemImp(id),
   mPlugInNumber(0),
   mProductionStatus(false),
   mAllowMultipleInstances(false),
   mExecutableInterface(false),
   mExecuteOnStartup(false),
   mDestroyAfterExecute(true),
   mpMenuIcon(NULL),
   mAbort(false),
   mWizardSupport(false),
   mBatchSupport(false),
   mInteractiveSupport(false),
   mpBatchInArgList(NULL),
   mpInteractiveInArgList(NULL),
   mpBatchOutArgList(NULL),
   mpInteractiveOutArgList(NULL),
   mImporterInterface(false),
   mExporterInterface(false),
   mInterpreterInterface(false),
   mTestableInterface(false),
   mTestable(false)
{
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mPlugIn);
   }

   if (pPlugIn != NULL)
   {
      setName(pPlugIn->getName());
      mVersion = pPlugIn->getVersion();
      mProductionStatus = pPlugIn->isProduction();
      mCreator = pPlugIn->getCreator();
      mCopyright = pPlugIn->getCopyright();
      mDependencyCopyright = pPlugIn->getDependencyCopyright();
      mDescription = pPlugIn->getDescription();
      mShortDescription = pPlugIn->getShortDescription();
      mType = pPlugIn->getType();
      mSubtype = pPlugIn->getSubtype();
      mAllowMultipleInstances = pPlugIn->areMultipleInstancesAllowed();

      Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
      if (pExecutable != NULL)
      {
         mExecutableInterface = true;
         mExecuteOnStartup = pExecutable->isExecutedOnStartup();
         mDestroyAfterExecute = pExecutable->isDestroyedAfterExecute();
         mMenuLocations = pExecutable->getMenuLocations();

         if (Service<ApplicationServices>()->isInteractive())
         {
            setupMenuIcon(QPixmap(pExecutable->getMenuIcon()));
         }

         mAbort = pExecutable->hasAbort();
         mWizardSupport = pExecutable->hasWizardSupport();

         mBatchSupport = pExecutable->setBatch();
         if (mBatchSupport)
         {
            pExecutable->getInputSpecification(mpBatchInArgList);
            pExecutable->getOutputSpecification(mpBatchOutArgList);
         }

         mInteractiveSupport = pExecutable->setInteractive();
         if (mInteractiveSupport)
         {
            pExecutable->getInputSpecification(mpInteractiveInArgList);
            pExecutable->getOutputSpecification(mpInteractiveOutArgList);
         }
      }

      Importer* pImporter = dynamic_cast<Importer*>(pPlugIn);
      if (pImporter != NULL)
      {
         mImporterInterface = true;
         mFileExtensions = pImporter->getDefaultExtensions();
      }

      Exporter* pExporter = dynamic_cast<Exporter*>(pPlugIn);
      if (pExporter != NULL)
      {
         mExporterInterface = true;
         mFileExtensions = pExporter->getDefaultExtensions();
      }

      Interpreter* pInterpreter = dynamic_cast<Interpreter*>(pPlugIn);
      if (pInterpreter != NULL)
      {
         mInterpreterInterface = true;
         mFileExtensions = pInterpreter->getFileExtensions();
      }

      Testable* pTestable = dynamic_cast<Testable*>(pPlugIn);
      if (pTestable != NULL)
      {
         mTestableInterface = true;
         mTestable = true;
      }
   }

   addPropertiesPage(PropertiesPlugInDescriptor::getName());
}

PlugInDescriptorImp::~PlugInDescriptorImp()
{
   try
   {
      Service<PlugInManagerServices> pManager;
      if (mpBatchInArgList != NULL)
      {
         pManager->destroyPlugInArgList(mpBatchInArgList);
      }

      if (mpInteractiveInArgList != NULL)
      {
         pManager->destroyPlugInArgList(mpInteractiveInArgList);
      }

      if (mpBatchOutArgList != NULL)
      {
         pManager->destroyPlugInArgList(mpBatchOutArgList);
      }

      if (mpInteractiveOutArgList != NULL)
      {
         pManager->destroyPlugInArgList(mpInteractiveOutArgList);
      }

      delete [] mpMenuIcon;
      destroyPlugIns();
   }
   catch (...)
   {
      // nothing we can do, but we don't want to throw from a destructor
   }
}

void PlugInDescriptorImp::setModuleName(const string& moduleName)
{
   mModuleName = moduleName;
}

void PlugInDescriptorImp::setModuleFileName(const string& moduleFilename)
{
   mModuleFilename = moduleFilename;
}

void PlugInDescriptorImp::setPlugInNumber(unsigned int ulPlugIn)
{
   mPlugInNumber = ulPlugIn;
}

string PlugInDescriptorImp::getModuleName() const
{
   return mModuleName;
}

string PlugInDescriptorImp::getModuleFileName() const
{
   return mModuleFilename;
}

unsigned int PlugInDescriptorImp::getPlugInNumber() const
{
   return mPlugInNumber;
}

string PlugInDescriptorImp::getVersion() const
{
   return mVersion;
}

bool PlugInDescriptorImp::isProduction() const
{
   return mProductionStatus;
}

string PlugInDescriptorImp::getCreator() const
{
   return mCreator;
}

string PlugInDescriptorImp::getCopyright() const
{
   return mCopyright;
}

map<string, string> PlugInDescriptorImp::getDependencyCopyright() const
{
   return mDependencyCopyright;
}

string PlugInDescriptorImp::getDescription() const
{
   return mDescription;
}

string PlugInDescriptorImp::getShortDescription() const
{
   return mShortDescription;
}

string PlugInDescriptorImp::getType() const
{
   return mType;
}

string PlugInDescriptorImp::getSubtype() const
{
   return mSubtype;
}

bool PlugInDescriptorImp::areMultipleInstancesAllowed() const
{
   return mAllowMultipleInstances;
}

bool PlugInDescriptorImp::hasExecutableInterface() const
{
   return mExecutableInterface;
}

bool PlugInDescriptorImp::isExecutedOnStartup() const
{
   return mExecuteOnStartup;
}

bool PlugInDescriptorImp::isDestroyedAfterExecute() const
{
   return mDestroyAfterExecute;
}

const vector<string>& PlugInDescriptorImp::getMenuLocations() const
{
   return mMenuLocations;
}

const char** PlugInDescriptorImp::getMenuIcon() const
{
   return const_cast<const char**>(mpMenuIcon);
}

bool PlugInDescriptorImp::hasAbort() const
{
   return mAbort;
}

bool PlugInDescriptorImp::hasWizardSupport() const
{
   return mWizardSupport;
}

bool PlugInDescriptorImp::hasBatchSupport() const
{
   return mBatchSupport;
}

bool PlugInDescriptorImp::hasInteractiveSupport() const
{
   return mInteractiveSupport;
}

const PlugInArgList* PlugInDescriptorImp::getBatchInputArgList() const
{
   return mpBatchInArgList;
}

const PlugInArgList* PlugInDescriptorImp::getInteractiveInputArgList() const
{
   return mpInteractiveInArgList;
}

const PlugInArgList* PlugInDescriptorImp::getBatchOutputArgList() const
{
   return mpBatchOutArgList;
}

const PlugInArgList* PlugInDescriptorImp::getInteractiveOutputArgList() const
{
   return mpInteractiveOutArgList;
}

bool PlugInDescriptorImp::hasImporterInterface() const
{
   return mImporterInterface;
}

bool PlugInDescriptorImp::hasExporterInterface() const
{
   return mExporterInterface;
}

bool PlugInDescriptorImp::hasInterpreterInterface() const
{
   return mInterpreterInterface;
}

string PlugInDescriptorImp::getFileExtensions() const
{
   return mFileExtensions;
}

bool PlugInDescriptorImp::hasTestableInterface() const
{
   return mTestableInterface;
}

bool PlugInDescriptorImp::isTestable() const
{
   return mTestable;
}

bool PlugInDescriptorImp::addPlugIn(PlugIn* pPlugIn)
{
   if (pPlugIn == NULL)
   {
      return false;
   }

   // Do not add the plug-in if it is already in the list
   bool bExists = containsPlugIn(pPlugIn);
   if (bExists == true)
   {
      return false;
   }

   // Only add plug-ins of the same name and type
   bool bAdd = false;

   string plugInName = pPlugIn->getName();
   string plugInType = pPlugIn->getType();

   if ((plugInName.empty() == false) && (plugInType.empty() == false))
   {
      if ((plugInName == getName()) && (plugInType == mType))
      {
         bAdd = true;
      }
   }

   if (bAdd == true)
   {
      mPlugIns.insert(pair<PlugIn*, ProgressAdapter*>(pPlugIn, NULL));
   }

   return bAdd;
}

bool PlugInDescriptorImp::containsPlugIn(PlugIn* pPlugIn) const
{
   if (pPlugIn == NULL)
   {
      return false;
   }

   map<PlugIn*, ProgressAdapter*>::const_iterator iter = mPlugIns.find(pPlugIn);
   return (iter != mPlugIns.end());
}

vector<PlugIn*> PlugInDescriptorImp::getPlugIns() const
{
   vector<PlugIn*> plugIns;

   map<PlugIn*, ProgressAdapter*>::const_iterator iter = mPlugIns.begin();
   while (iter != mPlugIns.end())
   {
      PlugIn* pPlugIn = iter->first;
      if (pPlugIn != NULL)
      {
         plugIns.push_back(pPlugIn);
      }

      ++iter;
   }

   return plugIns;
}

int PlugInDescriptorImp::getNumPlugIns() const
{
   int iCount = 0;
   iCount = mPlugIns.size();

   return iCount;
}

Progress* PlugInDescriptorImp::getProgress(PlugIn* pPlugIn)
{
   if (pPlugIn == NULL)
   {
      return NULL;
   }

   map<PlugIn*, ProgressAdapter*>::const_iterator iter = mPlugIns.find(pPlugIn);
   if (iter == mPlugIns.end())
   {
      return NULL;
   }

   ProgressAdapter* pProgress = iter->second;
   if (pProgress == NULL)
   {
      pProgress = new ProgressAdapter();
      if (pProgress != NULL)
      {
         pProgress->setPlugIn(pPlugIn);
         mPlugIns[pPlugIn] = pProgress;
      }
   }

   return pProgress;
}

bool PlugInDescriptorImp::destroyPlugIn(PlugIn* pPlugIn)
{
   if (pPlugIn == NULL)
   {
      return false;
   }

   // Destroy the plug-in
   delete pPlugIn;

   map<PlugIn*, ProgressAdapter*>::iterator iter = mPlugIns.find(pPlugIn);
   if (iter != mPlugIns.end())
   {
      // Destroy the Progress object
      ProgressAdapter* pProgress = iter->second;
      if (pProgress != NULL)
      {
         delete pProgress;
      }

      // Remove the plug-in instance from the map
      mPlugIns.erase(iter);
   }

   return true;
}

void PlugInDescriptorImp::destroyPlugIns()
{
   map<PlugIn*, ProgressAdapter*>::const_iterator iter = mPlugIns.begin();
   while (iter != mPlugIns.end())
   {
      try
      {
         // Destroy the plug-in
         PlugIn* pPlugIn = iter->first;
         if (pPlugIn != NULL)
         {
            delete pPlugIn;
         }

         // Destroy the Progress object
         ProgressAdapter* pProgress = iter->second;
         if (pProgress != NULL)
         {
            delete pProgress;
         }
      }
      catch (...)
      {
         // if one throws, we still want to delete the others
      }
      ++iter;
   }

   // Clear the map
   mPlugIns.clear();
}

bool PlugInDescriptorImp::serialize(SessionItemSerializer& serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool PlugInDescriptorImp::deserialize(SessionItemDeserializer& deserializer)
{
   return true;
}

void PlugInDescriptorImp::setupMenuIcon(const QPixmap& menuIcon)
{
   if (menuIcon.isNull() == false)
   {
      // Save the QPixmap data into a buffer in a XPM format.
      QByteArray byte;
      QBuffer buffer(&byte);
      buffer.open(QIODevice::ReadWrite);
      menuIcon.save(&buffer, "XPM");

      // Remove the white spaces in the byte array.
      byte = byte.simplified();

      // Remove both the header infomation from the beginning of the 
      // byte array(the first 34 bytes in the array) and the }; from the end
      // (the last 3 bytes).
      byte.remove(0, 34);
      byte.remove(byte.size() - 3, 3);

      // Use the QString split method to sparate the data in the byte array.
      QString byteString = byte;
      QStringList xpmArray = byteString.split("\", \"");

      // Take the resulting QStringList and deep copy the data to mpMenuIcon.
      mpMenuIcon = new char*[xpmArray.size()];
      for (int i = 0; i < xpmArray.size(); ++i)
      {
         mpMenuIcon[i] = new char[xpmArray[i].size() + 1];
         strcpy(mpMenuIcon[i], xpmArray[i].toAscii());
      }
   }
}
