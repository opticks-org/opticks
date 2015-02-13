/*
 * The information in this file is
 * Copyright(c) 2015 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSPROPERTIESDIALOG_H
#define OPTIONSPROPERTIESDIALOG_H

#include <QtGui/QWidget>

#include "AppVersion.h"

class QSpinBox;

class OptionsPropertiesDialog : public QWidget
{
   Q_OBJECT

public:
   OptionsPropertiesDialog();
   ~OptionsPropertiesDialog();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Properties Dialog Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Properties Dialog";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display Properties Dialog related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display Properties Dialog related options for the application";
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
      static std::string var = "{2F6A4992-695C-4519-8030-A20B5EF452E4}";
      return var;
   }

private:
   OptionsPropertiesDialog(const OptionsPropertiesDialog& rhs);
   OptionsPropertiesDialog& operator=(const OptionsPropertiesDialog& rhs);
   QSpinBox* mpOpacity;
};

#endif
