/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONITEMIMP_H
#define SESSIONITEMIMP_H

#include <QtCore/QMetaType>

#include <list>
#include <string>
#include <vector>

#include "ContextMenuAction.h"
#include "Serializable.h"
#include "SerializableImp.h"

class QIcon;
class SessionItem;
class SessionItemSerializer;
class SessionItemDeserializer;

Q_DECLARE_METATYPE(SessionItem*)

class SessionItemId
{
public:
   SessionItemId(const std::string &id) : mId(id) {}
   std::string mId;
};

class SessionItemImp : public Serializable
{
public:
   SessionItemImp(const std::string& id);
   SessionItemImp(const std::string& id, const std::string& name);
   virtual ~SessionItemImp();

   SessionItemImp& operator= (const SessionItemImp& sessionItem);

   const std::string& getId() const;
   static std::string generateUniqueId();

   virtual const QIcon& getIcon() const;
   virtual const std::string& getName() const;
   virtual const std::string& getDisplayName() const;
   virtual const std::string& getDisplayText() const;
   virtual std::list<ContextMenuAction> getContextMenuActions() const;
   bool hasFilenameDisplay() const;
   virtual std::vector<std::string> getPropertiesPages() const;

   virtual bool isValidSessionSaveItem() const;
   virtual bool serialize(SessionItemSerializer &serializer) const;
   virtual bool deserialize(SessionItemDeserializer &deserializer);
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

   virtual bool canRename() const;
   virtual bool rename(const std::string &newName, std::string &errorMessage);

protected:
   virtual bool setId(const SessionItemId& id);
   virtual void setValidSessionSaveItem(bool isValid);
   virtual void setIcon(const QIcon& icon);
   virtual void setName(const std::string& name);
   virtual void setDisplayName(const std::string& displayName);
   virtual void setDisplayText(const std::string& displayText);
   virtual void addContextMenuAction(const ContextMenuAction& menuAction);
   virtual void setContextMenuActions(const std::list<ContextMenuAction>& actions);
   void setFilenameDisplay(bool bFilenameDisplay);
   virtual void addPropertiesPage(const std::string& plugInName);
   virtual void removePropertiesPage(const std::string& plugInName);
   virtual void setPropertiesPages(const std::vector<std::string>& plugInNames);

private:
   void updateFilenameDisplay();
   void updateSessionExplorer();

   std::string mId;
   QIcon* mpIcon;
   std::string mName;
   std::string mDisplayName;
   std::string mDisplayText;
   std::list<ContextMenuAction> mMenuActions;
   bool mFilenameDisplay;
   std::vector<std::string> mPropertiesPages;
   mutable bool mIdLocked;
   bool mValidSessionSaveItem;
};

#define SESSIONITEMADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES \
   , public SessionItemExt1

#define SESSIONITEMACCESSOR_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   const std::string& getId() const \
   { \
      return impClass::getId(); \
   } \
   const QIcon& getIcon() const \
   { \
      return impClass::getIcon(); \
   } \
   const std::string& getName() const \
   { \
      return impClass::getName(); \
   } \
   const std::string& getDisplayName() const \
   { \
      return impClass::getDisplayName(); \
   } \
   const std::string& getDisplayText() const \
   { \
      return impClass::getDisplayText(); \
   } \
   std::list<ContextMenuAction> getContextMenuActions() const \
   { \
      return impClass::getContextMenuActions(); \
   } \
   bool hasFilenameDisplay() const \
   { \
      return impClass::hasFilenameDisplay(); \
   } \
   std::vector<std::string> getPropertiesPages() const \
   { \
      return impClass::getPropertiesPages(); \
   } \
   bool isValidSessionSaveItem() const \
   { \
      return impClass::isValidSessionSaveItem(); \
   }

#define SESSIONITEMADAPTER_METHODS(impClass) \
   SESSIONITEMACCESSOR_METHODS(impClass) \
   bool serialize(SessionItemSerializer &serializer) const \
   { \
      return impClass::serialize(serializer); \
   } \
   bool deserialize(SessionItemDeserializer &deserializer) \
   { \
      return impClass::deserialize(deserializer); \
   }

#endif