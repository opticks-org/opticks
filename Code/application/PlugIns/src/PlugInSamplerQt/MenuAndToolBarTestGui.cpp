/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "MenuAndToolBarTestGui.h"
#include "MenuBar.h"
#include "ToolBar.h"

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QWidgetAction>

MenuAndToolBarTestGui::MenuAndToolBarTestGui( QWidget* pParent, const char* pName, bool modal )
: QDialog(pParent)
{
   if (pName == NULL)
   {
      setObjectName( "MenuAndToolBarGui" );
   }
   setModal( FALSE );

   QGridLayout *pLayout = new QGridLayout( this );

   mpCancelButton = new QPushButton( "cancelButton", this );
   mpDisableOpenButton = new QPushButton( "disableOpenButton", this );
   mpEnableOpenButton = new QPushButton( "enableOpenButton", this );
   mpAddTestItemsButton = new QPushButton( "addTestItemsButton", this );
   mpRemoveTestItemsButton = new QPushButton( "removeTestItemsButton", this );
   mpDisableSpectralItemsButton = new QPushButton( "disableSpectralItemsButton", this );
   mpEnableSpectralItemsButton = new QPushButton( "enableSpectralItemsButton", this );
   mpReorderToolboxButton = new QPushButton( "reorderToolboxButton", this );
   mpResetToolboxButton = new QPushButton( "resetToolboxButton", this );

   pLayout->addWidget( mpDisableOpenButton, 0, 0 );
   pLayout->addWidget( mpEnableOpenButton, 0, 1 );
   pLayout->addWidget( mpDisableSpectralItemsButton, 1, 0 );
   pLayout->addWidget( mpEnableSpectralItemsButton, 1, 1 );
   pLayout->addWidget( mpAddTestItemsButton, 2, 0 );    
   pLayout->addWidget( mpRemoveTestItemsButton, 2, 1 );
   pLayout->addWidget( mpReorderToolboxButton, 3, 0 );    
   pLayout->addWidget( mpResetToolboxButton, 3, 1 );
   pLayout->addWidget( mpCancelButton, 4, 3 );

   mpCancelButton->setText("Close");
   mpDisableOpenButton->setText("Disable File Open Button");
   mpEnableOpenButton->setText("Enable File Open Button");
   mpAddTestItemsButton->setText("Add Items to Tests Menu");
   mpRemoveTestItemsButton->setText("Reset Tests Menu");
   mpDisableSpectralItemsButton->setText("Disable Contents of General Algorithms Menu");
   mpEnableSpectralItemsButton->setText("Enable Contents of General Algorithms Menu");
   mpReorderToolboxButton->setText("Reverse Order of Toolbox Toolbar Buttons");
   mpResetToolboxButton->setText("Reset Toolbox Toolbar Buttons");
   resize( QSize(500, 380).expandedTo(minimumSizeHint()) );

   // signals and slots connections
   bool ok = connect( mpCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
   ok = connect( mpDisableOpenButton, SIGNAL( clicked() ), this, SLOT( disableFileOpenButton() ) );
   ok = connect( mpEnableOpenButton, SIGNAL( clicked() ), this, SLOT( enableFileOpenButton() ) );
   ok = connect( mpAddTestItemsButton, SIGNAL( clicked() ), this, SLOT( addMenuItems() ) );
   ok = connect( mpRemoveTestItemsButton, SIGNAL( clicked() ), this, SLOT( removeMenuItems() ) );
   ok = connect( mpDisableSpectralItemsButton, SIGNAL( clicked() ), this, SLOT( disableGeneralAlgorithmsMenuItems() ) );
   ok = connect( mpEnableSpectralItemsButton, SIGNAL( clicked() ), this, SLOT( enableGeneralAlgorithmsMenuItems() ) );
   ok = connect( mpReorderToolboxButton, SIGNAL( clicked() ), this, SLOT( reorderToolboxToolbarButtons() ) );
   ok = connect( mpResetToolboxButton, SIGNAL( clicked() ), this, SLOT( resetToolboxToolbarButtons() ) );
   init();
}

/*
*  Destroys the object and frees any allocated resources
*/
MenuAndToolBarTestGui::~MenuAndToolBarTestGui()
{
   // no need to delete child widgets, Qt does it all for us
}

void MenuAndToolBarTestGui::init()
{
   ToolBar *pToolboxBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Toolbox", TOOLBAR ) );
   if( pToolboxBar != NULL )
   {
      mToolbarItems = pToolboxBar->getItems();
   }
}

void MenuAndToolBarTestGui::disableFileOpenButton()
{
   ToolBar *pToolBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Standard", TOOLBAR ) );
   std::vector<QAction*> items = pToolBar->getItems();
   items.at(1)->setEnabled( false );
}

void MenuAndToolBarTestGui::enableFileOpenButton()
{
   ToolBar *pToolBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Standard", TOOLBAR ) );
   std::vector<QAction*> items = pToolBar->getItems();
   items.at(1)->setEnabled( true );
}

void MenuAndToolBarTestGui::addMenuItems()
{
   // insert some test items into the menu
   ToolBar *pTestBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Tests", TOOLBAR ) );
   if( pTestBar != NULL )
   {
      MenuBar *pMenuBar = pTestBar->getMenuBar();
      if( pMenuBar != NULL )
      {
         QAction *pFirstItem = pMenuBar->getMenuItem( "Tests/Close Notification Test" );
         QAction *pMouseModeItem = pMenuBar->getMenuItem( "Tests/MouseMode Test" );

         pMenuBar->addCommand( "Tests/Last Item in the Menu" );
         pMenuBar->addCommand( "Tests/After Message Log Test", pMouseModeItem );
         pMenuBar->addCommand( "Tests/Top of the Menu", pFirstItem );
      }
   }
}

void MenuAndToolBarTestGui::removeMenuItems()
{
   // remove the test items from the menu
   ToolBar *pTestBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Tests", TOOLBAR ) );
   if( pTestBar != NULL )
   {
      MenuBar *pMenuBar = pTestBar->getMenuBar();
      if( pMenuBar != NULL )
      {
         QAction *pFirstItem = pMenuBar->getMenuItem( "Tests/Top of the Menu" );
         QAction *pAfterMsgItem = pMenuBar->getMenuItem( "Tests/After Message Log Test" );
         QAction *pLastItem = pMenuBar->getMenuItem( "Tests/Last Item in the Menu" );

         pMenuBar->removeMenuItem( pFirstItem );
         pMenuBar->removeMenuItem( pAfterMsgItem );
         pMenuBar->removeMenuItem( pLastItem );
      }
   }
}

void MenuAndToolBarTestGui::disableGeneralAlgorithmsMenuItems()
{
   ToolBar *pAlgorithmBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "General Algorithms", TOOLBAR ) );
   if( pAlgorithmBar != NULL )
   {
      MenuBar *pMenuBar = pAlgorithmBar->getMenuBar();
      if( pMenuBar != NULL )
      {
         QAction *pBandMath = pMenuBar->getMenuItem( "General Algorithms/Band Math" );
         QAction *pDataFusion = pMenuBar->getMenuItem( "General Algorithms/Data &Fusion" );
         QAction *pPCA = pMenuBar->getMenuItem( "General Algorithms/Principal Component Analysis" );
         pBandMath->setEnabled( false );
         pDataFusion->setEnabled( false );
         pPCA->setEnabled( false );
      }
   }
}

