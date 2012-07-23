
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

class AoiElement;
class QIcon;

class PreviousConvolutionExecution
{
public:
   PreviousConvolutionExecution() {}

   PreviousConvolutionExecution(const PreviousConvolutionExecution& rhs)
   {
      mRasterElementId = rhs.mRasterElementId;
      mAoiElementId = rhs.mAoiElementId;
      mActiveBandNums = rhs.mActiveBandNums;
   }

   PreviousConvolutionExecution& operator=(const PreviousConvolutionExecution& rhs)
   {
      if (this != &rhs)
      {
         mRasterElementId = rhs.mRasterElementId;
         mAoiElementId = rhs.mAoiElementId;
         mActiveBandNums = rhs.mActiveBandNums;
      }
      return *this;
   }
   
   std::string mRasterElementId;
   std::string mAoiElementId;
   std::vector<unsigned int> mActiveBandNums;
};

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

class FilterSettings
{
public:
   NEWMAT::Matrix mKernel;
   double mDivisor;
   double mOffset;
   bool mFloatOutput;
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
   ConvolutionMatrixWidget(const ConvolutionMatrixWidget& rhs);
   ConvolutionMatrixWidget& operator=(const ConvolutionMatrixWidget& rhs);
   QIcon* mpLockIcon;
   QIcon* mpUnlockIcon;
   QMap<QString, FilterSettings> mPresets;
   std::map<std::string, PreviousConvolutionExecution> mPreviousConvolves;
};

#endif