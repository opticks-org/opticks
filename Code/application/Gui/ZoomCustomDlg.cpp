/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QDoubleValidator>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "LocationType.h"
#include "ZoomCustomDlg.h"

ZoomCustomDlg::ZoomCustomDlg(QWidget* parent) :
   QDialog(parent)
{
   // X-coordinates
   QFont boldFont = font();
   boldFont.setBold(true);

   QLabel* pXLabel = new QLabel("X-Coordinates", this);
   pXLabel->setFont(boldFont);

   QFrame* pXLine = new QFrame(this);
   pXLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QLabel* pMinXLabel = new QLabel("Min:", this);
   QLabel* pMaxXLabel = new QLabel("Max:", this);

   mpMinXEdit = new QLineEdit(this);
   mpMinXEdit->setValidator(new QDoubleValidator(mpMinXEdit));
   mpMinXEdit->setFixedWidth(75);

   mpMaxXEdit = new QLineEdit(this);
   mpMaxXEdit->setValidator(new QDoubleValidator(mpMaxXEdit));
   mpMaxXEdit->setFixedWidth(75);

   // Y-coordinates
   QLabel* pYLabel = new QLabel("Y-Coordinates", this);
   pYLabel->setFont(boldFont);

   QFrame* pYLine = new QFrame(this);
   pYLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QLabel* pMinYLabel = new QLabel("Min:", this);
   QLabel* pMaxYLabel = new QLabel("Max:", this);

   mpMinYEdit = new QLineEdit(this);
   mpMinYEdit->setValidator(new QDoubleValidator(mpMinYEdit));
   mpMinYEdit->setFixedWidth(75);

   mpMaxYEdit = new QLineEdit(this);
   mpMaxYEdit->setValidator(new QDoubleValidator(mpMaxYEdit));
   mpMaxYEdit->setFixedWidth(75);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pXLayout = new QHBoxLayout();
   pXLayout->setMargin(0);
   pXLayout->setSpacing(5);
   pXLayout->addWidget(pXLabel);
   pXLayout->addWidget(pXLine, 10);

   QHBoxLayout* pYLayout = new QHBoxLayout();
   pYLayout->setMargin(0);
   pYLayout->setSpacing(5);
   pYLayout->addWidget(pYLabel);
   pYLayout->addWidget(pYLine, 10);

   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pOk);
   pButtonLayout->addWidget(pCancel);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addLayout(pXLayout, 0, 0, 1, 2);
   pGrid->addWidget(pMinXLabel, 1, 0);
   pGrid->addWidget(mpMinXEdit, 1, 1, Qt::AlignLeft);
   pGrid->addWidget(pMaxXLabel, 2, 0);
   pGrid->addWidget(mpMaxXEdit, 2, 1, Qt::AlignLeft);
   pGrid->setRowMinimumHeight(3, 10);
   pGrid->addLayout(pYLayout, 4, 0, 1, 2);
   pGrid->addWidget(pMinYLabel, 5, 0);
   pGrid->addWidget(mpMinYEdit, 5, 1, Qt::AlignLeft);
   pGrid->addWidget(pMaxYLabel, 6, 0);
   pGrid->addWidget(mpMaxYEdit, 6, 1, Qt::AlignLeft);
   pGrid->addWidget(pHLine, 7, 0, 1, 2, Qt::AlignBottom);
   pGrid->addLayout(pButtonLayout, 8, 0, 1, 2);
   pGrid->setRowStretch(7, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   setWindowTitle("Custom Zoom");
   setModal(true);
   resize(350, 225);

   // Connections
   connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

ZoomCustomDlg::~ZoomCustomDlg()
{
}

void ZoomCustomDlg::setZoomBox(const LocationType& llCorner, const LocationType& urCorner)
{
   mpMinXEdit->setText(QString::number(llCorner.mX));
   mpMinYEdit->setText(QString::number(llCorner.mY));
   mpMaxXEdit->setText(QString::number(urCorner.mX));
   mpMaxYEdit->setText(QString::number(urCorner.mY));
}

void ZoomCustomDlg::getZoomBox(LocationType& llCorner, LocationType& urCorner) const
{
   QString strMinX = mpMinXEdit->text();
   QString strMinY = mpMinYEdit->text();
   QString strMaxX = mpMaxXEdit->text();
   QString strMaxY = mpMaxYEdit->text();

   llCorner.mX = strMinX.toDouble();
   llCorner.mY = strMinY.toDouble();
   urCorner.mX = strMaxX.toDouble();
   urCorner.mY = strMaxY.toDouble();
}

void ZoomCustomDlg::accept()
{
   QString strMinX = mpMinXEdit->text();
   if (strMinX.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a value for the minimum x-coordinate!");
      mpMinXEdit->setFocus();
      return;
   }

   QString strMinY = mpMinYEdit->text();
   if (strMinY.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a value for the minimum y-coordinate!");
      mpMinYEdit->setFocus();
      return;
   }

   QString strMaxX = mpMaxXEdit->text();
   if (strMaxX.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a value for the maximum x-coordinate!");
      mpMaxXEdit->setFocus();
      return;
   }

   QString strMaxY = mpMaxYEdit->text();
   if (strMaxY.isEmpty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please enter a value for the maximum y-coordinate!");
      mpMaxYEdit->setFocus();
      return;
   }

   QDialog::accept();
}
