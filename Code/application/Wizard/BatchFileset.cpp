/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BatchFileset.h"
#include "ObjectFactory.h"
#include "FileFinder.h"

using namespace std;

BatchFileset::BatchFileset()
{
   mName = "";
   mDirectory = "";
   mpCurrentFile = NULL;
}

BatchFileset::BatchFileset(const string& name, const string& directory, const vector<BatchFile*>& fileset)
{
   mName = name;
   mDirectory = directory;
   mFiles = fileset;
   mpCurrentFile = NULL;
}

BatchFileset::~BatchFileset()
{
   for (unsigned int i = 0; i < mFiles.size(); i++)
   {
      delete (mFiles[i]);
   }
}

void BatchFileset::setName(const string& filesetName)
{
   if (filesetName != mName)
   {
      mName = filesetName;
   }
}

const string& BatchFileset::getName() const
{
   return mName;
}

void BatchFileset::setDirectory(const string& directory)
{
   if (directory != mDirectory)
   {
      mDirectory = directory;
   }
}

const string& BatchFileset::getDirectory() const
{
   return mDirectory;
}

void BatchFileset::addFile(BatchFile* pBatchFile)
{
   if (pBatchFile == NULL)
   {
      return;
   }

   bool bContains = false;

   vector<BatchFile*>::iterator iter;
   for (iter = mFiles.begin(); iter != mFiles.end(); iter++)
   {
      BatchFile* pCurrentFile = NULL;
      pCurrentFile = *iter;
      if (pCurrentFile == pBatchFile)
      {
         bContains = true;
         break;
      }
   }

   if (bContains == false)
   {
      mFiles.push_back(pBatchFile);
   }
}

void BatchFileset::addFiles(const vector<BatchFile*>& fileset)
{
   mFiles = fileset;
}

void BatchFileset::addFile(const string& filename)
{
   if (filename.empty() == true)
   {
      return;
   }

   // If the file already exists, do not add it
   if (doesFileExist(filename) == false)
   {
      BatchFile* pBatchFile = NULL;
      pBatchFile = new BatchFile(filename);
      if (pBatchFile != NULL)
      {
         addFile(pBatchFile);
      }
   }
}

void BatchFileset::addFilesetRequirement(BatchFileset::FilesetRequirementType eType,
   const string& requirement)
{
   switch (eType)
   {
      case INCLUDE:
         mFilesetReq.insert(pair<const string, string>(string("include"), requirement));
         break;

      case EXCLUDE:
         mFilesetReq.insert(pair<const string, string>(string("exclude"), requirement));
         break;

      default:
         break;
   }
}

void BatchFileset::clearFilesetRequirements()
{
   mFilesetReq.clear();
}

const multimap<string, string>& BatchFileset::getFilesetRequirements() const
{
   return mFilesetReq;
}

bool BatchFileset::doesFileExist(const string& filename) const
{
   if ((filename.empty() == true) || (mFiles.size() == 0))
   {
      return false;
   }

   for (unsigned int i = 0; i < mFiles.size(); i++)
   {
      if (mFiles[i]->getFileName() == filename)
      {
         return true;
      }
   }

   return false;
}

void BatchFileset::updateFileset(ObjectFactory* pObjFact) 
{
   if (pObjFact == NULL)
   {
      return;
   }

   FileFinder* pFileFinder = NULL;
   pFileFinder = reinterpret_cast<FileFinder*>(pObjFact->createObject("FileFinder"));
   if (pFileFinder == NULL)
   {
      return;
   }

   mFiles.clear();

   multimap<string, string>::iterator iter;
   for (iter = mFilesetReq.begin(); iter != mFilesetReq.end(); iter++)
   {
      string type = iter->first;
      string pattern = mDirectory + "/" + iter->second;

      // Check if the requirement is a specific file or contains a wild card
      bool bFile = false;

      int iPos = -1;
      iPos = pattern.find("*");
      if (iPos == -1)
      {
         iPos = pattern.find("?");
         if (iPos == -1)
         {
            bFile = true;
         }
      }

      if (bFile == false)
      {
         if (pFileFinder->findFile(mDirectory, iter->second) == true)
         {
            while (pFileFinder->findNextFile() != NULL)
            {
               string filePath = "";
               pFileFinder->getFullPath(filePath);

               if (type == "include")
               {
                  addFile(filePath);
               }
               else if (type == "exclude")
               {
                  vector<BatchFile*>::iterator fItr = mFiles.begin();
                  for (unsigned int iIndex = 0; iIndex < mFiles.size(); iIndex++, fItr++)
                  {
                     string batchFile = mFiles[iIndex]->getFileName().c_str();
                     if (batchFile == filePath)
                     {
                        mFiles.erase(fItr);
                     }
                  }
               }
            }
         }
      }
      else
      {
         if (type == "include")
         {
            addFile(pattern);
         }
         else if (type == "exclude")
         {
            vector<BatchFile*>::iterator fItr = mFiles.begin();
            for (unsigned int iIndex = 0; iIndex < mFiles.size(); iIndex++, fItr++)
            {
               string batchFile = mFiles[iIndex]->getFileName().c_str();
               if (batchFile == pattern)
               {
                  mFiles.erase(fItr);
               }
            }
         }
      }
   }

   pObjFact->destroyObject(pFileFinder, "FileFinder");
}

string BatchFileset::getFirstFile() 
{
   if (mFiles.size() == 0)
   {
      mpCurrentFile = NULL;
   }
   else
   {
      mpCurrentFile = *mFiles.begin();
   }

   return getCurrentFile();
}

string BatchFileset::getCurrentFile() 
{
   string currentFile = "";
   if (mpCurrentFile != NULL)
   {
      currentFile = mpCurrentFile->getFileName();
   }

   return currentFile;
}

string BatchFileset::getNextFile() 
{
   if (mpCurrentFile != NULL)
   {
      mpCurrentFile->setUsed(true);
      mpCurrentFile = NULL;

      // Go through the list and set the next current file.
      vector<BatchFile*>::iterator iter;
      for (iter = mFiles.begin(); iter != mFiles.end(); iter++)
      {
         BatchFile* pFile = NULL;
         pFile = *iter;
         if ((pFile != NULL) && (pFile->isUsed() == false))
         {
            mpCurrentFile = pFile;
            break;
         }
      }
   }

   return getCurrentFile();
}

bool BatchFileset::isComplete() const
{
   bool bComplete = true;
   if (mpCurrentFile != NULL)
   {
      bComplete = false;
   }

   return bComplete;
}
