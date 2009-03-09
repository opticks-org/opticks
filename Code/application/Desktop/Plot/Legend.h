/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LEGEND_H
#define LEGEND_H

#include <QtCore/QMap>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

class PlotObject;

class Legend : public QTreeWidget
{
   Q_OBJECT

public:
   Legend(QWidget* parent = 0);
   ~Legend();

   void showSecondaryObjects(bool bShow);
   bool areSecondaryObjectsShown() const;

public slots:
   bool insertItem(PlotObject* pObject);
   bool setItemText(PlotObject* pObject, const QString& strName);
   bool setItemSelected(PlotObject* pObject, bool bSelected);
   bool removeItem(PlotObject* pObject);
   void clear();

signals:
   void itemSelected(PlotObject* pObject, bool bSelected);

protected slots:
   void selectObject(QTreeWidgetItem* pItem);
   void updateItemText(const QString& strObjectName);
   void updateItemPixmap();

private:
   QMap<PlotObject*, QTreeWidgetItem*> mObjects;
   bool mSecondaryObjects;
};

#endif
