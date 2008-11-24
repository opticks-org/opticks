/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QLayout>

#include "bm.h"
#include "bm.ui.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"

using namespace std;

FrmBM::FrmBM(const RasterDataDescriptor* pDescriptor, const vector<RasterElement*>& cubeList, QWidget* parent) :
   QDialog(parent),
   mpDescriptor(pDescriptor)
{
   setWindowTitle("Band Math");
   setModal(true);

   // Units
   QGroupBox* pUnitsGroup = new QGroupBox(this);

   bgUnits = new QButtonGroup(pUnitsGroup);

   rbDegrees = new QRadioButton("Degrees", pUnitsGroup);
   rbRadians = new QRadioButton("Radians", pUnitsGroup);
   bgUnits->addButton(rbDegrees, 0);
   bgUnits->addButton(rbRadians, 1);

   QHBoxLayout* pUnitsLayout = new QHBoxLayout(pUnitsGroup);
   pUnitsLayout->setMargin(10);
   pUnitsLayout->setSpacing(5);
   pUnitsLayout->addWidget(rbDegrees);
   pUnitsLayout->addWidget(rbRadians);

   // Cube
   QGroupBox* pCubeGroup = new QGroupBox(this);

   bgCube = new QButtonGroup(pCubeGroup);

   rbBand = new QRadioButton("Single Cube", pCubeGroup);
   rbCube = new QRadioButton("Multi Cube", pCubeGroup);
   bgCube->addButton(rbBand, 0);
   bgCube->addButton(rbCube, 1);

   QHBoxLayout* pCubeLayout = new QHBoxLayout(pCubeGroup);
   pCubeLayout->setMargin(10);
   pCubeLayout->setSpacing(5);
   pCubeLayout->addWidget(rbBand);
   pCubeLayout->addWidget(rbCube);

   QHBoxLayout* pRadioLayout = new QHBoxLayout();
   pRadioLayout->setMargin(0);
   pRadioLayout->setSpacing(10);
   pRadioLayout->addWidget(pUnitsGroup);
   pRadioLayout->addWidget(pCubeGroup);

   // Operators
   QGroupBox* pOperatorsGroup = new QGroupBox("Operators", this);

   bgOperators = new QButtonGroup(pOperatorsGroup);

   btnPlus = new QPushButton("+", pOperatorsGroup);
   btnPlus->setFixedSize(40, 40);
   bgOperators->addButton(btnPlus, 1);

   btnMinus = new QPushButton("-", pOperatorsGroup);
   btnMinus->setFixedSize(40, 40);
   bgOperators->addButton(btnMinus, 2);

   btnMultiply = new QPushButton("*", pOperatorsGroup);
   btnMultiply->setFixedSize(40, 40);
   bgOperators->addButton(btnMultiply, 3);

   btnDivide = new QPushButton("/", pOperatorsGroup);
   btnDivide->setFixedSize(40, 40);
   bgOperators->addButton(btnDivide, 4);

   btnPower = new QPushButton("^", pOperatorsGroup);
   btnPower->setFixedSize(40, 40);
   bgOperators->addButton(btnPower, 5);

   btnSqrt = new QPushButton("sqrt", pOperatorsGroup);
   btnSqrt->setFixedSize(40, 40);
   bgOperators->addButton(btnSqrt, 6);

   btnSin = new QPushButton("sin", pOperatorsGroup);
   btnSin->setFixedSize(40, 40);
   bgOperators->addButton(btnSin, 7);

   btnCos = new QPushButton("cos", pOperatorsGroup);
   btnCos->setFixedSize(40, 40);
   bgOperators->addButton(btnCos, 8);

   btnTan = new QPushButton("tan", pOperatorsGroup);
   btnTan->setFixedSize(40, 40);
   bgOperators->addButton(btnTan, 9);

   btnSec = new QPushButton("sec", pOperatorsGroup);
   btnSec->setFixedSize(40, 40);
   bgOperators->addButton(btnSec, 10);

   btnCsc = new QPushButton("csc", pOperatorsGroup);
   btnCsc->setFixedSize(40, 40);
   bgOperators->addButton(btnCsc, 11);

   btnCotan = new QPushButton("cot", pOperatorsGroup);
   btnCotan->setFixedSize(40, 40);
   bgOperators->addButton(btnCotan, 12);

   btnAsin = new QPushButton("asin", pOperatorsGroup);
   btnAsin->setFixedSize(40, 40);
   bgOperators->addButton(btnAsin, 13);

   btnAcos = new QPushButton("acos", pOperatorsGroup);
   btnAcos->setFixedSize(40, 40);
   bgOperators->addButton(btnAcos, 14);

   btnAtan = new QPushButton("atan", pOperatorsGroup);
   btnAtan->setFixedSize(40, 40);
   bgOperators->addButton(btnAtan, 15);

   btnAsec = new QPushButton("asec", pOperatorsGroup);
   btnAsec->setFixedSize(40, 40);
   bgOperators->addButton(btnAsec, 16);

   btnAcsc = new QPushButton("acsc", pOperatorsGroup);
   btnAcsc->setFixedSize(40, 40);
   bgOperators->addButton(btnAcsc, 17);

   btnAcotan = new QPushButton("acot", pOperatorsGroup);
   btnAcotan->setFixedSize(40, 40);
   bgOperators->addButton(btnAcotan, 18);

   btnSinh = new QPushButton("sinh", pOperatorsGroup);
   btnSinh->setFixedSize(40, 40);
   bgOperators->addButton(btnSinh, 19);

   btnCosh = new QPushButton("cosh", pOperatorsGroup);
   btnCosh->setFixedSize(40, 40);
   bgOperators->addButton(btnCosh, 20);

   btnTanh = new QPushButton("tanh", pOperatorsGroup);
   btnTanh->setFixedSize(40, 40);
   bgOperators->addButton(btnTanh, 21);

   btnSech = new QPushButton("sech", pOperatorsGroup);
   btnSech->setFixedSize(40, 40);
   bgOperators->addButton(btnSech, 22);

   btnCsch = new QPushButton("csch", pOperatorsGroup);
   btnCsch->setFixedSize(40, 40);
   bgOperators->addButton(btnCsch, 23);

   btnCotanh = new QPushButton("coth", pOperatorsGroup);
   btnCotanh->setFixedSize(40, 40);
   bgOperators->addButton(btnCotanh, 24);

   btnLog = new QPushButton("log", pOperatorsGroup);
   btnLog->setFixedSize(40, 40);
   bgOperators->addButton(btnLog, 25);

   btnLog2 = new QPushButton("log2", pOperatorsGroup);
   btnLog2->setFixedSize(40, 40);
   bgOperators->addButton(btnLog2, 26);

   btnLog10 = new QPushButton("log10", pOperatorsGroup);
   btnLog10->setFixedSize(40, 40);
   bgOperators->addButton(btnLog10, 27);

   btnExp = new QPushButton("exp", pOperatorsGroup);
   btnExp->setFixedSize(40, 40);
   bgOperators->addButton(btnExp, 28);

   btnAbs = new QPushButton("abs", pOperatorsGroup);
   btnAbs->setFixedSize(40, 40);
   bgOperators->addButton(btnAbs, 29);

   btnRand = new QPushButton("rand", pOperatorsGroup);
   btnRand->setFixedSize(40, 40);
   bgOperators->addButton(btnRand, 30);
   btnRand->hide();

   btnInParen = new QPushButton("( )", pOperatorsGroup);
   btnInParen->setFixedSize(40, 40);
   bgOperators->addButton(btnInParen, 31);

   btnOutParen = new QPushButton("( ) ->", pOperatorsGroup);
   btnOutParen->setFixedSize(40, 40);
   bgOperators->addButton(btnOutParen, 32);

   QGridLayout* pOperatorGrid = new QGridLayout(pOperatorsGroup);
   pOperatorGrid->setMargin(15);
   pOperatorGrid->setSpacing(5);
   pOperatorGrid->addWidget(btnPlus, 0, 0);
   pOperatorGrid->addWidget(btnMinus, 0, 1);
   pOperatorGrid->addWidget(btnMultiply, 0, 2);
   pOperatorGrid->addWidget(btnDivide, 0, 3);
   pOperatorGrid->addWidget(btnPower, 0, 4);
   pOperatorGrid->addWidget(btnSqrt, 0, 5);
   pOperatorGrid->addWidget(btnSin, 1, 0);
   pOperatorGrid->addWidget(btnCos, 1, 1);
   pOperatorGrid->addWidget(btnTan, 1, 2);
   pOperatorGrid->addWidget(btnSec, 1, 3);
   pOperatorGrid->addWidget(btnCsc, 1, 4);
   pOperatorGrid->addWidget(btnCotan, 1, 5);
   pOperatorGrid->addWidget(btnAsin, 2, 0);
   pOperatorGrid->addWidget(btnAcos, 2, 1);
   pOperatorGrid->addWidget(btnAtan, 2, 2);
   pOperatorGrid->addWidget(btnAsec, 2, 3);
   pOperatorGrid->addWidget(btnAcsc, 2, 4);
   pOperatorGrid->addWidget(btnAcotan, 2, 5);
   pOperatorGrid->addWidget(btnSinh, 3, 0);
   pOperatorGrid->addWidget(btnCosh, 3, 1);
   pOperatorGrid->addWidget(btnTanh, 3, 2);
   pOperatorGrid->addWidget(btnSech, 3, 3);
   pOperatorGrid->addWidget(btnCsch, 3, 4);
   pOperatorGrid->addWidget(btnCotanh, 3, 5);
   pOperatorGrid->addWidget(btnLog, 4, 0);
   pOperatorGrid->addWidget(btnLog2, 4, 1);
   pOperatorGrid->addWidget(btnLog10, 4, 2);
   pOperatorGrid->addWidget(btnExp, 4, 3);
   pOperatorGrid->addWidget(btnAbs, 4, 4);
   pOperatorGrid->addWidget(btnRand, 4, 5);
   pOperatorGrid->addWidget(btnInParen, 5, 2);
   pOperatorGrid->addWidget(btnOutParen, 5, 3);

   // Numbers
   QGroupBox* pNumbersGroup = new QGroupBox("Numbers", this);

   bgNumbers = new QButtonGroup(pNumbersGroup);

   btn1 = new QPushButton("1", pNumbersGroup);
   btn1->setFixedSize(40, 40);
   bgNumbers->addButton(btn1, 1);

   btn2 = new QPushButton("2", pNumbersGroup);
   btn2->setFixedSize(40, 40);
   bgNumbers->addButton(btn2, 2);

   btn3 = new QPushButton("3", pNumbersGroup);
   btn3->setFixedSize(40, 40);
   bgNumbers->addButton(btn3, 3);

   btn4 = new QPushButton("4", pNumbersGroup);
   btn4->setFixedSize(40, 40);
   bgNumbers->addButton(btn4, 4);

   btn5 = new QPushButton("5", pNumbersGroup);
   btn5->setFixedSize(40, 40);
   bgNumbers->addButton(btn5, 5);

   btn6 = new QPushButton("6", pNumbersGroup);
   btn6->setFixedSize(40, 40);
   bgNumbers->addButton(btn6, 6);

   btn7 = new QPushButton("7", pNumbersGroup);
   btn7->setFixedSize(40, 40);
   bgNumbers->addButton(btn7, 7);

   btn8 = new QPushButton("8", pNumbersGroup);
   btn8->setFixedSize(40, 40);
   bgNumbers->addButton(btn8, 8);

   btn9 = new QPushButton("9", pNumbersGroup);
   btn9->setFixedSize(40, 40);
   bgNumbers->addButton(btn9, 9);

   btnPI = new QPushButton("p", pNumbersGroup);
   btnPI->setFixedSize(40, 40);

   QFont ftPiButton("symbol", 12);      // Lower case first letter for Unix support
   btnPI->setFont(ftPiButton);

   bgNumbers->addButton(btnPI, 11);

   btn0 = new QPushButton("0", pNumbersGroup);
   btn0->setFixedSize(40, 40);
   bgNumbers->addButton(btn0, 0);

   btnE = new QPushButton("e", pNumbersGroup);
   btnE->setFixedSize(40, 40);
   bgNumbers->addButton(btnE, 12);

   btnDecimal = new QPushButton(".", pNumbersGroup);
   btnDecimal->setFixedSize(40, 40);
   bgNumbers->addButton(btnDecimal, 10);

   QGridLayout* pNumbersGrid = new QGridLayout(pNumbersGroup);
   pNumbersGrid->setMargin(15);
   pNumbersGrid->setSpacing(5);
   pNumbersGrid->addWidget(btn1, 0, 0);
   pNumbersGrid->addWidget(btn2, 0, 1);
   pNumbersGrid->addWidget(btn3, 0, 2);
   pNumbersGrid->addWidget(btn4, 1, 0);
   pNumbersGrid->addWidget(btn5, 1, 1);
   pNumbersGrid->addWidget(btn6, 1, 2);
   pNumbersGrid->addWidget(btn7, 2, 0);
   pNumbersGrid->addWidget(btn8, 2, 1);
   pNumbersGrid->addWidget(btn9, 2, 2);
   pNumbersGrid->addWidget(btnPI, 3, 0);
   pNumbersGrid->addWidget(btn0, 3, 1);
   pNumbersGrid->addWidget(btnE, 3, 2);
   pNumbersGrid->addWidget(btnDecimal, 4, 1);

   // Bands
   mpBandsLabel = new QLabel(this);
   lisBands = new QListWidget(this);
   lisBands->setMinimumWidth(150);

   QVBoxLayout* pBandsLayout = new QVBoxLayout();
   pBandsLayout->setMargin(0);
   pBandsLayout->setSpacing(5);
   pBandsLayout->addWidget(mpBandsLabel);
   pBandsLayout->addWidget(lisBands, 10);

   // Expression
   txtExpression = new QLabel(this);
   txtExpression->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   txtExpression->setWordWrap(true);
   txtExpression->setFixedWidth(465);

   QFont txtExpression_font(txtExpression->font());
   txtExpression_font.setBold(true);
   txtExpression->setFont(txtExpression_font);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   btnClear = new QPushButton("C&lear", this);
   btnUndo = new QPushButton("&Undo", this);
   btnProcess = new QPushButton("&Process", this);
   QPushButton* btnCancel = new QPushButton("&Cancel", this);
   cbResults = new QCheckBox("Overlay Results", this);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(6);
   pButtonLayout->addWidget(btnClear);
   pButtonLayout->addWidget(btnUndo);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(cbResults);
   pButtonLayout->addWidget(btnProcess);
   pButtonLayout->addWidget(btnCancel);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addLayout(pRadioLayout, 0, 0, 1, 2);
   pGrid->addWidget(pOperatorsGroup, 1, 0);
   pGrid->addWidget(pNumbersGroup, 1, 1);
   pGrid->addLayout(pBandsLayout, 0, 2, 3, 1);
   pGrid->addWidget(txtExpression, 2, 0, 1, 2);
   pGrid->addWidget(pLine, 3, 0, 1, 3);
   pGrid->addLayout(pButtonLayout, 4, 0, 1, 3);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   connect(bgCube, SIGNAL(buttonClicked(int)), this, SLOT(setCubeList(int)));
   connect(bgOperators, SIGNAL(buttonClicked(int)), this, SLOT(addOperator(int)));
   connect(bgNumbers, SIGNAL(buttonClicked(int)), this, SLOT(addNumber(int)));
   connect(lisBands, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addBand()));
   connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
   connect(btnUndo, SIGNAL(clicked()), this, SLOT(undo()));
   connect(btnProcess, SIGNAL(clicked()), this, SLOT(accept()));
   connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

   const vector<DimensionDescriptor>& loadedBands = pDescriptor->getBands();

   const DynamicObject* pMetadata = pDescriptor->getMetadata();
   const vector<double>* pCenterWavelengths = NULL;
   if (pMetadata != NULL)
   {
      string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, CENTER_WAVELENGTHS_METADATA_NAME,
         END_METADATA_NAME };
      pCenterWavelengths = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(pCenterPath));
   }

   vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);

   // Initialize
   for (vector<string>::size_type counter = 0; counter < bandNames.size(); ++counter)
   {
      string bandName = bandNames[counter];
      QString strBandInfo;
      if (pCenterWavelengths != NULL)
      {
         double dWavelength = 0;
         if (counter < pCenterWavelengths->size())
         {
            dWavelength = (*pCenterWavelengths)[counter];
         }
         // Set the list box string
         strBandInfo = QString::fromStdString(bandName) + QString("    %1").arg(dWavelength);
      }
      else
      {
         strBandInfo = QString::fromStdString(bandName);
      }
      mBandList.append(strBandInfo);
   }

   for (unsigned int i = 0; i < cubeList.size(); i++)
   {
      DataElement* pElement = cubeList[i];
      if (pElement != NULL)
      {
         // Set the list box string
         QString strCubeInfo;
         strCubeInfo.sprintf("Cube %-6d    %s", i + 1, pElement->getName().c_str());

         if (strCubeInfo.isEmpty() == false)
         {
            mCubeList.append(strCubeInfo);
         }
      }
   }

   clear();
   lisBands->clear();
   rbRadians->setChecked(true);
   rbBand->setChecked(true);
   setCubeList(0);
}

FrmBM::~FrmBM()
{
}
