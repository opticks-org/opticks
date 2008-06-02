/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSPSEUDOCOLORLAYER_H
#define OPTIONSPSEUDOCOLORLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class SymbolTypeButton;

class OptionsPseudocolorLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsPseudocolorLayer();
   ~OptionsPseudocolorLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Pseudocolor Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Pseudocolor";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display Pseudocolor layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display Pseudocolor layer related related options for the application";
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
      static std::string var = "{1C06A8F1-ACE3-4262-80F6-4A2568A45E01}";
      return var;
   }

private:
   SymbolTypeButton* mpSymbolType;
};

#endif
