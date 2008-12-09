/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BM_UI_H
#define BM_UI_H

#include "RasterDataDescriptor.h"
#include <stdio.h>

bool bFirst;
int iPos;

struct Last
{
    bool bUnaryOps;
    bool bBinaryOps;
    bool bNumbers;
    QString qsExpr;
    bool bButtons;
    bool bProcess;
    bool bBands;
} sLast;

QString FrmBM::getExpression(bool bLoadedBands) const
{
   QString strOrigExpression = txtExpression->text();
   if (bLoadedBands == false)
   {
      return strOrigExpression;
   }

   QString strLoadedExpression;

   // Convert to loaded band expression
   int iOrigPos = -1;
   iOrigPos = strOrigExpression.indexOf("b");
   while (iOrigPos != -1)
   {
      strLoadedExpression += strOrigExpression.left(iOrigPos + 1);

      strOrigExpression = strOrigExpression.mid(iOrigPos);
      if (strOrigExpression.isEmpty() == false)
      {
         int bandPos = 1;

         // Get the original band number
         unsigned int ulOrigBand = 0;
         if (sscanf(strOrigExpression.toStdString().c_str(), "b%d", &ulOrigBand) == 1)
         {
            // Get the loaded band number
            unsigned int ulLoadedBand = getLoadedBandNumber(ulOrigBand) + 1;

            // Add the value in the loaded expression
            strLoadedExpression += QString::number(ulLoadedBand);

            // Find the length of the original band number
            for (bandPos = 1; bandPos < strOrigExpression.length(); bandPos++)
            {
               QChar character = strOrigExpression.at(bandPos);
               if (character.digitValue() == -1)
               {
                  break;
               }
            }
         }

         // Remove the original band number from the original expression
         strOrigExpression = strOrigExpression.mid(bandPos);

         // Find the next band occurrence
         iOrigPos = strOrigExpression.indexOf("b");
      }
      else
      {
         iOrigPos = -1;
      }
   }

   // Append final characters that are not band numbers
   strLoadedExpression += strOrigExpression;

   return strLoadedExpression;
}

bool FrmBM::isDegrees() const
{
   bool bDegrees = false;
   bDegrees = rbDegrees->isChecked();

   return bDegrees;
}

bool FrmBM::isMultiCube() const
{
   bool bMultiCube = false;
   bMultiCube = rbCube->isChecked();

   return bMultiCube;
}

bool FrmBM::isResultsMatrix() const
{
   bool bResultsMatrix = false;
   bResultsMatrix = cbResults->isChecked();

   return bResultsMatrix;
}

void FrmBM::clear()
{
   int i;

   bFirst = true;
   iPos = -1;

   //do it twice so the fields of sLast are set correctly
   for (i = 1; i <= 2; i++)
   {
      setUnaryOps(true);
      setBinaryOps(false);
      setNumbers(true);
      setExpression("");
      setButtons(false);
      setProcess(false);
      setList(true);
   }

   bgOperators->button(31)->setEnabled(true);
   bgOperators->button(32)->setEnabled(false);
}

void FrmBM::addBand()
{
   QString qsTmp;

   qsTmp = txtExpression->text();
   sLast.qsExpr = qsTmp;

   // Get original band number
   unsigned int originalBandNumber = 0;
   int index = lisBands->currentRow();
   if ((mpDescriptor != NULL) && (index >= 0) && (index < static_cast<int>(mpDescriptor->getBandCount())))
   {
      DimensionDescriptor desc = mpDescriptor->getActiveBand(index);
      if (desc.isValid())
      {
         // BandMath is using 1-based numbers, original numbers are 0-based numbers
         originalBandNumber = desc.getOriginalNumber() + 1;
      }
   }
   
   if (iPos > 0)
   {
      if (rbBand->isChecked())
      {
         qsTmp.insert(iPos, QString("b%1").arg(originalBandNumber));
      }
      else
      {
         qsTmp.insert(iPos, QString("c%1").arg(lisBands->currentRow()+1));
      }

      bgOperators->button(32)->setEnabled(true);
      iPos = qsTmp.indexOf(')', iPos);
   }
   else
   {
      if (rbBand->isChecked())
      {
         qsTmp = QString("%1b%2").arg(txtExpression->text()).arg(originalBandNumber);
      }
      else
      {
         qsTmp = QString("%1c%2").arg(txtExpression->text()).arg(lisBands->currentRow()+1);
      }
   }

   if (bFirst == true)
   {
      bFirst = false;
   }

   setProcess(!bFirst);

   setUnaryOps(false);
   setBinaryOps(true);
   setNumbers(false);
   setExpression(qsTmp);
   setButtons(true);
   setList(false);
   bgOperators->button(31)->setEnabled(false);

   bFirst = false;
}

