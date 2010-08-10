/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "PaperSizeDlg.h"

static int findSize(double height, double width);

PaperSizeDlg::PaperSizeDlg(double dWidth, double dHeight, QWidget* pParent) :
   QDialog(pParent)
{
   // Size
   QGroupBox* pSizeGroup = new QGroupBox("Size", this);

   QLabel* pTypeLabel = new QLabel("Paper Type:", pSizeGroup);

   mpTypeCombo = new QComboBox(pSizeGroup);
   mpTypeCombo->setEditable(false);
   mpTypeCombo->addItem("Letter (8.5\"x11\")");
   mpTypeCombo->addItem("Tabloid (11\"x17\")");
   mpTypeCombo->addItem("Executive (10\"x7.5\")");
   mpTypeCombo->addItem("Legal (8.5\"x14\")");
   mpTypeCombo->addItem("Folio (210 x 330 mm)");
   mpTypeCombo->addItem("DLE (110 x 220 mm)");
   mpTypeCombo->addItem("A0 (841 x 1189 mm)");
   mpTypeCombo->addItem("A1 (594 x 841 mm)");
   mpTypeCombo->addItem("A2 (420 x 594 mm)");
   mpTypeCombo->addItem("A3 (297 x 420 mm)");
   mpTypeCombo->addItem("A4 (210 x 297 mm)");
   mpTypeCombo->addItem("A5 (148 x 210 mm)");
   mpTypeCombo->addItem("A6 (105 x 148 mm)");
   mpTypeCombo->addItem("A7 (74 x 105 mm)");
   mpTypeCombo->addItem("A8 (52 x 74 mm)");
   mpTypeCombo->addItem("A9 (37 x 52 mm)");
   mpTypeCombo->addItem("B0 (1030 x 1456 mm)");
   mpTypeCombo->addItem("B1 (728 x 1030 mm)");
   mpTypeCombo->addItem("B2 (515 x 728 mm)");
   mpTypeCombo->addItem("B3 (364 x 515 mm)");
   mpTypeCombo->addItem("B4 (257 x 364 mm)");
   mpTypeCombo->addItem("B5 (182 x 257 mm)");
   mpTypeCombo->addItem("B6 (128 x 182 mm)");
   mpTypeCombo->addItem("B7 (91 x 128 mm)");
   mpTypeCombo->addItem("B8 (64 x 91 mm)");
   mpTypeCombo->addItem("B9 (45 x 64 mm)");
   mpTypeCombo->addItem("B10 (32 x 45 mm)");

   // Orientation
   QGroupBox* pOrientationGroup = new QGroupBox("Orientation", this);
   mpPortraitRadio = new QRadioButton("Portrait", pOrientationGroup);
   mpLandscapeRadio = new QRadioButton("Landscape", pOrientationGroup);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pSizeLayout = new QHBoxLayout(pSizeGroup);
   pSizeLayout->setMargin(10);
   pSizeLayout->setSpacing(5);
   pSizeLayout->addWidget(pTypeLabel);
   pSizeLayout->addWidget(mpTypeCombo);

   QVBoxLayout* pOrientationLayout = new QVBoxLayout(pOrientationGroup);
   pOrientationLayout->setMargin(10);
   pOrientationLayout->setSpacing(5);
   pOrientationLayout->addWidget(mpPortraitRadio);
   pOrientationLayout->addWidget(mpLandscapeRadio);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pSizeGroup, 0, Qt::AlignLeft);
   pLayout->addWidget(pOrientationGroup, 0, Qt::AlignLeft);
   pLayout->addStretch();
   pLayout->addWidget(pLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Paper Size");
   setModal(true);
   resize(300, 200);

   mpTypeCombo->setCurrentIndex(findSize(dWidth, dHeight));

   if (dWidth > dHeight)
   {
      mpLandscapeRadio->setChecked(true);
   }
   else
   {
      mpPortraitRadio->setChecked(true);
   }

   // Connections
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

PaperSizeDlg::~PaperSizeDlg()
{
}

class PaperType
{
public:
   double height;
   double width;
};

const PaperType types[] =
{
   11.0, 8.5,                // "Letter (8.5\"x11\")");
   17.0, 11.0,               // "Tabloid (11\"x17\")");
   10.0, 7.5,                // "Executive (10\"x7.5\")");
   14.0, 8.5,                // "Legal (8.5\"x14\")");
   330.0/25.4, 210.0/25.4,   // "Folio (210 x 330 mm)");
   220.0/25.4, 110.0/25.4,   // "DLE (110 x 220 mm)");
   1189.0/25.4, 841.0/25.4,  // "A0 (841 x 1189 mm)");
   841.0/25.4, 594.0/25.4,   // "A1 (594 x 841 mm)");
   594.0/25.4, 420.0/25.4,   // "A2 (420 x 594 mm)");
   420.0/25.4, 297.0/25.4,   // "A3 (297 x 420 mm)");
   297.0/25.4, 210.0/25.4,   // "A4 (210 x 297 mm)");
   210.0/25.4, 148.0/25.4,   // "A5 (148 x 210 mm)");
   148.0/25.4, 105.0/25.4,   // "A6 (105 x 148 mm)");
   105.0/25.4, 74.0/25.4,    // "A7 (74 x 105 mm)");
   74.0/25.4, 52.0/25.4,     // "A8 (52 x 74 mm)");
   52.0/25.4, 37.0/25.4,     // "A9 (37 x 52 mm)");
   1456.0/25.4, 1030.0/25.4, // "B0 (1030 x 1456 mm)");
   1030.0/25.4, 728.0/25.4,  // "B1 (728 x 1030 mm)");
   728.0/25.4, 515.0/25.4,   // "B2 (515 x 728 mm)");
   515.0/25.4, 364.0/25.4,   // "B3 (364 x 515 mm)");
   364.0/25.4, 257.0/25.4,   // "B4 (257 x 364 mm)");
   257.0/25.4, 182.0/25.4,   // "B5 (182 x 257 mm)");
   182.0/25.4, 128.0/25.4,   // "B6 (128 x 182 mm)");
   128.0/25.4, 91.0/25.4,    // "B7 (91 x 128 mm)");
   91.0/25.4, 64.0/25.4,     // "B8 (64 x 91 mm)");
   64.0/25.4, 45.0/25.4,     // "B9 (45 x 64 mm)");
   45.0/25.4, 32.0/25.4      // "B10 (32 x 45 mm)");
};

void PaperSizeDlg::getSize(double& dWidth, double& dHeight) const
{
   dWidth = types[mpTypeCombo->currentIndex()].width;
   dHeight = types[mpTypeCombo->currentIndex()].height;

   if (mpLandscapeRadio->isChecked() == true)
   {
      double swap = dWidth;
      dWidth = dHeight;
      dHeight = swap;
   }
}

static int findSize(double height, double width)
{
   if (height < width)
   {
      double temp = height;
      height = width;
      width = temp;
   }

   int i;
   const double fudge = 0.05;
   for (i = 0; i < sizeof(types) / sizeof(types[0]); i++)
   {
      if (fabs(height-types[i].height) < fudge && fabs(width-types[i].width) < fudge)
      {
         return i;
      }
   }
   return 0;
}
