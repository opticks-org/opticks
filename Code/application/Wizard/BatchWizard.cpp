/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BatchFileset.h"
#include "BatchWizard.h"
#include "ObjectFactory.h"
#include "Value.h"
#include "xmlwriter.h"

using namespace std;

BatchWizard::BatchWizard() :
   mbClean(false),
   mpRepeatFileset(NULL)
{
}

BatchWizard::~BatchWizard()
{
   vector<Value*>::iterator iter;
   for (iter = mInputValues.begin(); iter != mInputValues.end(); iter++)
   {
      delete (*iter);
   }
}

void BatchWizard::setWizardFilename(const string& filename)
{
   mWizardFilename = filename;
}

const string& BatchWizard::getWizardFilename() const
{
   return mWizardFilename;
}

void BatchWizard::setRepeatFileset(BatchFileset* pFileset)
{
   mpRepeatFileset = pFileset;
   addFileset(pFileset);
}

bool BatchWizard::isRepeating(string& repeatName) const
{
   if (mpRepeatFileset != NULL)
   {
      repeatName = mpRepeatFileset->getName();
      return true;
   }

   return false;
}

void BatchWizard::setCleanup(bool bCleanup)
{
   mbClean = bCleanup;
}

bool BatchWizard::doesCleanup() const
{
   return mbClean;
}

void BatchWizard::addFileset(BatchFileset* pFileset)
{
   if (pFileset != NULL)
   {
      bool bContains = false;
      bContains = containsFileset(pFileset);
      if (bContains == false)
      {
         mFilesets.push_back(pFileset);
      }
   }
}

BatchFileset* BatchWizard::getFileset(const string& filesetName) const
{
   vector<BatchFileset*>::const_iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = *iter;
      if (pFileset != NULL)
      {
         string name = pFileset->getName();
         if (name == filesetName)
         {
            return pFileset;
         }
      }
   }

   return NULL;
}

const vector<BatchFileset*>& BatchWizard::getFilesets() const
{
   return mFilesets;
}

int BatchWizard::getNumFilesets() const
{
   int iCount = static_cast<int>(mFilesets.size());
   return iCount;
}

bool BatchWizard::containsFileset(BatchFileset* pFileset) const
{
   if (pFileset == NULL)
   {
      return false;
   }

   for (vector<BatchFileset*>::const_iterator iter = mFilesets.begin(); iter != mFilesets.end(); ++iter)
   {
      BatchFileset* pCurrentFileset = *iter;
      if (pCurrentFileset != NULL)
      {
         if (pCurrentFileset == pFileset)
         {
            return true;
         }
      }
   }

   return false;
}

bool BatchWizard::removeFileset(BatchFileset* pFileset)
{
   for (vector<BatchFileset*>::iterator iter = mFilesets.begin(); iter != mFilesets.end(); ++iter)
   {
      BatchFileset* pCurrentFileset = *iter;
      if (pCurrentFileset != NULL)
      {
         if (pCurrentFileset == pFileset)
         {
            if (pFileset == mpRepeatFileset)
            {
               mpRepeatFileset = NULL;
            }

            mFilesets.erase(iter);
            return true;
         }
      }
   }

   return false;
}

Value* BatchWizard::setInputValue(const string& itemName, const string& nodeName, const string& nodeType,
                                  const DataVariant& value)
{
   if (hasInputValue(itemName, nodeName, nodeType) == true)
   {
      return NULL;
   }

   Value* pValue = new Value(itemName, nodeName, nodeType, value);
   mInputValues.push_back(pValue);

   return pValue;
}

bool BatchWizard::hasInputValue(const string& itemName, const string& nodeName, const string& nodeType) const
{
   for (vector<Value*>::const_iterator iter = mInputValues.begin(); iter != mInputValues.end(); ++iter)
   {
      Value* pValue = *iter;
      if (pValue != NULL)
      {
         if ((pValue->getItemName() == itemName) && (pValue->getNodeName() == nodeName) &&
            (pValue->getNodeType() == nodeType))
         {
            return true;
         }
      }
   }

   return false;
}

