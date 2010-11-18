/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include "LinkDatasetPage.h"
#include "DesktopServicesImp.h"
#include "Window.h"

#include <vector>
using namespace std;

LinkDatasetPage::LinkDatasetPage(QWidget* parent) :
   QWidget(parent)
{
   // Data set
   QLabel* pDatasetLabel = new QLabel("Available Data Sets:", this);
   mpDatasetCombo = new QComboBox(this);
   mpDatasetCombo->setEditable(false);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pDatasetLabel);
   pLayout->addWidget(mpDatasetCombo);
   pLayout->addStretch();

   // Initialization
   int initialIndex = 0;

   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   if (pDesktop != NULL)
   {
      // Populate the data set combo box
      vector<Window*> windows;
      pDesktop->getWindows(windows);
      for (unsigned int i = 0; i < windows.size(); i++)
      {
         Window* pWindow = windows[i];
         if (pWindow != NULL)
         {
            if (pWindow->isKindOf("SpatialDataWindow") == true)
            {
               string windowName = pWindow->getName();
               if (windowName.empty() == false)
               {
                  mpDatasetCombo->addItem(QString::fromStdString(windowName));
               }
            }
         }
      }

      // Get the initial combo box value from the active window
      string currentWindowName = "";
      pDesktop->getCurrentWorkspaceWindowName(currentWindowName);
      if (currentWindowName.empty() == false)
      {
         initialIndex = mpDatasetCombo->findText(QString::fromStdString(currentWindowName));
      }
   }

   mpDatasetCombo->setCurrentIndex(initialIndex);
}

LinkDatasetPage::~LinkDatasetPage()
{
}

QString LinkDatasetPage::getSelectedDatasetName() const
{
   QString strDataset;
   if (mpDatasetCombo != NULL)
   {
      strDataset = mpDatasetCombo->currentText();
   }

   return strDataset;
}
