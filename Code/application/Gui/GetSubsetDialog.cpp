/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GetSubsetDialog.h"
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>

GetSubsetDialog::GetSubsetDialog(const QString& plotName,
                                 const QStringList& aoiNames,
                                 const QStringList& bands,
                                 const std::vector<int>& defaultSelection,
                                 QWidget* pParent) : QDialog(pParent)
{
   setWindowTitle("Select a subset");

   mpName = new QLineEdit(this);
   mpName->setText(plotName);

   mpAoiSelect = new QComboBox(this);
   mpAoiSelect->setEditable(false);
   mpAoiSelect->addItem("<no AOI>");
   mpAoiSelect->addItems(aoiNames);

   mpBandSelect = new QListWidget(this);
   mpBandSelect->setResizeMode(QListView::Adjust);
   mpBandSelect->setFlow(QListWidget::TopToBottom);
   mpBandSelect->setUniformItemSizes(true);
   mpBandSelect->setSelectionMode(QListWidget::ExtendedSelection);
   mpBandSelect->addItems(bands);
   mpBandSelect->setWrapping(true);
   if (defaultSelection.empty())
   {
      mpBandSelect->selectAll();
   }
   else
   {
      mpBandSelect->clearSelection();
      for (std::vector<int>::const_iterator idx = defaultSelection.begin(); idx != defaultSelection.end(); ++idx)
      {
         mpBandSelect->item(*idx)->setSelected(true);
      }
   }

   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     Qt::Horizontal, this);

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->addWidget(new QLabel("Plot name:", this), 0, 0);
   pLayout->addWidget(mpName, 0, 1);
   pLayout->addWidget(new QLabel("Select spatial subset:", this), 1, 0);
   pLayout->addWidget(mpAoiSelect, 1, 1);
   pLayout->addWidget(new QLabel("Select band subset:", this), 2, 0);
   pLayout->addWidget(mpBandSelect, 3, 0, 1, 2);
   pLayout->setRowStretch(3, 1);
   pLayout->addWidget(pButtons, 4, 0, 1, 2);

   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));
}

GetSubsetDialog::~GetSubsetDialog()
{
}

QString GetSubsetDialog::getPlotName() const
{
   return mpName->text();
}

QString GetSubsetDialog::getSelectedAoi() const
{
   if (mpAoiSelect->currentIndex() == 0)
   {
      // no AOI selected
      return QString();
   }
   return mpAoiSelect->currentText();
}

std::vector<int> GetSubsetDialog::getBandSelectionIndices() const
{
   std::vector<int> bands;
   QList<QListWidgetItem*> rows = mpBandSelect->selectedItems();
   foreach(QListWidgetItem* pRow, rows)
   {
      bands.push_back(mpBandSelect->row(pRow));
   }
   return bands;
}