const vector<Value*>& BatchWizard::getInputValues() const
{
   return mInputValues;
}

void BatchWizard::initializeFilesets(ObjectFactory* pObjFact)
{
   if (mpRepeatFileset != NULL)
   {
      mpRepeatFileset->updateFileset(pObjFact);
      mpRepeatFileset->getFirstFile();
   }

   vector<BatchFileset*>::iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = *iter;
      if ((pFileset != NULL) && (pFileset != mpRepeatFileset))
      {
         pFileset->updateFileset(pObjFact);
         pFileset->getFirstFile();
      }
   }
}

void BatchWizard::updateFilesets()
{
   if (mpRepeatFileset != NULL)
   {
      mpRepeatFileset->getNextFile();
   }

   vector<BatchFileset*>::iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = *iter;
      if ((pFileset != NULL) && (pFileset != mpRepeatFileset))
      {
         pFileset->getNextFile();
      }
   }
}

void BatchWizard::getCurrentRepeatFile(string& currentFile) const
{
   currentFile.erase();

   if (mpRepeatFileset != NULL)
   {
      currentFile = mpRepeatFileset->getCurrentFile().c_str();
   }
}

void BatchWizard::getCurrentFilesetFile(const string& filesetName, string& currentFile) const
{
   currentFile.erase();

   vector<BatchFileset*>::const_iterator iter;
   for (iter = mFilesets.begin(); iter != mFilesets.end(); iter++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = *iter;
      if (pFileset != NULL)
      {
         string currentName = pFileset->getName();
         if (currentName == filesetName)
         {
            currentFile = pFileset->getCurrentFile().c_str();
            break;
         }
      }
   }
}

bool BatchWizard::isComplete() const
{
   bool bComplete = true;
   if (mpRepeatFileset != NULL)
   {
      bComplete = mpRepeatFileset->isComplete();
   }

   return bComplete;
}

bool BatchWizard::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }
   XMLWriter& writer = *pXml;
   writer.pushAddPoint(writer.addElement("wizard"));
   writer.addAttr("filename", mWizardFilename);

   string repeatFileset = "";
   if (isRepeating(repeatFileset))
   {
      writer.addAttr("repeat", repeatFileset);
   }

   if (doesCleanup())
   {
      writer.addAttr("cleanup", "true");
   }

   //Filesets
   for (vector<BatchFileset*>::const_iterator filesetIter = mFilesets.begin();
        filesetIter != mFilesets.end(); ++filesetIter)
   {
      BatchFileset* pFileset = *filesetIter;
      writer.pushAddPoint(writer.addElement("fileset"));
      writer.addAttr("name", pFileset->getName());
      writer.addAttr("dir", pFileset->getDirectory());
      multimap<string, string> criteria = pFileset->getFilesetRequirements();
      for (multimap<string, string>::iterator criteriaIter = criteria.begin();
           criteriaIter != criteria.end();
           criteriaIter++)
      {
         string type = criteriaIter->first;
         string pattern = criteriaIter->second;
         writer.pushAddPoint(writer.addElement(type.c_str()));
         writer.addAttr("name", pattern);
         writer.popAddPoint();
      }
      writer.popAddPoint();
   }

   //Wizards
   for (vector<Value*>::const_iterator valueIter = mInputValues.begin();
        valueIter != mInputValues.end(); ++valueIter)
   {
      Value* pValue = *valueIter;
      writer.pushAddPoint(writer.addElement("parameter"));
      writer.addAttr("item", pValue->getItemName());
      writer.addAttr("name", pValue->getNodeName());
      writer.addAttr("type", pValue->getNodeType());
      writer.pushAddPoint(writer.addElement("value"));
      writer.addText(pValue->getValue().toXmlString());
      writer.popAddPoint();
      writer.popAddPoint();
   }
   writer.popAddPoint(); 
   return true;
}

bool BatchWizard::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}
