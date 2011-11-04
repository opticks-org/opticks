/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSGCPLAYER_H
#define OPTIONSGCPLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomColorButton;
class GcpSymbolButton;
class QSpinBox;

class OptionsGcpLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsGcpLayer();
   ~OptionsGcpLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Gcp Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Gcp";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display Gcp layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display Gcp layer related related options for the application";
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
      static std::string var = "{AA46CDA0-3B73-4315-9A32-A11A78828F91}";
      return var;
   }

private:
   OptionsGcpLayer(const OptionsGcpLayer& rhs);
   OptionsGcpLayer& operator=(const OptionsGcpLayer& rhs);
   QSpinBox* mpMarkerSize;
   GcpSymbolButton* mpMarkerSymbol;
   CustomColorButton* mpMarkerColor;
};

#endif
