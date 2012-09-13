/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSFILELOCATIONS_H
#define OPTIONSFILELOCATIONS_H

#include "AppVersion.h"
#include "LabeledSectionGroup.h"

#include <string>
#include <vector>

class CustomTreeWidget;
class FileBrowser;

class OptionsFileLocations : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsFileLocations();
   virtual ~OptionsFileLocations();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "File Location Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Session/File Locations";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display file location options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display file location options for the application";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT_MSG;
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = APP_VERSION_NUMBER;
      return var;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{8E585626-7C31-45ac-A543-146CEA495FD0}";
      return var;
   }

private:
   OptionsFileLocations(const OptionsFileLocations& rhs);
   OptionsFileLocations& operator=(const OptionsFileLocations& rhs);

   class FileLocationDescriptor
   {
   public:
      FileLocationDescriptor(const std::string& text, const std::string& key,
         FileBrowser* pFileBrowser = NULL, const std::string& argumentKey = std::string()) :
         mText(text),
         mKey(key),
         mpFileBrowser(pFileBrowser),
         mArgumentKey(argumentKey)
      {}

      ~FileLocationDescriptor() {}

      const std::string& getText() const
      {
         return mText;
      }

      const std::string& getKey() const
      {
         return mKey;
      }

      FileBrowser* getFileBrowser() const
      {
         return mpFileBrowser;
      }

      const std::string& getArgumentKey() const
      {
         return mArgumentKey;
      }

   private:
      std::string mText;
      std::string mKey;
      FileBrowser* mpFileBrowser;
      std::string mArgumentKey;
   };

   static const QString sBookmarkListSentinal;
   std::vector<FileLocationDescriptor> mFileLocations;
   std::string mWizardPath;
   CustomTreeWidget* mpPathTree;
   CustomTreeWidget* mpFileTree;
   void applyChanges(CustomTreeWidget* pTree);
};

#endif
