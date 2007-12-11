/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSAMPLE_H
#define OPTIONSSAMPLE_H

#include <QtGui/QWidget>

#include "AppVersion.h"
#include "ConfigurationSettings.h"

class QCheckBox;
class QLineEdit;
class QSpinBox;

class OptionsSample : public QWidget
{
   Q_OBJECT

public:
   OptionsSample();
   ~OptionsSample();

   SETTING(TestTextSetting, Plug-In Sampler, std::string, "");
   SETTING(TestIntSetting, Plug-In Sampler, unsigned int, 0);
   SETTING(TestBoolSetting, Plug-In Sampler, bool, false);

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Plug-in Sampler example options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Plug-in Sampler/Example Options";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Plug-in Sampler example options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Plug-in Sampler example options";
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
      return false;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{D6B1E4DC-55FB-4bdf-91F4-AFDE0C7EBE0D}";
      return var;
   }

private:
   QLineEdit* mpTextSetting;
   QSpinBox* mpIntSetting;
   QCheckBox* mpBoolSetting;
};

#endif
