/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AspamPlotSelectionDialog.h"

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

namespace
{
const int sMarginSize = 10;
};

AspamPlotSelectionDialog::AspamPlotSelectionDialog(QStringList &choices, QWidget *pParent) :
                  QDialog(pParent)
{
   setObjectName("Selection Dialog");
   setWindowTitle("Aspam Plot Parameter Selection");
   this->setModal(true);

   setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

   QLabel *pPrimaryLabel = new QLabel("Choose the primary axis.", this);
   QLabel *pPlotNameLabel = new QLabel("Add to plot", this);

   mpPrimary = new QComboBox(this);
   mpPrimary->addItems(choices);

   mpPlotName = new QComboBox(this);
   mpPlotName->addItem("New Plot");

   QPushButton *pOk = new QPushButton("Ok", this);
   pOk->setDefault(true);
   QPushButton *pCancel = new QPushButton("Cancel", this);

   // Layout
   QGridLayout *pTopLayout = new QGridLayout(this);
   pTopLayout->setMargin(sMarginSize);
   pTopLayout->setSpacing(sMarginSize);
   pTopLayout->addWidget(pPrimaryLabel, 0, 0);
   pTopLayout->addWidget(mpPrimary, 0, 1);
   pTopLayout->addWidget(pPlotNameLabel, 2, 0);
   pTopLayout->addWidget(mpPlotName, 2, 1);

   QHBoxLayout *pButtonLayout = new QHBoxLayout;
   pTopLayout->addLayout(pButtonLayout, 3, 0, 1, 2);
   pButtonLayout->addStretch(1);
   pButtonLayout->addWidget(pOk, 0);
   pButtonLayout->addWidget(pCancel, 0);

   connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

QString AspamPlotSelectionDialog::getPrimaryAxis() const
{
   return mpPrimary->currentText();
}

QString AspamPlotSelectionDialog::getPlotName() const
{
   return mpPlotName->currentText();
}

void AspamPlotSelectionDialog::setPlotNames(const QStringList &names, const QString &selected)
{
   mpPlotName->clear();
   mpPlotName->addItem("New Plot");
   mpPlotName->addItems(names);
   for(int i = 0; i < mpPlotName->count(); i++)
   {
      if(mpPlotName->itemText(i) == selected)
      {
         mpPlotName->setCurrentIndex(i);
         break;
      }
   }
}
