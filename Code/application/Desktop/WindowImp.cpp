/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Window.h"
#include "WindowImp.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Subject.h"
#include "xmlreader.h"
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>

XERCES_CPP_NAMESPACE_USE
using namespace std;

WindowImp::WindowImp(const string& id, const string& windowName) :
   SessionItemImp(id, windowName), mAcceptAllSessionItemDrops(false)
{
}

WindowImp::~WindowImp()
{
}

const string& WindowImp::getObjectType() const
{
   static string type("WindowImp");
   return type;
}

bool WindowImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Window"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void WindowImp::setName(const string& windowName)
{
   if (windowName.empty() == true)
   {
      return;
   }

   if (windowName != getName())
   {
      SessionItemImp::setName(windowName);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool WindowImp::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool WindowImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader xml(NULL, false);
   DOMNode* pRoot = deserializer.deserialize(xml, getObjectType().c_str());
   if (!fromXml(pRoot, XmlBase::VERSION))
   {
      return false;
   }
   return true;
}

bool WindowImp::toXml(XMLWriter* pXml) const
{
   if (!SessionItemImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("type", getObjectType());
   return true;
}

void WindowImp::enableSessionItemDrops(Window::SessionItemDropFilter *pFilter)
{
   if (pFilter == NULL)
   {
      mAcceptAllSessionItemDrops = true;
   }
   else
   {
      mSessionItemDropFilters.push_back(pFilter);
   }
}

void WindowImp::dragEnterEvent(QDragEnterEvent *pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   const QMimeData* pData = pEvent->mimeData();
   if (pData == NULL)
   {
      return;
   }

   if (pData->hasFormat("application/x-sessionitem-id") == true)
   {
      QByteArray encoded = pData->data("application/x-sessionitem-id");
      QDataStream stream(&encoded, QIODevice::ReadOnly);
      bool accepted = mAcceptAllSessionItemDrops;
      while (!accepted && !stream.atEnd())
      {
         QString id;
         stream >> id;
         SessionItem* pItem = Service<SessionManager>()->getSessionItem(id.toStdString());
         if (pItem != NULL)
         {
            for (vector<Window::SessionItemDropFilter*>::iterator filter = mSessionItemDropFilters.begin();
               !accepted && filter != mSessionItemDropFilters.end(); ++filter)
            {
               accepted = (*filter)->accept(pItem);
            }
         }
      }
      if (accepted)
      {
         pEvent->acceptProposedAction();
      }
   }
}

void WindowImp::dropEvent(QDropEvent *pEvent)
{
   if (pEvent == NULL)
   {
      return;
   }

   const QMimeData* pData = pEvent->mimeData();
   if (pData == NULL)
   {
      return;
   }

   if (pData->hasFormat("application/x-sessionitem-id") == true)
   {
      vector<SessionItem*> droppedItems;
      QByteArray encoded = pData->data("application/x-sessionitem-id");
      QDataStream stream(&encoded, QIODevice::ReadOnly);
      bool accepted = false;
      while (!stream.atEnd())
      {
         QString id;
         stream >> id;
         SessionItem* pItem = Service<SessionManager>()->getSessionItem(id.toStdString());
         if (pItem != NULL)
         {
            accepted = true;
            droppedItems.push_back(pItem);
            notify(SIGNAL_NAME(Window, SessionItemDropped), boost::any(pItem));
         }
      }
      notify(SIGNAL_NAME(Window, SessionItemsDropped), boost::any(droppedItems));
      if (accepted)
      {
         pEvent->acceptProposedAction();
      }
   }
}
