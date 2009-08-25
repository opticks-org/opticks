/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QWidget>

#include "PlugInManagerServices.h"
#include "PropertiesShell.h"

using namespace std;

PropertiesShell::PropertiesShell() :
   mpWidget(NULL),
   mpSessionItem(NULL)
{
   setType(PlugInManagerServices::PropertiesType());
}

PropertiesShell::~PropertiesShell()
{
   delete mpWidget;
}

QWidget* PropertiesShell::getWidget()
{
   if (mpWidget == NULL)
   {
      mpWidget = createWidget();
   }

   return mpWidget;
}

const string& PropertiesShell::getPropertiesName() const
{
   return mName;
}

bool PropertiesShell::initialize(SessionItem* pSessionItem)
{
   mpSessionItem = pSessionItem;
   return true;
}

void PropertiesShell::setPropertiesName(const string& name)
{
   mName = name;
}

SessionItem* PropertiesShell::getSessionItem() const
{
   return mpSessionItem;
}
