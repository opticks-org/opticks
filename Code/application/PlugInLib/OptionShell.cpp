/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionShell.h"
#include "PlugInManagerServices.h"

#include <QtGui/QWidget>

using namespace std;

OptionShell::OptionShell() :
   mpOptionWidget(NULL)
{
   setType(PlugInManagerServices::OptionType());
}

OptionShell::~OptionShell()
{
   delete mpOptionWidget;
}

const string& OptionShell::getOptionName()
{
   return mOptionName;
}

void OptionShell::setOptionName(const string& name)
{
   mOptionName = name;
}

QWidget* OptionShell::getWidget()
{
   if (mpOptionWidget == NULL)
   {
      mpOptionWidget = createOptionsWidget();
   }

   return mpOptionWidget;
}

QWidget* OptionShell::getStoredWidget()
{
   return mpOptionWidget;
}
