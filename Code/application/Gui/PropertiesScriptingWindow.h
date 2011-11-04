/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESSCRIPTINGWINDOW_H
#define PROPERTIESSCRIPTINGWINDOW_H

#include <QtGui/QFont>
#include <QtGui/QSpinBox>

#include "LabeledSectionGroup.h"

#include <string>

class CustomColorButton;
class ScriptingWindow;
class SessionItem;

class PropertiesScriptingWindow : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesScriptingWindow();
   ~PropertiesScriptingWindow();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

   static const std::string& getName();
   static const std::string& getPropertiesName();
   static const std::string& getDescription();
   static const std::string& getShortDescription();
   static const std::string& getCreator();
   static const std::string& getCopyright();
   static const std::string& getVersion();
   static const std::string& getDescriptorId();
   static bool isProduction();

protected slots:
   void setCommandFont();
   void setOutputFont();
   void setErrorFont();

private:
   PropertiesScriptingWindow(const PropertiesScriptingWindow& rhs);
   PropertiesScriptingWindow& operator=(const PropertiesScriptingWindow& rhs);
   ScriptingWindow* mpScriptingWindow;

   // General
   QSpinBox* mpScrollSpin;

   // Input text
   QFont mCommandFont;

   // Output text
   CustomColorButton* mpOutputColorButton;
   QFont mOutputFont;
   CustomColorButton* mpErrorColorButton;
   QFont mErrorFont;
};

#endif
