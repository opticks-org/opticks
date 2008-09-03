/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PixmapGridButton.h"

#include "AppVerify.h"
#include "PixmapGrid.h"

#include <QtCore/QEvent>
#include <QtGui/QActionEvent>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QWidgetAction>

#include <string>
#include <vector>

using namespace std;

PixmapGridButton::PixmapGridButton(QWidget* pParent)
: QToolButton(pParent)
{   
   initialize();
}

PixmapGridButton::PixmapGridButton(PixmapGrid* pGrid, bool syncIcon, QWidget* pParent)
: QToolButton(pParent)
{
   initialize();
   setSyncIcon(syncIcon);
   setPixmapGrid(pGrid);
}

void PixmapGridButton::initialize()
{
   mpGrid = NULL;
   mSyncIcon = true;
   mpGridMenu = new QMenu(this->parentWidget());
   mpGridAction = new QWidgetAction(this);
   VERIFYNR(connect(mpGridMenu, SIGNAL(aboutToShow()), this, SLOT(menuBeingShown())));
   VERIFYNR(connect(mpGridMenu, SIGNAL(aboutToHide()), this, SLOT(menuBeingHidden())));

   mpGridMenu->addAction(mpGridAction);
   setMenu(mpGridMenu);
   setPopupMode(QToolButton::MenuButtonPopup);
   setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   QAction* pMenuAction = mpGridMenu->menuAction();
   mClickShowsMenu = false;
   setClickShowsMenu(true);
   setPixmapGrid(NULL);
}

void PixmapGridButton::setClickShowsMenu(bool showMenu)
{
   if (mClickShowsMenu == showMenu)
   {
      return;
   }
   if (showMenu)
   {
      VERIFYNR(connect(this, SIGNAL(clicked(bool)), this, SLOT(showMenu())));
   }
   else
   {
      VERIFYNR(disconnect(this, SIGNAL(clicked(bool)), this, SLOT(showMenu())));
   }
   mClickShowsMenu = showMenu;
}

bool PixmapGridButton::getClickShowsMenu()
{
   return mClickShowsMenu;
}

void PixmapGridButton::menuBeingShown()
{
   if (mpGrid != NULL)
   {
      VERIFYNR(disconnect(mpGrid, SIGNAL(pixmapSelected(const QPixmap&)), this, SLOT(updateIcon(const QPixmap&))));
      mInitialMenuPixmap = getCurrentIdentifier();
   }
}

void PixmapGridButton::menuBeingHidden()
{
   if (mpGrid != NULL)
   {
      if (mInitialMenuPixmap.isEmpty() == false)
      {
         setCurrentIdentifier(mInitialMenuPixmap);
         mInitialMenuPixmap.clear();
      }

      VERIFYNR(connect(mpGrid, SIGNAL(pixmapSelected(const QPixmap&)), this, SLOT(updateIcon(const QPixmap&))));
   }
}

void PixmapGridButton::refreshMenu()
{
   QActionEvent actionChangedEvent(QEvent::ActionChanged, mpGridAction);
   QApplication::sendEvent(mpGridMenu, &actionChangedEvent);
}

void PixmapGridButton::setPixmapGrid(PixmapGrid* pGrid)
{  
   if (mpGrid != NULL)
   {
      VERIFYNR(disconnect(mpGrid, SIGNAL(pixmapSelected(const QPixmap&)), this, SLOT(updateIcon(const QPixmap&))));
      mpGrid->removeEventFilter(this);
   }
   mpGrid = pGrid;
   if (mpGrid != NULL)
   {
      VERIFYNR(connect(mpGrid, SIGNAL(pixmapSelected(const QPixmap&)), this, SLOT(updateIcon(const QPixmap&))));
      mpGrid->setCellTracking(true);
      if (mSyncIcon)
      {
         setIcon(QIcon(mpGrid->getSelectedPixmap()));
      }
      mpGrid->installEventFilter(this);
   }
   mpGridAction->setDefaultWidget(mpGrid);
   mpGridMenu->clear();
   mpGridMenu->addAction(mpGridAction);

}

PixmapGrid* PixmapGridButton::getPixmapGrid() const
{
   return mpGrid;
}

const QString& PixmapGridButton::getCurrentIdentifier()
{
   static QString retValue = "";
   if (mpGrid != NULL)
   {
      retValue = mpGrid->getSelectedPixmapIdentifier();
   }
   else
   {
      retValue = "";
   }
   return retValue;
}

void PixmapGridButton::setCurrentIdentifier(const QString& newValue)
{
   if (mpGrid != NULL)
   {
      mpGrid->setSelectedPixmap(newValue);
   }
}

void PixmapGridButton::updateIcon(const QPixmap& newPix)
{
   if (mSyncIcon)
   {
      setIcon(QIcon(newPix));
   }
   if (mpGrid != NULL)
   {
      emit valueChanged(mpGrid->getSelectedPixmapIdentifier());
   }
}

void PixmapGridButton::setSyncIcon(bool bValue)
{
   mSyncIcon = bValue;
}

bool PixmapGridButton::getSyncIcon()
{
   return mSyncIcon;
}

bool PixmapGridButton::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pEvent->type() == QEvent::MouseButtonRelease)
      {
         if (pObject == mpGrid)
         {
            mInitialMenuPixmap.clear();
            mpGridMenu->close();

            QPixmap pix = mpGrid->getSelectedPixmap();
            updateIcon(pix);
         }
      }
   }

   return QToolButton::eventFilter(pObject, pEvent);
}
