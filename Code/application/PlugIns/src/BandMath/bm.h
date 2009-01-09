/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRMBM_H
#define FRMBM_H

#include <QtCore/QStringList>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTextEdit>

#include <vector>

class RasterElement;
class RasterDataDescriptor;

class FrmBM : public QDialog
{
   Q_OBJECT

public:
   FrmBM(const RasterDataDescriptor* pDescriptor, const std::vector<RasterElement*>& cubeList, QWidget* parent = 0);
   ~FrmBM();

   QString getExpression(bool bLoadedBands) const;
   bool isDegrees() const;
   bool isMultiCube() const;
   bool isResultsMatrix() const;

protected slots:
   virtual void setBinaryOps(bool bVal);
   virtual void setButtons(bool bVal);
   virtual void setExpression(QString qsExpr);
   virtual void setList(bool bVal);
   virtual void setNumbers(bool bVal);
   virtual void setProcess(bool bVal);
   virtual void setUnaryOps(bool bVal);

   virtual void setCubeList(int iButton);
   virtual void addOperator(int iButton);
   virtual void addNumber(int iButton);
   virtual void addBand();
   virtual void clear();
   virtual void undo();

private:
   // Units
   QButtonGroup* bgUnits;
   QRadioButton* rbDegrees;
   QRadioButton* rbRadians;

   // Cube
   QButtonGroup* bgCube;
   QRadioButton* rbBand;
   QRadioButton* rbCube;
   QCheckBox* cbResults;

   // Operators
   QButtonGroup* bgOperators;
   QPushButton* btnPlus;
   QPushButton* btnMinus;
   QPushButton* btnMultiply;
   QPushButton* btnDivide;
   QPushButton* btnPower;
   QPushButton* btnSqrt;
   QPushButton* btnSin;
   QPushButton* btnCos;
   QPushButton* btnTan;
   QPushButton* btnSec;
   QPushButton* btnCsc;
   QPushButton* btnCotan;
   QPushButton* btnAsin;
   QPushButton* btnAcos;
   QPushButton* btnAtan;
   QPushButton* btnAsec;
   QPushButton* btnAcsc;
   QPushButton* btnAcotan;
   QPushButton* btnSinh;
   QPushButton* btnCosh;
   QPushButton* btnTanh;
   QPushButton* btnSech;
   QPushButton* btnCsch;
   QPushButton* btnCotanh;
   QPushButton* btnLog;
   QPushButton* btnLog2;
   QPushButton* btnLog10;
   QPushButton* btnExp;
   QPushButton* btnAbs;
   QPushButton* btnRand;
   QPushButton* btnInParen;
   QPushButton* btnOutParen;

   // Numbers
   QButtonGroup* bgNumbers;
   QPushButton* btn1;
   QPushButton* btn2;
   QPushButton* btn3;
   QPushButton* btn4;
   QPushButton* btn5;
   QPushButton* btn6;
   QPushButton* btn7;
   QPushButton* btn8;
   QPushButton* btn9;
   QPushButton* btnPI;
   QPushButton* btn0;
   QPushButton* btnE;
   QPushButton* btnDecimal;

   // List
   QLabel* mpBandsLabel;
   QListWidget* lisBands;
   QStringList mBandList;
   QStringList mCubeList;

   // Expression
   QTextEdit* txtExpression;

   // Buttons
   QPushButton* btnClear;
   QPushButton* btnUndo;
   QPushButton* btnProcess;
   const RasterDataDescriptor* mpDescriptor;

   unsigned int getLoadedBandNumber(unsigned int ulOrigBand) const;
};

#endif // FRMBM_H
