/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ToolBarAdapter.h"

using namespace std;

ToolBarAdapter::ToolBarAdapter(const string& id, const string& name, QWidget* parent) :
   ToolBarImp(id, name, parent)
{
}

ToolBarAdapter::~ToolBarAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& ToolBarAdapter::getObjectType() const
{
   static string type("ToolBarAdapter");
   return type;
}

bool ToolBarAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ToolBar"))
   {
      return true;
   }

   return ToolBarImp::isKindOf(className);
}
