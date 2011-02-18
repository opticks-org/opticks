/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVOLUTIONMATRIXWIDGET_H
#define CONVOLUTIONMATRIXWIDGET_H

#include "ConfigurationSettings.h"
#include "ui_ConvolutionMatrixWidget.h"
#include <ossim/matrix/newmat.h>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QWidget>

class QIcon;

class NumberItem : public QTableWidgetItem
{
public:
   enum ItemType { Type=QTableWidgetItem::UserType }; // Define NumberItem::Type
   NumberItem();
   NumberItem(const NumberItem& other);
   virtual ~NumberItem();

   QTableWidgetItem* clone() const;
   void setData(int role, const QVariant& value);
};

class ConvolutionMatrixWidget : public QWidget, private Ui_ConvolutionMatrixWidget
{
   Q_OBJECT

public:
   ConvolutionMatrixWidget(QWidget* pParent = NULL);
   virtual ~ConvolutionMatrixWidget();

protected slots:
   void resizeFilter();
   void linkToggled(bool locked);
   void matrixButtonPressed(QAbstractButton* pButton);
   void presetButtonPressed(QAbstractButton* pButton);

protected:
   NEWMAT::Matrix getCurrentMatrix() const;
   void saveToConfigurationSettings() const;
   void loadFromConfigurationSettings();

private:
   QIcon* mpLockIcon;
   QIcon* mpUnlockIcon;
   QMap<QString, QPair<NEWMAT::Matrix, double> > mPresets;
};

#endif