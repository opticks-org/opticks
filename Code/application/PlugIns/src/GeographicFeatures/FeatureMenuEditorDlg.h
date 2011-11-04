/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATUREMENUEDITORDLG_H
#define FEATUREMENUEDITORDLG_H

#include <QtGui/QDialog>

#include "FeatureClass.h"
#include "ObjectResource.h"

#include <vector>
#include <string>

class DynamicObject;
class ListInspectorWidget;
class QListWidget;
class QListWidgetItem;

class FeatureMenuEditorDlg : public QDialog
{
   Q_OBJECT

public:
   FeatureMenuEditorDlg(QWidget *pParent = NULL);

public slots:
   void accept();
   void addItems();
   void saveInspector(QWidget *pInspector, QListWidgetItem *pItem);
   void loadInspector(QWidget *pInspector, QListWidgetItem *pItem);
   void removeItem(QListWidgetItem *pItem);

private:
   FeatureMenuEditorDlg(const FeatureMenuEditorDlg& rhs);
   FeatureMenuEditorDlg& operator=(const FeatureMenuEditorDlg& rhs);
   void populateList();

   FeatureClass mCurrentFeatureClass;
   FactoryResource<DynamicObject> mpOptionsSet;
   ListInspectorWidget* mpListInspector;
};

#endif
