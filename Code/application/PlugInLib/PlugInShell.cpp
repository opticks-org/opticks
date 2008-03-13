/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlugInShell.h"
#include "AppVersion.h"
#include "SettableSessionItem.h"

using namespace std;

PlugInShell::PlugInShell() :
   mVersion(APP_VERSION_NUMBER),
   mProductionStatus(false),
   mAllowMultipleInstances(false)
{
}

PlugInShell::~PlugInShell()
{
}

const string& PlugInShell::getId() const
{
   return mpSessionItem->getId();
}

const QIcon& PlugInShell::getIcon() const
{
   return mpSessionItem->getIcon();
}

const string& PlugInShell::getName() const
{
   return mpSessionItem->getName();
}

const string& PlugInShell::getDisplayName() const
{
   return mpSessionItem->getDisplayName();
}

const string& PlugInShell::getDisplayText() const
{
   return mpSessionItem->getDisplayText();
}

list<ContextMenuAction> PlugInShell::getContextMenuActions() const
{
   return mpSessionItem->getContextMenuActions();
}

bool PlugInShell::hasFilenameDisplay() const
{
   return mpSessionItem->hasFilenameDisplay();
}

vector<string> PlugInShell::getPropertiesPages() const
{
   return mpSessionItem->getPropertiesPages();
}

bool PlugInShell::serialize(SessionItemSerializer& serializer) const
{
   return false;
}

bool PlugInShell::deserialize(SessionItemDeserializer& deserializer)
{
   return false;
}

string PlugInShell::getVersion() const
{
   return mVersion;
}

bool PlugInShell::isProduction() const
{
   return mProductionStatus;
}

string PlugInShell::getCreator() const
{
   return mCreator;
}

string PlugInShell::getCopyright() const
{
   return mCopyright;
}

map<string, string> PlugInShell::getDependencyCopyright() const
{
   return mDependencyCopyright;
}

string PlugInShell::getDescription() const
{
   return mDescription;
}

string PlugInShell::getShortDescription() const
{
   return mShortDescription;
}

string PlugInShell::getDescriptorId() const
{
   return mDescriptorId;
}

string PlugInShell::getType() const
{
   return mType;
}

string PlugInShell::getSubtype() const
{
   return mSubtype;
}

bool PlugInShell::areMultipleInstancesAllowed() const
{
   return mAllowMultipleInstances;
}

bool PlugInShell::setId(const SessionItemId& id)
{
   return mpSessionItem->setId(id);
}

void PlugInShell::setName(const string& name)
{
   mpSessionItem->setName(name);
}

void PlugInShell::setDisplayName(const string& displayName)
{
   mpSessionItem->setDisplayName(displayName);
}

void PlugInShell::setDisplayText(const string& displayText)
{
   mpSessionItem->setDisplayText(displayText);
}

void PlugInShell::addContextMenuAction(const ContextMenuAction& menuAction)
{
   mpSessionItem->addContextMenuAction(menuAction);
}

void PlugInShell::setContextMenuActions(const list<ContextMenuAction>& actions)
{
   mpSessionItem->setContextMenuActions(actions);
}

void PlugInShell::setFilenameDisplay(bool bFilenameDisplay)
{
   mpSessionItem->setFilenameDisplay(bFilenameDisplay);
}

void PlugInShell::addPropertiesPage(const string& plugInName)
{
   mpSessionItem->addPropertiesPage(plugInName);
}

void PlugInShell::removePropertiesPage(const string& plugInName)
{
   mpSessionItem->removePropertiesPage(plugInName);
}

void PlugInShell::setPropertiesPages(const vector<string>& plugInNames)
{
   mpSessionItem->setPropertiesPages(plugInNames);
}

void PlugInShell::setVersion(const string& version)
{
   mVersion = version;
}

void PlugInShell::setProductionStatus(bool productionStatus)
{
   mProductionStatus = productionStatus;
}

void PlugInShell::setCreator(const string& creator)
{
   mCreator = creator;
}

void PlugInShell::setCopyright(const string& copyright)
{
   mCopyright = copyright;
}

void PlugInShell::addDependencyCopyright(const string &dependencyName, const string &copyright)
{
   if(!dependencyName.empty())
   {
      if(copyright.empty())
      {
         map<string, string>::iterator loc = mDependencyCopyright.find(dependencyName);
         if(loc != mDependencyCopyright.end())
         {
            mDependencyCopyright.erase(loc);
         }
      }
      else
      {
         mDependencyCopyright[dependencyName] = copyright;
      }
   }
}

void PlugInShell::setDescription(const string& description)
{
   mDescription = description;
}

void PlugInShell::setShortDescription(const string& shortDescription)
{
   mShortDescription = shortDescription;
}

void PlugInShell::setDescriptorId(const string& id)
{
   mDescriptorId = id;
}

void PlugInShell::setType(const string& type)
{
   mType = type;
}

void PlugInShell::setSubtype(const string& subtype)
{
   mSubtype = subtype;
}

void PlugInShell::allowMultipleInstances(bool bMultipleInstances)
{
   mAllowMultipleInstances = bMultipleInstances;
}
