/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETTABLESESSIONITEMADAPTER_H
#define SETTABLESESSIONITEMADAPTER_H

#include "SettableSessionItem.h"
#include "SessionItemImp.h"

#define SETTABLESESSIONITEMADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES

#define SETTABLESESSIONITEMADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   bool setId(const SessionItemId& id) \
   { \
      return impClass::setId(id); \
   } \
   void setIcon(const QIcon& icon) \
   { \
      impClass::setIcon(icon); \
   } \
   void setName(const std::string& name) \
   { \
      impClass::setName(name); \
   } \
   void setDisplayName(const std::string& displayName) \
   { \
      impClass::setDisplayName(displayName); \
   } \
   void setDisplayText(const std::string& displayText) \
   { \
      impClass::setDisplayText(displayText); \
   } \
   void addContextMenuAction(const ContextMenuAction& menuAction) \
   { \
      impClass::addContextMenuAction(menuAction); \
   } \
   void setContextMenuActions(const std::list<ContextMenuAction>& actions) \
   { \
      impClass::setContextMenuActions(actions); \
   } \
   void setFilenameDisplay(bool bFilenameDisplay) \
   { \
      impClass::setFilenameDisplay(bFilenameDisplay); \
   } \
   void addPropertiesPage(const std::string& plugInName) \
   { \
      impClass::addPropertiesPage(plugInName); \
   } \
   void removePropertiesPage(const std::string& plugInName) \
   { \
      impClass::removePropertiesPage(plugInName); \
   } \
   void setPropertiesPages(const std::vector<std::string>& plugInNames) \
   { \
      impClass::setPropertiesPages(plugInNames); \
   } \
   void setValidSessionSaveItem(bool isValid) \
   { \
      impClass::setValidSessionSaveItem(isValid); \
   }

class SettableSessionItemAdapter : public SettableSessionItem, public SessionItemImp
   SETTABLESESSIONITEMADAPTEREXTENSION_CLASSES
{
public:
   SettableSessionItemAdapter(const std::string& id);
   ~SettableSessionItemAdapter();

   SETTABLESESSIONITEMADAPTER_METHODS(SessionItemImp)
};

#endif