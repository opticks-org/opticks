/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QColorDialog>
#include <QtGui/QWidgetAction>

#include "ColorMenu.h"
#include "ColorGrid.h"
#include "DesktopServices.h"

ColorMenu::ColorMenu(QWidget* pParent) :
   QMenu(pParent),
   mColor(Qt::black),
   mpColorGrid(NULL)
{
   mpColorGrid = new ColorGrid(this);
   mpColorGrid->setCellTracking(true);
   mpColorGrid->installEventFilter(this);

   QWidgetAction* pGridAction = new QWidgetAction(this);
   pGridAction->setDefaultWidget(mpColorGrid);

   addAction(pGridAction);
   addSeparator();
   addAction("&Custom...", this, SLOT(setCustomColor()));
}

ColorMenu::~ColorMenu()
{
}

QColor ColorMenu::getSelectedColor() const
{
   return mColor;
}

void ColorMenu::setSelectedColor(const QColor& color)
{
   // Set the grid color in case the user cancelled the custom color dialog,
   // where the grid color could be different than the menu color
   mpColorGrid->setSelectedColor(color);

   // Set the menu color
   if (color != mColor)
   {
      mColor = color;
      emit colorSelected(mColor);
   }
}

void ColorMenu::setCustomColor()
{
   // Since this is a popup widget, set the dialog parent as a non-popup widget
   QWidget* pParent = parentWidget();
   while ((pParent != NULL) && (pParent->windowType() == Qt::Popup))
   {
      pParent = pParent->parentWidget();
   }

   if (pParent == NULL)
   {
      Service<DesktopServices> pDesktop;
      pParent = pDesktop->getMainWidget();
   }

   QColor customColor = QColorDialog::getColor(mColor, pParent);
   if (customColor.isValid() == true)
   {
      setSelectedColor(customColor);
   }
}

bool ColorMenu::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pEvent->type() == QEvent::MouseButtonRelease)
      {
         if (pObject == mpColorGrid)
         {
            close();

            QColor color = mpColorGrid->getSelectedColor();
            setSelectedColor(color);
         }
      }
   }

   return QMenu::eventFilter(pObject, pEvent);
}
