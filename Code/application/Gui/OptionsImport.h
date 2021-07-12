/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSIMPORT_H__
#define OPTIONSIMPORT_H__

#include <QtWidgets/QWidget>

#include "AppVersion.h"

class QCheckBox;

class OptionsImport : public QWidget
{
   Q_OBJECT

public:
   OptionsImport();
   ~OptionsImport();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Import Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Import";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display import options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display import options for the application";
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
      static std::string var = "{0E6C0AB7-0004-4665-88BE-F9E123610714}";
      return var;
   }

private:
   OptionsImport(const OptionsImport& rhs);
   OptionsImport& operator=(const OptionsImport& rhs);
   QCheckBox* mpBackgroundImport;
};

#endif

