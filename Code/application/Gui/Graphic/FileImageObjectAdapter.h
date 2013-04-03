/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEIMAGEOBJECTADAPTER_H
#define FILEIMAGEOBJECTADAPTER_H

#include "FileImageObject.h"
#include "FileImageObjectImp.h"

class FileImageObjectAdapter : public FileImageObject, public FileImageObjectImp FILEIMAGEOBJECTADAPTEREXTENSION_CLASSES
{
public:
   FileImageObjectAdapter(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer,
                          LocationType pixelCoord) :
      FileImageObjectImp(id, type, pLayer, pixelCoord)
   {
   }

   ~FileImageObjectAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   // TypeAwareObject
   const std::string& getObjectType() const
   {
      static std::string type("FileImageObjectAdapter");
      return type;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "FileImageObject"))
      {
         return true;
      }

      return FileImageObjectImp::isKindOf(className);
   }

   FILEIMAGEOBJECTADAPTER_METHODS(FileImageObjectImp)

private:
   FileImageObjectAdapter(const FileImageObjectAdapter& rhs);
   FileImageObjectAdapter& operator=(const FileImageObjectAdapter& rhs);
};

#endif
