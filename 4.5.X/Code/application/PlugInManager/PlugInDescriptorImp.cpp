/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDataStream>
#include <QtCore/QString>
#include <QtGui/QIcon>

#include "ApplicationServices.h"
#include "DynamicObjectAdapter.h"
#include "Executable.h"
#include "Exporter.h"
#include "Importer.h"
#include "InterpreterManager.h"
#include "ModuleDescriptor.h"
#include "PlugIn.h"
#include "PlugInArgListImp.h"
#include "PlugInDescriptorImp.h"
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

      if (Service<ApplicationServices>()->isInteractive())
      {
         setIcon(pPlugIn->getIcon());
      }

      Executable* pExecutable = dynamic_cast<Executable*>(pPlugIn);
      if (pExecutable != NULL)
      {
         mExecutableInterface = true;
         mExecuteOnStartup = pExecutable->isExecutedOnStartup();
         mDestroyAfterExecute = pExecutable->isDestroyedAfterExecute();
         mMenuLocations = pExecutable->getMenuLocations();

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

      InterpreterManager* pInterpreter = dynamic_cast<InterpreterManager*>(pPlugIn);
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

bool PlugInDescriptorImp::updateSettings(QDataStream& writer) const
{
   if (!getIcon().isNull())
   {
      //since the state of a non NULL QIcon cannot
      //be serialized/de-serialized, we can't
      //write it to the QDataStream, therefore
      //we can't cache this PlugInDescriptorImp,
      //so return false.
      return false;
   }

   writer << QString::fromStdString(getId());
   writer << QString::fromStdString(getName());
   writer << QString::fromStdString(mModuleName);
   writer << QString::fromStdString(mModuleFilename);
   writer << mPlugInNumber;
   writer << QString::fromStdString(mVersion);
   writer << mProductionStatus;
   writer << QString::fromStdString(mCreator);
   writer << QString::fromStdString(mCopyright);
   writer << QString::fromStdString(mDescription);
   writer << QString::fromStdString(mShortDescription);
   writer << QString::fromStdString(mType);
   writer << QString::fromStdString(mSubtype);
   writer << mAllowMultipleInstances;
   writer << mExecutableInterface;
   writer << mExecuteOnStartup;
   writer << mDestroyAfterExecute;
   writer << static_cast<quint64>(mMenuLocations.size());
   for (vector<string>::const_iterator locationIter = mMenuLocations.begin();
        locationIter != mMenuLocations.end(); ++locationIter)
   {
      writer << QString::fromStdString(*locationIter);
   }
   writer << mAbort;
   writer << mWizardSupport;
   writer << mBatchSupport;
   writer << mInteractiveSupport;
   writer << mImporterInterface;
   writer << mExporterInterface;
   writer << mInterpreterInterface;
   writer << QString::fromStdString(mFileExtensions);
   writer << mTestableInterface;
   writer << mTestable;

   writer << static_cast<quint64>(mDependencyCopyright.size());
   map<string,string>::const_iterator pCopyrights;
   for (pCopyrights = mDependencyCopyright.begin(); pCopyrights != mDependencyCopyright.end(); ++pCopyrights)
   {
      writer << QString::fromStdString(pCopyrights->first);
      writer << QString::fromStdString(pCopyrights->second);
   }

   vector<PlugInArgList*> lists(4);
   lists[0] = mpBatchInArgList;
   lists[1] = mpInteractiveInArgList;
   lists[2] = mpBatchOutArgList;
   lists[3] = mpInteractiveOutArgList;
   for (vector<PlugInArgList*>::iterator pListIter = lists.begin();
        pListIter != lists.end(); ++pListIter)
   {
      if (*pListIter == NULL)
      {
         writer << false;
      }
      else
      {
         writer << true;
         if (!static_cast<PlugInArgListImp*>(*pListIter)->updateSettings(writer))
         {
            return false;
         }
      }
   }

   return true;
}

PlugInDescriptorImp* PlugInDescriptorImp::fromSettings(QDataStream& reader)
{
   QString id;
   READ_FROM_STREAM(id);
   auto_ptr<PlugInDescriptorImp> pDescriptor (new PlugInDescriptorImp(id.toStdString(), NULL));
   bool success = pDescriptor->populateFromSettings(reader);
   if (success)
   {
      return pDescriptor.release();
   }
   return NULL;
}


bool PlugInDescriptorImp::populateFromSettings(QDataStream& reader)
{
   string name;
   READ_STR_FROM_STREAM(name);
   setName(name);
   READ_STR_FROM_STREAM(mModuleName);
   READ_STR_FROM_STREAM(mModuleFilename);
   READ_FROM_STREAM(mPlugInNumber);
   READ_STR_FROM_STREAM(mVersion);
   READ_FROM_STREAM(mProductionStatus);
   READ_STR_FROM_STREAM(mCreator);
   READ_STR_FROM_STREAM(mCopyright);
   READ_STR_FROM_STREAM(mDescription);
   READ_STR_FROM_STREAM(mShortDescription);
   READ_STR_FROM_STREAM(mType);
   READ_STR_FROM_STREAM(mSubtype);
   READ_FROM_STREAM(mAllowMultipleInstances);
   READ_FROM_STREAM(mExecutableInterface);
   READ_FROM_STREAM(mExecuteOnStartup);
   READ_FROM_STREAM(mDestroyAfterExecute);
   quint64 numMenuLocations;
   READ_FROM_STREAM(numMenuLocations);
   for (quint64 count = 0; count < numMenuLocations; ++count)
   {
      string menuLocation;
      READ_STR_FROM_STREAM(menuLocation);
      mMenuLocations.push_back(menuLocation);
   }
   READ_FROM_STREAM(mAbort);
   READ_FROM_STREAM(mWizardSupport);
   READ_FROM_STREAM(mBatchSupport);
   READ_FROM_STREAM(mInteractiveSupport);
   READ_FROM_STREAM(mImporterInterface);
   READ_FROM_STREAM(mExporterInterface);
   READ_FROM_STREAM(mInterpreterInterface);
   READ_STR_FROM_STREAM(mFileExtensions);
   READ_FROM_STREAM(mTestableInterface);
   READ_FROM_STREAM(mTestable);
   quint64 numCopyrights;
   READ_FROM_STREAM(numCopyrights);
   for (quint64 count = 0; count < numCopyrights; ++count)
   {
      string key;
      string value;
      READ_STR_FROM_STREAM(key);
      READ_STR_FROM_STREAM(value);
      mDependencyCopyright.insert(make_pair(key, value));
   }
   vector<PlugInArgList**> lists(4);
   lists[0] = &mpBatchInArgList;
   lists[1] = &mpInteractiveInArgList;
   lists[2] = &mpBatchOutArgList;
   lists[3] = &mpInteractiveOutArgList;

   for (vector<PlugInArgList**>::iterator pListIter = lists.begin();
        pListIter != lists.end(); ++pListIter)
   {
      PlugInArgList** pVal = *pListIter;
      *pVal = NULL;
      bool haveArgList;
      READ_FROM_STREAM(haveArgList);
      if (!haveArgList)
      {
         continue;
      }
      *pVal = PlugInArgListImp::fromSettings(reader);
      if (*pVal == NULL)
      {
         return false;
      }
   }
   return true;
}

bool PlugInDescriptorImp::serialize(SessionItemSerializer& serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool PlugInDescriptorImp::deserialize(SessionItemDeserializer& deserializer)
{
   return true;
}
