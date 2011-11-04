/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSESSION_H
#define OPTIONSSESSION_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class QCheckBox;
class QComboBox;
class QSpinBox;

class OptionsSession : public QWidget
{
   Q_OBJECT

public:
   OptionsSession();
   ~OptionsSession();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Session Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Session/Save";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display session related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display session related options for the application";
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
      static std::string var = "{0180A593-3693-42a1-B1FB-1DD751D76F37}";
      return var;
   }

private:
   OptionsSession(const OptionsSession& rhs);
   OptionsSession& operator=(const OptionsSession& rhs);
   QComboBox* mpSaveCombo;
   QCheckBox* mpAutoSaveEnabledCheck;
   QSpinBox* mpAutoSaveIntervalSpin;
};

#endif
