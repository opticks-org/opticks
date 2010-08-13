/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LogContextMenuActions.h"
#include "ContextMenu.h"
#include "MenuBar.h"
#include "PlugInRegistration.h"
#include "SessionItemSerializer.h"
#include "ToolBar.h"

#include <string>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, LogContextMenuActions);

LogContextMenuActions::LogContextMenuActions() :
   mpLogAction(NULL)
{
   AlgorithmShell::setName("Log Context Menu Actions");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Toggles logging of context menu actions in the message log");
   setDescriptorId("{C9CC7DC0-3CF0-4D48-BA70-366F124A86C1}");
   executeOnStartup(true);
   allowMultipleInstances(false);
   destroyAfterExecute(false);
   setWizardSupported(false);
}

LogContextMenuActions::~LogContextMenuActions()
{
   ToolBar* pToolBar = static_cast<ToolBar*>(mpDesktop->getWindow("Developer Tools", TOOLBAR));
   if (pToolBar != NULL)
   {
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(mpLogAction);
      }
   }
}

bool LogContextMenuActions::createAction()
{
   // Add a menu command to toggle context menu logging
   ToolBar* pToolBar = static_cast<ToolBar*>(mpDesktop->getWindow("Developer Tools", TOOLBAR));
   if (pToolBar != NULL)
   {
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      if (pMenuBar != NULL)
      {
         const string& name = getName();

         mpLogAction = pMenuBar->addCommand(string("Developer Tools/") + name, name);
         if (mpLogAction != NULL)
         {
            mpLogAction->setAutoRepeat(false);
            mpLogAction->setCheckable(true);
            mpLogAction->setToolTip(QString::fromStdString(getName()));
            mpLogAction->setStatusTip(QString::fromStdString(getDescription()));

            if (ContextMenu::hasSettingLogActions() == true)
            {
               mpLogAction->setChecked(ContextMenu::getSettingLogActions());
            }

            connect(mpLogAction, SIGNAL(toggled(bool)), this, SLOT(logContextMenuActions(bool)));
         }
      }
   }

   return (mpLogAction != NULL);
}

bool LogContextMenuActions::setBatch()
{
   AlgorithmShell::setBatch();
   return false;
}

bool LogContextMenuActions::getInputSpecification(PlugInArgList*& pArgList)
{
   return true;
}

bool LogContextMenuActions::getOutputSpecification(PlugInArgList*& pArgList)
{
   return true;
}

bool LogContextMenuActions::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   return createAction();
}

void LogContextMenuActions::logContextMenuActions(bool bLog)
{
   ContextMenu::setSettingLogActions(bLog);
}

bool LogContextMenuActions::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session restore
}

bool LogContextMenuActions::deserialize(SessionItemDeserializer &deserializer)
{
   return createAction();
}
