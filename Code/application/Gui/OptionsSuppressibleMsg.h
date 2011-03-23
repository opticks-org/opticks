/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSUPPRESSIBLEMESSAGES_H
#define OPTIONSSUPPRESSIBLEMESSAGES_H

#include <QtGui/QWidget>
#include "AppVersion.h"

#include <string>
#include <utility>
#include <vector>

class QCheckBox;

class OptionsSuppressibleMsg : public QWidget
{
   Q_OBJECT

public:
   OptionsSuppressibleMsg();
   ~OptionsSuppressibleMsg();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Suppressible Dialog Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Optional Messages/Opticks";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display suppressible dialog options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display suppressible dialog options for the application";
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
      static std::string var = "{891394DA-03C4-4591-A34B-F5D1D50719F0}";
      return var;
   }

private:
   std::vector<std::pair<QCheckBox*, std::string> > mCheckBoxes;
};

#endif