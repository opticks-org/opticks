/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTIEPOINTLAYER_H
#define OPTIONSTIEPOINTLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class QCheckBox;
class QSpinBox;

class OptionsTiePointLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsTiePointLayer();
   ~OptionsTiePointLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Tie Point Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Tie Point";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display Tie Point layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display Tie Point layer related related options for the application";
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
      static std::string var = "{7C61D47D-A822-4cc5-845C-FB8D27D1D2D9}";
      return var;
   }

private:
   OptionsTiePointLayer(const OptionsTiePointLayer& rhs);
   OptionsTiePointLayer& operator=(const OptionsTiePointLayer& rhs);
   QSpinBox* mpSymbolSize;
   QCheckBox* mpLabelsEnabled;
   CustomColorButton* mpColor;
   QCheckBox* mpAutoColor;
};

#endif
