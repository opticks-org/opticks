/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSHORTCUTS_H
#define OPTIONSSHORTCUTS_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class CustomTreeWidget;
class DynamicObject;
class QTreeWidgetItem;

class OptionsShortcuts : public QWidget
{
   Q_OBJECT

public:
   OptionsShortcuts();
   ~OptionsShortcuts();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Keyboard Shortcut Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Session/Keyboard Shortcuts";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display keyboard shortcut options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display keyboard shortcut options for the application";
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
      static std::string var = "{C4706B1D-E941-4298-ACAF-B957E6A6A5E5}";
      return var;
   }

private:
   void shortcutsIntoGui(const DynamicObject* pCurObject, QTreeWidgetItem* pParent);
   void shortcutsFromGui(QTreeWidgetItem* pParent, DynamicObject* pCurObject) const; 

   CustomTreeWidget* mpShortcutTree;
};

#endif