void FrmBM::addNumber( int iButton )
{
   QString qsTmp;

   QString textToAdd = bgNumbers->button(iButton)->text();
   if (textToAdd == "p")
   {
      textToAdd = "pi";
   }

   qsTmp = txtExpression->text();
   sLast.qsExpr = qsTmp;

   if (iPos > 0)
   {
      qsTmp.insert(iPos, textToAdd);
      bgOperators->button(32)->setEnabled(true);
      iPos = qsTmp.indexOf(')', iPos);
   }
   else
   {
      qsTmp.append(textToAdd);
   }

   setUnaryOps(false);
   setBinaryOps(true);
   setExpression(qsTmp);
   setButtons(true);
   setProcess(!bFirst);
   setList(false);

   setUnaryOps(false);
   setBinaryOps(true);
   setList(false);

   if (iButton == 10)
   {
      setNumbers(true);
      bgNumbers->button(10)->setEnabled(false);
      bgNumbers->button(11)->setEnabled(false);
      bgNumbers->button(12)->setEnabled(false);
   }
   else if ((iButton == 11) || (iButton == 12))
   {   //pi or e
      setNumbers(false);
   }
   else
   {
      setNumbers(true);
      bgNumbers->button(11)->setEnabled(false);
      bgNumbers->button(12)->setEnabled(false);
   }

   bgOperators->button(31)->setEnabled(false);

   bFirst = false;
}

void FrmBM::addOperator(int iButton)
{
   QString qsTmp;

   qsTmp = txtExpression->text();
   sLast.qsExpr = qsTmp;

   if ((iButton >= 1) && (iButton <= 5))
   {
      setUnaryOps(true);
      setBinaryOps(false);
      setNumbers(true);
      setProcess(false);
      setList(true);

      if (iPos > 0)
      {
         qsTmp.insert(iPos, bgOperators->button(iButton)->text());
         bgOperators->button(31)->setEnabled(true);
         bgOperators->button(32)->setEnabled(false);
         iPos = qsTmp.indexOf(')', iPos);
      }
      else
      {
         qsTmp.append(bgOperators->button(iButton)->text());
         bgOperators->button(31)->setEnabled(true);
         bgOperators->button(32)->setEnabled(false);
      }
   }
   else if ((iButton >= 6) && (iButton <= 30))
   {
      setUnaryOps(true);
      setBinaryOps(false);
      setNumbers(true);
      setProcess(false);
      setList(true);

      if (iPos > 0)
      {
         qsTmp.insert(iPos, QString("%1()").arg(bgOperators->button(iButton)->text()));
         bgOperators->button(31)->setEnabled(true);
         bgOperators->button(32)->setEnabled(false);
      }
      else
      {
         iPos = strlen(qsTmp.toStdString().c_str());
         qsTmp.append(QString("%1()").arg(bgOperators->button(iButton)->text()));
         bgOperators->button(31)->setEnabled(false);
         bgOperators->button(32)->setEnabled(false);
      }
      
      iPos = qsTmp.indexOf(')', iPos);
   }
   else if (iButton == 31)      //in paren
   {
      if (iPos > 0)
      {
         qsTmp.insert(iPos, bgOperators->button(iButton)->text());
         iPos = qsTmp.indexOf(')', iPos);
      }
      else
      {
         qsTmp.append("()");
         iPos = strlen(qsTmp.toStdString().c_str())-1;
      }
      
      setUnaryOps(true);
      setBinaryOps(false);
      setNumbers(true);
      setProcess(false);
      setList(true);

      bgOperators->button(31)->setEnabled(false);
      bgOperators->button(32)->setEnabled(true);
   }
   else if (iButton == 32)      //out paren
   {
      setUnaryOps(false);
      setBinaryOps(true);
      setNumbers(false);
      setProcess(true);
      setList(false);
      
      iPos = qsTmp.indexOf(')', iPos+1);
      if (iPos > 0)
      {
         bgOperators->button(31)->setEnabled(false);
         bgOperators->button(32)->setEnabled(true);
      }
      else
      {
         bgOperators->button(31)->setEnabled(false);
         bgOperators->button(32)->setEnabled(false);
      }
   }

   bFirst = false;

   setButtons(true);
   setExpression(qsTmp);
}

