/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSVIEW_H
#define OPTIONSVIEW_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QRadioButton;

class OptionsView : public QWidget
{
   Q_OBJECT

public:
   OptionsView();
   ~OptionsView();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "View Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Workspace/All";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display view related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display view related options for the application";
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
      static std::string var = "{7AE43763-076B-4acf-BEA4-77AC70145B48}";
      return var;
   }

private:
   QRadioButton* mpFixedSizeRadio;
   QRadioButton* mpMaximizedRadio;
   QRadioButton* mpPercentageRadio;
   QRadioButton* mpZoomToFitRadio;
   QRadioButton* mpZoomPercentRadio;
   QSpinBox* mpHeightSpin;
   QSpinBox* mpWidthSpin;
   QSpinBox* mpPercentageSpin;
   QSpinBox* mpZoomPercentSpin;
   CustomColorButton* mpBackgroundColor;
   QComboBox* mpOriginCombo;
   QCheckBox* mpConfirmClose;
};

#endif
