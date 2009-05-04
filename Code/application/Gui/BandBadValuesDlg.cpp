/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

#include "BandBadValuesDlg.h"
#include "BandBadValuesValidator.h"
#include "StringUtilities.h"

#include <string>
using namespace std;

BandBadValuesDlg::BandBadValuesDlg(QWidget* parent) :
   QDialog(parent)
{
   // create a grid layout for the entire dialog with
   // five rows and 2 columns
   QGridLayout* pDialogLayout = new QGridLayout(this);
   pDialogLayout->setMargin(10);
   pDialogLayout->setSpacing(5);

   //create a label and put it into row 0, column 0
   QString labelTxt = QString("Bad Values: ");
   pDialogLayout->addWidget(new QLabel(labelTxt, this), 0, 0); 

   //create the text box that the user enters bad values
   //into and put it into row 0, column 1
   mpBandBadValue = new QLineEdit(this);
   QValidator* validator = new BandBadValuesValidator(this);
   mpBandBadValue->setValidator(validator);
   pDialogLayout->addWidget(mpBandBadValue, 0, 1);

   //set the column stretch for column 1, so that
   //the text box will get bigger and the label will
   //remain the same size as the user resizes
   pDialogLayout->setColumnStretch(1, 10);

   //set the row stretch for the empty row 2 so
   //the label, text box and check box will remain
   //at the top and the button will remain at the bottom
   //during resize
   pDialogLayout->setRowStretch(2, 10);

   //create a check box and place it into all of row 1
   mpChkBoxAllBands = new QCheckBox("Set Bad Values for all Bands in Layer", this);
   pDialogLayout->addWidget(mpChkBoxAllBands, 1, 0, 1, 2);

   //create a horizontal, sunken line and place it into all of row 3
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::Sunken | QFrame::HLine);
   pDialogLayout->addWidget(pLine, 3, 0, 1, 2);

   //create an 'OK' and 'Cancel' button, but first put them
   //into their own QHBoxLayout and use that for all of
   //row 4
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   QString okTxt = QString("OK");
   QString cancelTxt = QString("Cancel");
   pButtonLayout->addStretch();
   QPushButton* pOkBtn = new QPushButton(okTxt, this);
   QPushButton* pCancelBtn = new QPushButton(cancelTxt, this);
   pButtonLayout->addWidget(pOkBtn);
   pButtonLayout->addWidget(pCancelBtn);
   pDialogLayout->addLayout(pButtonLayout, 4, 0, 1, 2);

   // Initialization
   setWindowTitle("Band Bad Values...");
   setModal(true);

   //connect the button clicks up to the appropriate
   //signals
   connect(pOkBtn, SIGNAL(clicked()), this, SLOT(okPressed()));
   connect(pCancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

BandBadValuesDlg::~BandBadValuesDlg()
{
}

void BandBadValuesDlg::setBadValues(const std::vector<int> & badValues)
{
   if (badValues != mBadValues)
   {
      //upate the internal vector and
      //then convert the vector into text
      //that be displayed to the user in text box
      //that they can edit.
      mBadValues = badValues;
      QString value = BandBadValuesValidator::convertVectorToString(badValues);
      mpBandBadValue->setText(value);
   }
}

void BandBadValuesDlg::getBadValues(std::vector<int>& badValues) const
{
   badValues = mBadValues;
}

void BandBadValuesDlg::setChangeForAllBands(bool & value)
{
   if (value != mpChkBoxAllBands->isChecked())
   {
      mpChkBoxAllBands->setChecked(value);
   }
}

bool BandBadValuesDlg::getChangeForAllBands() const
{
   return mpChkBoxAllBands->isChecked();
}

void BandBadValuesDlg::okPressed()
{
   bool conversionSuccess;
   vector<int> enteredValues;
   QString tmp = mpBandBadValue->text();
   enteredValues = BandBadValuesValidator::convertStringToVector(tmp, conversionSuccess);
   if (conversionSuccess)
   {
      //change the vector of ints returned from this dialog since the
      //user entered valid text and then close this dialog but
      //calling the accept() slot
      mBadValues = enteredValues;
      this->accept();
   }
   else
   {
      //the user did not enter valid text, so convert the last
      //good vector back into text and overwrite the user's
      //input
      mpBandBadValue->setText(BandBadValuesValidator::convertVectorToString(mBadValues));
   }
}
