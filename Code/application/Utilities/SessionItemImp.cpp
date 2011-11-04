/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtGui/QIcon>

#include "AppVerify.h"
#include "DesktopServices.h"
#include "SessionExplorerImp.h"
#include "SessionItemImp.h"
#include "XercesIncludes.h"

#include <algorithm>

using namespace std;
XERCES_CPP_NAMESPACE_USE

SessionItemImp::SessionItemImp(const string& id) :
   mId(id),
   mpIcon(new QIcon()),
   mFilenameDisplay(true),
   mIdLocked(false),
   mValidSessionSaveItem(true)
{
   VERIFYNRV(mId.empty() == false);
}

SessionItemImp::SessionItemImp(const string& id, const string& name) :
   mId(id),
   mpIcon(new QIcon()),
   mName(name),
   mFilenameDisplay(true),
   mIdLocked(false),
   mValidSessionSaveItem(true)
{
   VERIFYNRV(mId.empty() == false);
   updateFilenameDisplay();
}

SessionItemImp::~SessionItemImp()
{
   delete mpIcon;
}

SessionItemImp& SessionItemImp::operator= (const SessionItemImp& sessionItem)
{
   if (&sessionItem != this)
   {
      setIcon(*sessionItem.mpIcon);
      setName(sessionItem.mName);
      setDisplayName(sessionItem.mDisplayName);
      setDisplayText(sessionItem.mDisplayText);
      setFilenameDisplay(sessionItem.mFilenameDisplay);
      setPropertiesPages(sessionItem.mPropertiesPages);
      setValidSessionSaveItem(sessionItem.mValidSessionSaveItem);
      mIdLocked = sessionItem.mIdLocked;
   }

   return *this;
}

bool SessionItemImp::setId(const SessionItemId &id)
{
   if (!mIdLocked)
   {
      mId = id.mId;
      return true;
   }
   return false;
}

const string& SessionItemImp::getId() const
{
   mIdLocked = true;
   return mId;
}

string SessionItemImp::generateUniqueId()
{
   QUuid id = QUuid::createUuid();
   QString strId = id.toString();

   VERIFYRV(strId.isEmpty() == false, string());
   return strId.toStdString();
}

const QIcon& SessionItemImp::getIcon() const
{
   return *mpIcon;
}

const string& SessionItemImp::getName() const
{
   return mName;
}

const string& SessionItemImp::getDisplayName(bool fullName) const
{
   if ((mDisplayName.empty() == true) && (fullName == true))
   {
      return getName();
   }

   return mDisplayName;
}

const string& SessionItemImp::getDisplayText() const
{
   return mDisplayText;
}

list<ContextMenuAction> SessionItemImp::getContextMenuActions() const
{
   return mMenuActions;
}

bool SessionItemImp::hasFilenameDisplay() const
{
   return mFilenameDisplay;
}

vector<string> SessionItemImp::getPropertiesPages() const
{
   return mPropertiesPages;
}

//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : These should eventually be pure virtual, " \
//   "once all of the implementations are written (TJOHNSON)")
bool SessionItemImp::serialize(SessionItemSerializer &serializer) const
{
   return false;
}

bool SessionItemImp::deserialize(SessionItemDeserializer &deserializer)
{
   return false;
}

bool SessionItemImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", mName);
   pXml->addAttr("displayName", mDisplayName);
   pXml->addAttr("displayText", mDisplayText);
   pXml->addAttr("filenameDisplay", mFilenameDisplay);
   pXml->addAttr("validSessionSaveItem", mValidSessionSaveItem);
   return true;
}

bool SessionItemImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }
   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   setName(A(pElem->getAttribute(X("name"))));
   setDisplayName(A(pElem->getAttribute(X("displayName"))));
   setDisplayText(A(pElem->getAttribute(X("displayText"))));
   setFilenameDisplay(StringUtilities::fromXmlString<bool>(
      A(pElem->getAttribute(X("filenameDisplay")))));

   // This check ensures backward compatibility with files existing before the validSessionSaveItem implementation.
   if (pElem->hasAttribute(X("validSessionSaveItem")))
   {
      setValidSessionSaveItem(StringUtilities::fromXmlString<bool>(
         A(pElem->getAttribute(X("validSessionSaveItem")))));
   }

   return true;
}

bool SessionItemImp::canRename() const
{
   return false;
}

bool SessionItemImp::rename(const string &newName, string &errorMessage)
{
   setName(newName);
   setDisplayName(string());
   setFilenameDisplay(QFileInfo(QString::fromStdString(newName)).isFile());
   return true;
}

