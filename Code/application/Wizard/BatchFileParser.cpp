/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "BatchFileParser.h"
#include "StringUtilities.h"

using namespace std;

BatchFileParser::BatchFileParser()
{
   mCurrentWizard = 0;

   mpDocument = NULL;
   mpDocument = new QDomDocument();
}

BatchFileParser::~BatchFileParser() 
{
   delete mpDocument;
}

bool BatchFileParser::setFile(const string& filename)
{
   if (filename.empty() == true)
   {
      mErrorMessage = "The filename is empty!";
      return false;
   }

   QFileInfo fileInfo(QString::fromStdString(filename));

   bool bFile = false;
   bFile = fileInfo.isFile();
   if (bFile == false)
   {
      mErrorMessage = "The filename is not a valid file!";
      return false;
   }

   if (mpDocument == NULL)
   {
      mErrorMessage = "The XML file parser is invalid!";
      return false;
   }

   // Loading the XML file
   QFile file(QString::fromStdString(filename));

   bool bSuccess = false;
   bSuccess = mpDocument->setContent(&file);
   if (bSuccess == false)
   {
      mErrorMessage = "The file: " + filename + " is not a valid XML file!";
      return false;
   }

   bSuccess = parseFilesets();
   return bSuccess;
}

void BatchFileParser::getFileSets(vector<BatchFileset*>& filesets) const
{
   filesets.clear();

   map<string, BatchFileset*>::const_iterator iter;
   iter = mFilesets.begin();
   while (iter != mFilesets.end())
   {
      BatchFileset* pFileset = NULL;
      pFileset = iter->second;
      if (pFileset != NULL)
      {
         filesets.push_back(pFileset);
      }

      iter++;
   }
}

const string& BatchFileParser::getError() const
{
   return mErrorMessage;
}

BatchWizard* BatchFileParser::read()
{
   if (mpDocument == NULL)
   {
      return NULL;
   }

   // Get the list of nodes with the wizard tag
   QDomNodeList wizardList = mpDocument->elementsByTagName("wizard");
   if (wizardList.length() == 0)
   {
      mErrorMessage = "The <wizard> tag is not defined in the XML batch file!";
      return NULL;
   }

   unsigned int numberOfWizards = wizardList.length();
   if (mCurrentWizard >= numberOfWizards)
   {
      return NULL;
   }

   BatchWizard* pBatchWizard = NULL;
   pBatchWizard = new BatchWizard(); 
   if (pBatchWizard == NULL)
   {
      return NULL;
   }

   QDomNode wizardNode = wizardList.item(mCurrentWizard);
   QDomNamedNodeMap attrs = wizardNode.attributes();

   // Filename - required
   if (attrs.namedItem("filename").isNull() == false)
   {
      string text = ((attrs.namedItem("filename")).nodeValue()).toStdString();
      pBatchWizard->setWizardFilename(text);
   }
   else
   {
      mErrorMessage = "The wizard filename was not found while parsing the XML batch file!";
      return NULL;
   }

   // Cleanup - not required
   if (attrs.namedItem("cleanup").isNull() == false)
   {
      string text = ((attrs.namedItem("cleanup")).nodeValue()).toStdString();
      bool cleanup = StringUtilities::fromXmlString<bool>(text);
      pBatchWizard->setCleanup(true);
   }

   // Repeat - not required
   if (attrs.namedItem("repeat").isNull() == false)
   {
      string repeatName = ((attrs.namedItem("repeat")).nodeValue()).toStdString();

      map<string, BatchFileset*>::iterator iter;
      iter = mFilesets.find(repeatName);
      if (iter != mFilesets.end())
      {
         pBatchWizard->setRepeatFileset(iter->second);
      }
   }

   // Input parameters
   QDomNodeList parameters = wizardNode.childNodes();

   for (unsigned int node = 0; node < parameters.length(); node++)
   {
      QDomNode parameterNode = parameters.item(node);
      if (parameterNode.nodeName() == "parameter")
      {
         QDomNamedNodeMap inputAttr = (parameters.item(node)).attributes();

         QDomNode item = inputAttr.namedItem("item");
         QDomNode name = inputAttr.namedItem("name");
         QDomNode type = inputAttr.namedItem("type");

         string itemString = "";
         string nameString = "";
         string typeString = "";
         string valueString = "";

         if (item.isNull() == false)
         {
            itemString = item.nodeValue().toStdString();
         }

         if (name.isNull() == false)
         {
            nameString = name.nodeValue().toStdString();
         }

         if (type.isNull() == false)
         {
            typeString = type.nodeValue().toStdString();
         }

         QDomElement valueElement = parameterNode.firstChildElement("value");
         if (!valueElement.isNull())
         {
            valueString = valueElement.text().toStdString();
         }

         string variantType = typeString;
         if (typeString == "File set")
         {
            variantType = "string";
         }

         DataVariant valueData;
         valueData.fromXmlString(variantType, valueString);
         pBatchWizard->setInputValue(itemString, nameString, typeString, valueData);

         if ((valueString.empty() == false) && (typeString == "File set"))
         {
            map<string, BatchFileset*>::iterator iter = mFilesets.find(valueString);
            if (iter != mFilesets.end())
            {
               pBatchWizard->addFileset(iter->second);
            }
         }
      }
   }

   mCurrentWizard++;
   return pBatchWizard;
}

