/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SessionExplorerAdapter.h"

using namespace std;

SessionExplorerAdapter::SessionExplorerAdapter(const string& id, QWidget* parent) :
   SessionExplorerImp(id, "Session Explorer", parent)
{
}

SessionExplorerAdapter::~SessionExplorerAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& SessionExplorerAdapter::getObjectType() const
{
   static string type("SessionExplorerAdapter");
   return type;
}

bool SessionExplorerAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SessionExplorer"))
   {
      return true;
   }

   return SessionExplorerImp::isKindOf(className);
}