void SessionItemImp::setIcon(const QIcon& icon)
{
   *mpIcon = icon;
   updateSessionExplorer();
}

void SessionItemImp::setName(const string& name)
{
   if (name.empty() == true)
   {
      return;
   }

   if (name != mName)
   {
      mName = name;
      updateFilenameDisplay();
      updateSessionExplorer();
   }
}

void SessionItemImp::setDisplayName(const string& displayName)
{
   if (displayName != mDisplayName)
   {
      mDisplayName = displayName;
      updateSessionExplorer();
   }
}

void SessionItemImp::setDisplayText(const string& displayText)
{
   if (displayText != mDisplayText)
   {
      mDisplayText = displayText;
      updateSessionExplorer();
   }
}

void SessionItemImp::addContextMenuAction(const ContextMenuAction& menuAction)
{
   list<ContextMenuAction>::iterator iter = find(mMenuActions.begin(), mMenuActions.end(), menuAction);
   if (iter == mMenuActions.end())
   {
      mMenuActions.push_back(menuAction);
   }
}

void SessionItemImp::setContextMenuActions(const list<ContextMenuAction>& actions)
{
   mMenuActions = actions;
}

void SessionItemImp::setFilenameDisplay(bool bFilenameDisplay)
{
   if (bFilenameDisplay != mFilenameDisplay)
   {
      mFilenameDisplay = bFilenameDisplay;
      updateFilenameDisplay();
   }
}

void SessionItemImp::addPropertiesPage(const string& plugInName)
{
   if (plugInName.empty() == true)
   {
      return;
   }

   vector<string>::iterator iter = find(mPropertiesPages.begin(), mPropertiesPages.end(), plugInName);
   if (iter == mPropertiesPages.end())
   {
      mPropertiesPages.push_back(plugInName);
   }
}

void SessionItemImp::removePropertiesPage(const string& plugInName)
{
   if (plugInName.empty() == true)
   {
      return;
   }

   vector<string>::iterator iter = find(mPropertiesPages.begin(), mPropertiesPages.end(), plugInName);
   if (iter != mPropertiesPages.end())
   {
      mPropertiesPages.erase(iter);
   }
}

void SessionItemImp::setPropertiesPages(const vector<string>& plugInNames)
{
   mPropertiesPages = plugInNames;
}

void SessionItemImp::updateFilenameDisplay()
{
   if (mFilenameDisplay == false)
   {
      return;
   }

   string displayName;
   string displayText;

   // Iteratively check for a directory based on slashes in the name and
   // set the display name as all text in the name following the directory
   QString name = QString::fromStdString(mName);
   QRegExp slashes("\\\\|/");
   int lastPos = -1;

   // First check the last slash in the name
   int pos = name.lastIndexOf(slashes);
   while (pos != -1)
   {
      QString dir = name.left(pos + 1);

      QFileInfo fileInfo(dir);
      if (fileInfo.isDir() == true)
      {
         if (lastPos < 0)
         {
            // A directory was found that was not the first slash, so set
            // all text to the right of the directory as the display name
            displayName = mName.substr(pos + 1);
            displayText = mName;
            break;
         }
      }
      else if (lastPos > -1)
      {
         // The first slash did not indicate a directory, so no other checks are needed
         break;
      }

      if (lastPos == -1)
      {
         // The last slash did not indicate a directory, so check the
         // first slash to see if other slashes need to be checked
         lastPos = pos - 1;
         pos = name.indexOf(slashes);

         if (pos == lastPos + 1)
         {
            // The name only has one slash that is not a directory
            break;
         }
      }
      else if (lastPos > -1)
      {
         // The first slash indicated a directory, so continue to check slashes
         // at the end of the name to get the complete directory path
         pos = name.lastIndexOf(slashes, lastPos);
         lastPos = -2;
      }
      else
      {
         // The last slash did not indicate a directory, so check the next-to-last slash
         pos = name.lastIndexOf(slashes, pos - 1);
      }
   }

   setDisplayName(displayName);
   setDisplayText(displayText);
}

void SessionItemImp::updateSessionExplorer()
{
   Service<DesktopServices> pDesktop;

   SessionExplorerImp* pExplorer =
      dynamic_cast<SessionExplorerImp*>(pDesktop->getWindow("Session Explorer", DOCK_WINDOW));
   if (pExplorer != NULL)
   {
      pExplorer->updateData(dynamic_cast<SessionItem*>(this));
   }
}

bool SessionItemImp::isValidSessionSaveItem() const
{
   return mValidSessionSaveItem;
}

void SessionItemImp::setValidSessionSaveItem(bool isValid)
{
   mValidSessionSaveItem = isValid;
}
