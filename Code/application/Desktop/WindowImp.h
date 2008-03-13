/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WINDOWIMP_H
#define WINDOWIMP_H

#include <QtCore/QString>

#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "Window.h"
#include "xmlwriter.h"

#include "XercesIncludes.h"

#include <string>

class QDragEnterEvent;
class QDropEvent;
class SessionItemDeserializer;
class SessionItemSerializer;

class WindowImp : public SessionItemImp, public SubjectImp
{
public:
   WindowImp(const std::string& id, const std::string& windowName);
   ~WindowImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void setName(const std::string& windowName);
   virtual WindowType getWindowType() const = 0;

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer); 
   virtual bool toXml(XMLWriter* pXml) const;

   virtual void enableSessionItemDrops(Window::SessionItemDropFilter *pFilter = NULL);

protected:
   virtual void dragEnterEvent(QDragEnterEvent *pEvent);
   virtual void dropEvent(QDropEvent *pEvent);

private:
   std::vector<Window::SessionItemDropFilter*> mSessionItemDropFilters;
   bool mAcceptAllSessionItemDrops;
};

/**
 *  This macro should be used by WindowImp subclasses which actually inherit from Qt.
 *  This will change the member protection mode to protected.
 */
#define WINDOWIMPDROP_METHODS(windowImpClass) \
   public: \
   void enableSessionItemDrops(Window::SessionItemDropFilter *pFilter = NULL) \
   { \
      setAcceptDrops(true); \
      windowImpClass::enableSessionItemDrops(pFilter); \
   } \
   protected: \
   void dragEnterEvent(QDragEnterEvent *pEvent) \
   { \
      windowImpClass::dragEnterEvent(pEvent); \
   } \
   void dropEvent(QDropEvent *pEvent) \
   { \
      windowImpClass::dropEvent(pEvent); \
   }

#define WINDOWADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   void setName(const std::string& windowName) \
   { \
      return impClass::setName(windowName); \
   } \
   WindowType getWindowType() const \
   { \
      return impClass::getWindowType(); \
   } \
   void enableSessionItemDrops(SessionItemDropFilter *pFilter = NULL) \
   { \
      return impClass::enableSessionItemDrops(pFilter); \
   }

#endif
