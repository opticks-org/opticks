/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "NitfResource.h"

#include "FileResource.h"
#include "Progress.h"

#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/init/ossimInit.h>


ossimNitfFile* Nitf::OssimFileObject::obtainResource(const Args &args) const
{
   string fname = args.mFilename;
   if (args.mAccess.find("r") >= 0)
   {
      FileResource readTest(fname.c_str(), "r");

      if (readTest.get() == NULL)
      {
         fname = args.mFilename.c_str();
         for(size_t i = 0; i < fname.size(); i++)
         {
            fname[i] = static_cast<char>(toupper(fname[i]));
         }
         readTest = FileResource(fname.c_str(), "r");
      }

      if (readTest.get() == NULL)
      {
         fname = args.mFilename.c_str();
         for(size_t i = 0; i < fname.size(); i++)
         {
            fname[i] = static_cast<char>(tolower(fname[i]));
         }
         readTest = FileResource(fname.c_str(), "r");
      }

      if (readTest.get() == NULL)
      {
         return NULL;
      }
   }

   ossimNitfFile* pFile = new ossimNitfFile;
   if (pFile != NULL)
   {
      pFile->parseFile(fname.c_str());
   }
   return pFile;
}

void Nitf::OssimFileObject::releaseResource(const Args &args, ossimNitfFile* pFile) const
{
   delete pFile;
}

ossimImageHandler *Nitf::OssimImageHandlerObject::obtainResource(const Args &args) const
{
   ossimImageHandler *pHandler = NULL;
   if (args.mFilename.empty() == false)
   {
      pHandler = ossimImageHandlerRegistry::instance()->open(ossimString(args.mFilename.c_str()));
      if(!pHandler)
      {
         ossimInit::instance()->setPluginLoaderEnabledFlag(true);
         ossimInit::instance()->initialize();
         pHandler = static_cast<ossimImageHandler*>
            (ossimImageHandlerRegistry::instance()->createObject(ossimString("ossimGdalTileSource")));
         if (pHandler != NULL)
         {
            pHandler->open(ossimString(args.mFilename.c_str()));
         }
      }
   }

   return pHandler;
}

void Nitf::OssimImageHandlerObject::releaseResource(const Args &args, ossimImageHandler *pHandler)
{
   pHandler->close();
   delete pHandler;
}
