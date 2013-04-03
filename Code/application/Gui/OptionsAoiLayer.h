/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSAOILAYER_H
#define OPTIONSAOILAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class GraphicObjectTypeButton;
class QCheckBox;
class SymbolTypeButton;

class OptionsAoiLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsAoiLayer();
   virtual ~OptionsAoiLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "AOI Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/AOI";
      return var;
   }

   static const std::string& getDescription()
   {
      return OptionsAoiLayer::getShortDescription();
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display AOI layer related options for the application";
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
      static std::string var = "{9EB3635E-5BCD-4d0c-8D24-97AB310F61D5}";
      return var;
   }

private:
   OptionsAoiLayer(const OptionsAoiLayer& rhs);
   OptionsAoiLayer& operator=(const OptionsAoiLayer& rhs);

   SymbolTypeButton* mpMarkerSymbol;
   CustomColorButton* mpMarkerColor;
   QCheckBox* mpAutoColor;
   GraphicObjectTypeButton* mpSelectionTool;
};

#endif