void FrmBM::undo()
{
   if (sLast.qsExpr == "")
   {
      clear();
   }
   else
   {
      setUnaryOps(sLast.bUnaryOps);
      setBinaryOps(sLast.bBinaryOps);
      setNumbers(sLast.bNumbers);
      setExpression(sLast.qsExpr);
      setButtons(sLast.bButtons);
      setProcess(sLast.bProcess);
      setList(sLast.bBands);
      btnUndo->setEnabled(false);
   }
}

void FrmBM::setUnaryOps(bool bVal)
{
   int i;

   sLast.bUnaryOps = btnSin->isEnabled();

   for (i = 6; i <= 30; i++)
   {
      bgOperators->button(i)->setEnabled(bVal);
   }
}

void FrmBM::setBinaryOps(bool bVal)
{
   int i;

   sLast.bBinaryOps = btnPlus->isEnabled();

   for (i = 1; i <= 5; i++)
   {
      bgOperators->button(i)->setEnabled(bVal);
   }
}

void FrmBM::setNumbers(bool bVal)
{
   sLast.bNumbers = btn1->isEnabled();

   QList<QAbstractButton*> numberButtons = bgNumbers->buttons();
   for (int i = 0; i < numberButtons.count(); ++i)
   {
      QAbstractButton* pButton = numberButtons[i];
      if (pButton != NULL)
      {
         pButton->setEnabled(bVal);
      }
   }
}

void FrmBM::setExpression(QString qsExpr)
{
   sLast.qsExpr = txtExpression->text();

   txtExpression->setText(qsExpr);
   adjustSize();
}

void FrmBM::setButtons(bool bVal)
{
   sLast.bButtons = btnClear->isEnabled();

   btnClear->setEnabled(bVal);
   btnUndo->setEnabled(bVal);
}

void FrmBM::setProcess(bool bVal)
{
   sLast.bProcess = btnProcess->isEnabled();

   btnProcess->setEnabled(bVal);
}

void FrmBM::setList(bool bVal)
{
   sLast.bBands = lisBands->isEnabled();

   lisBands->setEnabled(bVal);
}

unsigned int FrmBM::getLoadedBandNumber(unsigned int ulOrigBand) const
{
   if (mpDescriptor != NULL)
   {
      DimensionDescriptor bandDim = mpDescriptor->getOriginalBand(ulOrigBand-1);
      if (bandDim.isActiveNumberValid())
      {
         return bandDim.getActiveNumber();
      }
   }
   return 0;
}

void FrmBM::setCubeList(int iButton)
{
   clear();
   mpBandsLabel->clear();
   lisBands->clear();

   if (iButton == 0)
   {
      mpBandsLabel->setText("Bands:");
      lisBands->addItems(mBandList);
      cbResults->setEnabled(true);
   }
   else if (iButton == 1)
   {
      mpBandsLabel->setText("Cubes:");
      lisBands->addItems(mCubeList);
      cbResults->setEnabled(false);
   }
}

#endif
