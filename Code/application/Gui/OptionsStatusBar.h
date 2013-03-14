/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSTATUSBAR_H
#define OPTIONSSTATUSBAR_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class QCheckBox;

class OptionsStatusBar : public QWidget
{
   Q_OBJECT

public:
   OptionsStatusBar();
   ~OptionsStatusBar();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Status Bar Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Status Bar";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display status bar related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display status bar related options for the application";
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
      static std::string var = "{2F6A4992-695C-4519-8030-A20B5EF452E1}";
      return var;
   }

private:
   OptionsStatusBar(const OptionsStatusBar& rhs);
   OptionsStatusBar& operator=(const OptionsStatusBar& rhs);
   QCheckBox* mpPixelCoordsCheck;
   QCheckBox* mpGeoCoordsCheck;
   QCheckBox* mpCubeValueCheck;
   QCheckBox* mpCubeValueUnitsCheck;
   QCheckBox* mpResultValueCheck;
   QCheckBox* mpRotationValueCheck;
   QCheckBox* mpElevationValueCheck;
};

#endif
