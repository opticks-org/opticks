/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DockWindowAdapter.h"

using namespace std;

DockWindowAdapter::DockWindowAdapter(const string& id, const string& windowName, QWidget* parent) :
   DockWindowImp(id, windowName, parent)
{
}

DockWindowAdapter::~DockWindowAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& DockWindowAdapter::getObjectType() const
{
   static string type("DockWindowAdapter");
   return type;
}

bool DockWindowAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DockWindow"))
   {
      return true;
   }

   return DockWindowImp::isKindOf(className);
}
