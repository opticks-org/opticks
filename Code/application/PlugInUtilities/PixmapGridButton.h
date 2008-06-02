/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXMAPGRIDBUTTON_H
#define PIXMAPGRIDBUTTON_H

#include "TypesFile.h"

#include <QtGui/QToolButton>

class PixmapGrid;
class QMenu;
class QWidgetAction;

class PixmapGridButton : public QToolButton
{
   Q_OBJECT

public:
   PixmapGridButton(QWidget* pParent);
   PixmapGridButton(PixmapGrid* pGrid, bool syncIcon, QWidget* pParent);
   void setPixmapGrid(PixmapGrid* pGrid);
   PixmapGrid* getPixmapGrid() const;
   void setSyncIcon(bool bValue);
   bool getSyncIcon();
   void setClickShowsMenu(bool showMenu);
   bool getClickShowsMenu();
   const QString& getCurrentIdentifier();
   void setCurrentIdentifier(const QString& newValue);

signals:
   void valueChanged(const QString& identifier);

protected:
   void refreshMenu();

private slots:
   void updateIcon(const QPixmap& newPix);
   void menuBeingShown();
   void menuBeingHidden();

private:
   void initialize();
   bool eventFilter(QObject* pObject, QEvent* pEvent);

   PixmapGrid* mpGrid;
   QMenu* mpGridMenu;
   QWidgetAction* mpGridAction;
   bool mSyncIcon;
   bool mClickShowsMenu;
   QString mInitialMenuPixmap;
};

#endif
