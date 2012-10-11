/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLORMENU_H
#define COLORMENU_H

#include <QtGui/QMenu>

class ColorGrid;

class ColorMenu : public QMenu
{
   Q_OBJECT

public:
   ColorMenu(QWidget* pParent = 0);
   ~ColorMenu();

   QColor getSelectedColor() const;

public slots:
   void setSelectedColor(const QColor& color);
   void setCustomColor();

signals:
   void colorSelected(const QColor& color);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);

private:
   ColorMenu(const ColorMenu& rhs);
   ColorMenu& operator=(const ColorMenu& rhs);
   QColor mColor;
   ColorGrid* mpColorGrid;
};

#endif
