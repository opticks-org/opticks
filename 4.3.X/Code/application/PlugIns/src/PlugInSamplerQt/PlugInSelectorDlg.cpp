/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "AppVerify.h"
#include "PlugInSelectorDlg.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

using namespace std;

PlugInSelectorDlg::PlugInSelectorDlg(QWidget* pParent, const vector<string>& plugins) : QDialog(pParent),
   mpCombo(NULL), mpRunAll(NULL)
{
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pPlugInsLabel = new QLabel("Available Plug-Ins: ", this);
   pLayout->addWidget(pPlugInsLabel, 0, 0);

   mpCombo = new QComboBox(this);
   pLayout->addWidget(mpCombo, 0, 1, 1, 2);
   pLayout->setColumnStretch(1, 10);

   for (vector<string>::const_iterator it = plugins.begin(); mpCombo != NULL && it != plugins.end(); ++it)
   {
      mpCombo->addItem(it->c_str());
   }
   mpCombo->setEditable(false);

   mpRunAll = new QCheckBox("Run All Tests", this);
   pLayout->addWidget(mpRunAll, 1, 1, 1, 2, Qt::AlignLeft);

   pLayout->setRowStretch(2, 10);

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 3, 0, 1, 3);

   QPushButton* pAccept = new QPushButton("OK", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pAccept);

   QPushButton* pReject = new QPushButton("Cancel", this);
   pRespLayout->addWidget(pReject);

   bool success = connect(mpRunAll, SIGNAL(toggled(bool)), this, SLOT(enableRunIndivTest(bool)));
   REQUIRE_DEBUG(success == true);

   success = connect(pAccept, SIGNAL(clicked()), this, SLOT(accept()));
   REQUIRE_DEBUG(success == true);

   success = connect(pReject, SIGNAL(clicked()), this, SLOT(reject()));
   REQUIRE_DEBUG(success == true);
}

void PlugInSelectorDlg::enableRunIndivTest(bool bRunAll)
{
   mpCombo->setEnabled(bRunAll == false);
}

bool PlugInSelectorDlg::runAllTests() const
{
   VERIFY(mpRunAll != NULL);
   return mpRunAll->isChecked();
}

vector<string> PlugInSelectorDlg::getSelectedPlugins() const
{
   vector<string> plugIns;
   VERIFYRV(mpCombo != NULL, plugIns);

   if (runAllTests() == true)
   {
      for (int i = 0; i < mpCombo->count(); ++i)
      {
         QString str = mpCombo->itemText(i);
         if (str.isEmpty() == false)
         {
            plugIns.push_back(str.toStdString());
         }
      }
   }
   else
   {
      if (mpCombo != NULL)
      {
         QString str = mpCombo->currentText();
         if (str.isEmpty() == false)
         {
            plugIns.push_back(str.toStdString());
         }
      }
   }
   return plugIns;
}
