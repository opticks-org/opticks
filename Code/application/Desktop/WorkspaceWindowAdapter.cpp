/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WorkspaceWindowAdapter.h"

using namespace std;

WorkspaceWindowAdapter::WorkspaceWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   WorkspaceWindowImp(id, windowName, parent)
{
}

WorkspaceWindowAdapter::~WorkspaceWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& WorkspaceWindowAdapter::getObjectType() const
{
   static string type("WorkspaceWindowAdapter");
   return type;
}

bool WorkspaceWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WorkspaceWindow"))
   {
      return true;
   }

   return WorkspaceWindowImp::isKindOf(className);
}
