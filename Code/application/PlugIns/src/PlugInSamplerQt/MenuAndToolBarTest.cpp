/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "MenuAndToolBarTest.h"
#include "MenuAndToolBarTestGui.h"
#include "MenuBar.h"
#include "ToolBar.h"

MenuAndToolBarTest::MenuAndToolBarTest() :
   mpGui(NULL)
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   ViewerShell::setName( "Menu And ToolBar Test" );
   setDescription( "Verifies MPR2 Requirements 6, 115, 259, and 260." );
   setMenuLocation( "[Tests]\\Menu And ToolBar Test" );
   setDescriptorId("{63BAC425-5CDC-4499-8F68-1C6C9A77C66A}");
}

MenuAndToolBarTest::~MenuAndToolBarTest()
{
}

bool MenuAndToolBarTest::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   Service<DesktopServices> pDesktop;

   mpGui = new MenuAndToolBarTestGui( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   mpGui->show();
   return true;
}

QWidget* MenuAndToolBarTest::getWidget() const
{
   return mpGui;
}

void MenuAndToolBarTest::dialogClosed()
{
   abort();
}



