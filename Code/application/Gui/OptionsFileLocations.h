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

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomTreeWidget;

class OptionsFileLocations : public QWidget
{
   Q_OBJECT

public:
   OptionsFileLocations();
   ~OptionsFileLocations();

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
   static const QString sBookmarkListSentinal;
   std::vector<std::pair<std::string, std::string> > mFileLocations;
   std::string mPlugInPath;
   std::string mWizardPath;
   CustomTreeWidget* mpFileTree;
};

#endif
