/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicSymbolWidget.h"
#include "AppVerify.h"
#include "DesktopServices.h"

#include <QtGui/QListWidget>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QSpinBox>

#include <vector>
#include <string>

using namespace std;

const QString GraphicSymbolWidget::NO_SYMBOL = "No symbol";

GraphicSymbolWidget::GraphicSymbolWidget(QWidget *pParent) : QWidget(pParent)
{
   Service<DesktopServices> pDesktop;
   const vector<string> &symbolNames = pDesktop->getAvailableSymbolNames();
   
   QLabel *pSymbolListLabel = new QLabel("Available symbols:", this);
   mpSymbolList = new QListWidget(this);
   mpSymbolList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpSymbolList->setWrapping(true);
   mpSymbolList->setFlow(QListView::TopToBottom);
   mpSymbolList->setMovement(QListView::Static);
   mpSymbolList->setResizeMode(QListView::Adjust);

   mpSymbolList->addItem(NO_SYMBOL);
   for (vector<string>::const_iterator iter = symbolNames.begin();
      iter != symbolNames.end(); ++iter)
   {
      mpSymbolList->addItem(QString::fromStdString(*iter));
   }

   mpSymbolPreview = new QLabel(this);
   mpSymbolPreview->setAlignment(Qt::AlignCenter);
   mpSymbolPreview->setWordWrap(true);
   
   QLabel *pSymbolSizeLabel = new QLabel("Symbol size:", this);
   mpSymbolSizeSpin = new QSpinBox(this);
   mpSymbolSizeSpin->setRange(0, 500);

   // Layout
   QGridLayout *pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pSymbolListLabel, 0, 0, Qt::AlignTop);
   pLayout->addWidget(mpSymbolList, 0, 1);
   pLayout->addWidget(mpSymbolPreview, 0, 2);
   pLayout->addWidget(pSymbolSizeLabel, 1, 0);
   pLayout->addWidget(mpSymbolSizeSpin, 1, 1, 1, 2, Qt::AlignLeft);
   pLayout->setRowStretch(0, 10);
   pLayout->setColumnStretch(1, 10);

   // Initialization
   setSymbolName(NO_SYMBOL);
   updateSelection();

   // Connections
   VERIFYNR(connect(mpSymbolList, SIGNAL(currentTextChanged(const QString&)), this,
      SIGNAL(nameChanged(const QString&))));
   VERIFYNR(connect(mpSymbolList, SIGNAL(itemSelectionChanged()), this, SLOT(updateSelection())));
   VERIFYNR(connect(mpSymbolSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(notifySizeChange(int))));
}

GraphicSymbolWidget::~GraphicSymbolWidget()
{
}

void GraphicSymbolWidget::setSymbolName(const QString &name)
{
   if (name == getSymbolName())
   {
      return;
   }

   QString findName = name;
   if (findName.isEmpty())
   {
      findName = NO_SYMBOL;
   }

   QList<QListWidgetItem*> items = 
      mpSymbolList->findItems(findName, Qt::MatchExactly);

   VERIFYNRV(items.size() == 1);

   QListWidgetItem *pOldItem = getSelectedItem();
   if (pOldItem != NULL)
   {
      mpSymbolList->setItemSelected(pOldItem, false);
   }
   mpSymbolList->setItemSelected(items.front(), true);
}

QString GraphicSymbolWidget::getSymbolName() const
{
   QString name;

   QListWidgetItem *pItem = getSelectedItem();
   if (pItem != NULL)
   {
      QString qName = pItem->text();
      if (qName != NO_SYMBOL)
      {
         name = qName;
      }
   }
   return name;
}

void GraphicSymbolWidget::setSymbolSize(unsigned int size)
{
   if (size != getSymbolSize())
   {
      mpSymbolSizeSpin->setValue(size);
   }
}

unsigned int GraphicSymbolWidget::getSymbolSize() const
{
   return mpSymbolSizeSpin->value();
}

QListWidgetItem *GraphicSymbolWidget::getSelectedItem() const
{
   QList<QListWidgetItem*> selectedItems = mpSymbolList->selectedItems();
   if (selectedItems.empty())
   {
      return NULL;
   }

   VERIFY(selectedItems.size() == 1);

   return selectedItems.front();
}

void GraphicSymbolWidget::updateSelection()
{
   QListWidgetItem *pItem = getSelectedItem();
   if (pItem == NULL || pItem->text() == NO_SYMBOL)
   {
      mpSymbolPreview->setText("No symbol selected");
   }
   else
   {
      QString symbolName = pItem->text();

      Service<DesktopServices> pDesktop;
      QImage symbolImage = pDesktop->getSymbolImage(symbolName.toStdString());

      mpSymbolPreview->setPixmap(QPixmap::fromImage(symbolImage));
   }
}

void GraphicSymbolWidget::notifySizeChange(int size)
{
   emit sizeChanged(static_cast<unsigned int>(size));
}
