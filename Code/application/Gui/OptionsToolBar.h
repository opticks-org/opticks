/*
 * The information in this file is
 * Copyright(c) 2015 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTOOLBAR_H
#define OPTIONSTOOLBAR_H

#include <QtGui/QWidget>

#include "AppVersion.h"

class QSpinBox;

class OptionsToolBar : public QWidget
{
   Q_OBJECT

public:
   OptionsToolBar();
   ~OptionsToolBar();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Tool Bar Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Tool Bars";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display tool bar related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display tool bar related options for the application";
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
      static std::string var = "{2F6A4992-695C-4519-8030-A20B5EF452E2}";
      return var;
   }

private:
   OptionsToolBar(const OptionsToolBar& rhs);
   OptionsToolBar& operator=(const OptionsToolBar& rhs);
   QSpinBox* mpOpacity;
};

#endif
