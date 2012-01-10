/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLineEdit>

#include "DisplayToolBar.h"

using namespace std;

DisplayToolBar::DisplayToolBar(const string& id, QWidget* parent) :
   ToolBarAdapter(id, "Display", parent),
   mpPercentCombo(NULL)
{
}

DisplayToolBar::~DisplayToolBar()
{
}

void DisplayToolBar::addPercentageCombo()
{
   delete mpPercentCombo;
   mpPercentCombo = NULL;

   mpPercentCombo = new QComboBox(this);
   if (mpPercentCombo == NULL)
   {
      return;
   }

   mpPercentCombo->setEditable(true);
   mpPercentCombo->setAutoCompletion(false);
   mpPercentCombo->setFixedWidth(75);
   mpPercentCombo->setInsertPolicy(QComboBox::NoInsert);
   mpPercentCombo->setStatusTip("Zooms to a percentage of the spatial pixel size");
   mpPercentCombo->setToolTip("Zoom Percentage");
   mpPercentCombo->addItem("800%");
   mpPercentCombo->addItem("400%");
   mpPercentCombo->addItem("200%");
   mpPercentCombo->addItem("150%");
   mpPercentCombo->addItem("100%");
   mpPercentCombo->addItem("75%");
   mpPercentCombo->addItem("50%");
   mpPercentCombo->addItem("10%");
   mpPercentCombo->clearEditText();

   addWidget(mpPercentCombo);

   connect(mpPercentCombo, SIGNAL(activated(int)), this, SLOT(zoomToComboValue(int)));

   QLineEdit* pEdit = mpPercentCombo->lineEdit();
   if (pEdit != NULL)
   {
      connect(pEdit, SIGNAL(returnPressed()), this, SLOT(zoomToCustomValue()));
   }
}

void DisplayToolBar::enablePercentageCombo(bool bEnable)
{
   mpPercentCombo->setEnabled(bEnable);
}

void DisplayToolBar::setZoomPercentage(double dPercent)
{
   mpPercentCombo->clearEditText();

   if (dPercent >= 0.0)
   {
      int iPercent = static_cast<int>(dPercent + 0.5);

      QString strPercent = QString::number(iPercent);
      strPercent += "%";

      mpPercentCombo->setEditText(strPercent);
   }
}

void DisplayToolBar::zoomToComboValue(int iIndex)
{
   double dPercent = 0.0;
   switch (iIndex)
   {
      case 0:      // 800%
         dPercent = 800.0;
         break;

      case 1:      // 400%
         dPercent = 400.0;
         break;

      case 2:      // 200%
         dPercent = 200.0;
         break;

      case 3:      // 150%
         dPercent = 150.0;
         break;

      case 4:      // 100%
         dPercent = 100.0;
         break;

      case 5:      // 75%
         dPercent = 75.0;
         break;

      case 6:      // 50%
         dPercent = 50.0;
         break;

      case 7:      // 10%
         dPercent = 10.0;
         break;

      default:
         return;
   }

   emit zoomChanged(dPercent);
}

void DisplayToolBar::zoomToCustomValue()
{
   QString strZoom = mpPercentCombo->currentText();

   int iIndex = 0;
   while (iIndex != -1)
   {
      iIndex = strZoom.lastIndexOf("%");
      if (iIndex != -1)
      {
         strZoom.remove(iIndex, 1);
      }
   }

   bool bSuccess = false;

   double dPercent = 0;
   dPercent = strZoom.toDouble(&bSuccess);
   if (bSuccess == true)
   {
      emit zoomChanged(dPercent);
   }
}
