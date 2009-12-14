/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDWIDGET_H
#define WIZARDWIDGET_H

#include <QtCore/QMap>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>
#include <QtGui/QStackedWidget>

#include "Value.h"

class BatchWizard;
class DataVariantEditor;

class WizardWidget : public QWidget
{
   Q_OBJECT

public:
   WizardWidget(QWidget* parent = 0);
   ~WizardWidget();

   void setActiveWizard(BatchWizard* pWizard, QStringList filesetNames);

signals:
   void modified();

protected:
   QTreeWidgetItem* getItemItem(const QString& strItem) const;
   Value* getCurrentValue() const;

protected slots:
   void showValues(QTreeWidgetItem* pItem);
   void showSelectedValues();
   void setValueWidgets(const QString& strNodeType);
   void updateBatchWizardValues();

private:
   QTreeWidget* mpNameTree;
   QMap<QTreeWidgetItem*, Value*> mValues;

   QStackedWidget* mpTypeStack;
   QLabel* mpTypeLabel;
   QComboBox* mpTypeCombo;

   QStackedWidget* mpValueStack;
   DataVariantEditor* mpValueEditor;
   QWidget* mpValueEditorWidget;
   QWidget* mpFilesetWidget;
   QComboBox* mpValueFilesetCombo;

   QCheckBox* mpRepeatCheck;
   QComboBox* mpFilesetCombo;
   QCheckBox* mpCleanupCheck;

   BatchWizard* mpWizard;
};

#endif
