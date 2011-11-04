/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSOVERVIEWWINDOW_H
#define OPTIONSOVERVIEWWINDOW_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class QSpinBox;

class OptionsOverviewWindow : public QWidget
{
   Q_OBJECT

public:
   OptionsOverviewWindow();
   ~OptionsOverviewWindow();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Overview Window Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Overview";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display overview window related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display overview window related related options for the application";
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
      static std::string var = "{F06D3A82-DA54-4f22-9380-EA500861C982}";
      return var;
   }

private:
   OptionsOverviewWindow(const OptionsOverviewWindow& rhs);
   OptionsOverviewWindow& operator=(const OptionsOverviewWindow& rhs);
   CustomColorButton* mpTrailColor;
   QSpinBox* mpTrailOpacity;
   QSpinBox* mpTrailThreshold;
};

#endif