void MenuAndToolBarTestGui::enableGeneralAlgorithmsMenuItems()
{
   ToolBar *pAlgorithmBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "General Algorithms", TOOLBAR ) );
   if( pAlgorithmBar != NULL )
   {
      MenuBar *pMenuBar = pAlgorithmBar->getMenuBar();
      if( pMenuBar != NULL )
      {
         QAction *pBandMath = pMenuBar->getMenuItem( "General Algorithms/Band Math" );
         QAction *pDataFusion = pMenuBar->getMenuItem( "General Algorithms/Data &Fusion" );
         QAction *pPCA = pMenuBar->getMenuItem( "General Algorithms/Principal Component Analysis" );
         pBandMath->setEnabled( true );
         pDataFusion->setEnabled( true );
         pPCA->setEnabled( true );
      }
   }
}

void MenuAndToolBarTestGui::reorderToolboxToolbarButtons()
{
   ToolBar *pToolboxBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Toolbox", TOOLBAR ) );
   if( pToolboxBar != NULL )
   {
      std::vector<QAction*> tempItems = pToolboxBar->getItems();
      for( unsigned int index = 0; index < tempItems.size(); index++ )
      {
         QAction* pAction = tempItems.at(index);
         if ((pAction != NULL) && (dynamic_cast<QWidgetAction*>(pAction) == NULL))
         {
            pToolboxBar->removeItem(pAction);
         }
      }

      std::vector<QAction*>::reverse_iterator iter;
      for (iter = mToolbarItems.rbegin(); iter != mToolbarItems.rend(); ++iter)
      {
         QAction* pAction = *iter;
         if ((pAction != NULL) && (dynamic_cast<QWidgetAction*>(pAction) == NULL))
         {
            pToolboxBar->addButton(pAction);
         }
      }
   }
}

void MenuAndToolBarTestGui::resetToolboxToolbarButtons()
{
   ToolBar *pToolboxBar = dynamic_cast<ToolBar*>( Service<DesktopServices>()->getWindow( "Toolbox", TOOLBAR ) );
   if( pToolboxBar != NULL )
   {
      std::vector<QAction*> tempItems = pToolboxBar->getItems();
      for( unsigned int index = 0; index < tempItems.size(); index++ )
      {
         QAction* pAction = tempItems.at(index);
         if ((pAction != NULL) && (dynamic_cast<QWidgetAction*>(pAction) == NULL))
         {
            pToolboxBar->removeItem(pAction);
         }
      }

      std::vector<QAction*>::iterator iter;
      for (iter = mToolbarItems.begin(); iter != mToolbarItems.end(); ++iter)
      {
         QAction* pAction = *iter;
         if ((pAction != NULL) && (dynamic_cast<QWidgetAction*>(pAction) == NULL))
         {
            pToolboxBar->addButton(pAction);
         }
      }
   }
}