bool BatchFileParser::parseFilesets() 
{
   if (mpDocument == NULL)
   {
      return false;
   }

   // Parse the file set portion of the XML file.
   QDomNodeList filesetList = mpDocument->elementsByTagName("fileset");

   for (unsigned int node = 0; node < filesetList.length(); node++)
   {
      BatchFileset* pFileset = NULL;
      pFileset = new BatchFileset();
      if (pFileset != NULL)
      {
         QDomNamedNodeMap attributes = (filesetList.item(node)).attributes();
         string filesetName = "";

         // Name - required
         QDomNode name = attributes.namedItem("name");
         if (name.isNull() == false)
         {
            filesetName = name.nodeValue().toStdString();
            pFileset->setName(filesetName);
         }
      
         if (filesetName.empty() == true)
         {
            mErrorMessage = "One of the file sets does not have a name!";
            return false;
         }

         // Directory - required and it MUST be the full path name 
         QDomNode dir = attributes.namedItem("dir");
         if (dir.isNull() == false)
         {
            pFileset->setDirectory(dir.nodeValue().toStdString());
         }
         else
         {
            mErrorMessage = "A directory was not found for the '" + filesetName + "' file set!";
            return false;
         }

         // Parse the <include> and <exclude> tags
         QDomNodeList cludes = (filesetList.item(node)).childNodes();

         for (unsigned int x = 0; x < cludes.length(); x++)
         {
            // Include Tag
            if (cludes.item(x).nodeName() == "include")
            {
               QDomNamedNodeMap  includeAttr = (cludes.item(x)).attributes();

               // If a name and a value do exist then this is a singular input
               if (includeAttr.namedItem("name").isNull() == false)
               {                                 
                  pFileset->addFilesetRequirement(BatchFileset::INCLUDE,
                     includeAttr.namedItem("name").nodeValue().toStdString());
               }
            }

            // Exclude Tag
            else if (cludes.item(x).nodeName() == "exclude")
            {
               QDomNamedNodeMap  excludeAttr = (cludes.item(x)).attributes();
               // If a name and a value do exist then this is a singular input

               if (excludeAttr.namedItem("name").isNull() == false)
               {
                  pFileset->addFilesetRequirement(BatchFileset::EXCLUDE,
                     excludeAttr.namedItem("name").nodeValue().toStdString());
               }
            }
         }

         mFilesets.insert(pair<string, BatchFileset*>(filesetName, pFileset));
      }
   }

   return true;
}
