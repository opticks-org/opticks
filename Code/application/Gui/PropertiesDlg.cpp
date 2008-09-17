/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>

#include "AppVerify.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Properties.h"
#include "PropertiesDlg.h"

#include <string>
using namespace std;

PropertiesDlg::PropertiesDlg(SessionItem* pSessionItem, const vector<string>& displayedPages, QWidget* pParent) :
   QDialog(pParent),
   mpSessionItem(pSessionItem),
   mpButtonBox(NULL)
{
   // Properties widget
   QStackedWidget* pStack = new QStackedWidget(this);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   mpButtonBox = new QDialogButtonBox(this);
   mpButtonBox->setOrientation(Qt::Horizontal);
   mpButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pStack, 10);
   pLayout->addWidget(pLine);
   pLayout->addWidget(mpButtonBox);

   // Initialization
   setWindowTitle("Properties");
   setModal(true);

   if (mpSessionItem != NULL)
   {
      // Update the caption to include the item name
      string itemName = mpSessionItem->getDisplayName();
      if (itemName.empty() == true)
      {
         itemName = mpSessionItem->getName();
      }

      if (itemName.empty() == false)
      {
         setWindowTitle("Properties: " + QString::fromStdString(itemName));
      }

      // Add the properties widgets
      if (displayedPages.empty() == false)
      {
         // Use a tab widget if there is more than one properties page
         QTabWidget* pTabWidget = NULL;

         vector<string>::size_type numWidgets = displayedPages.size();
         if (numWidgets > 1)
         {
            pTabWidget = new QTabWidget(this);
            pTabWidget->setTabPosition(QTabWidget::North);
         }

         for (vector<string>::size_type i = 0; i < numWidgets; ++i)
         {
            string plugInName = displayedPages[i];
            if (plugInName.empty() == false)
            {
               PlugInResource propertiesPlugIn(plugInName);

               Properties* pProperties = dynamic_cast<Properties*>(propertiesPlugIn.get());
               if (pProperties != NULL)
               {
                  QWidget* pWidget = pProperties->getWidget();
                  if (pWidget != NULL)
                  {
                     if (pProperties->initialize(mpSessionItem) == true)
                     {
                        mPlugIns.push_back(pProperties);
                        propertiesPlugIn.release();

                        if (pTabWidget != NULL)
                        {
                           const string& pageName = pProperties->getPropertiesName();
                           if (pageName.empty() == false)
                           {
                              QLayout* pLayout = pWidget->layout();
                              if (pLayout != NULL)
                              {
                                 pLayout->setMargin(10);
                              }
                              else
                              {
                                 QWidget* pMarginWidget = new QWidget(NULL);
                                 pWidget->setParent(pMarginWidget);

                                 QVBoxLayout* pMarginLayout = new QVBoxLayout(pMarginWidget);
                                 pMarginLayout->setMargin(10);
                                 pMarginLayout->setSpacing(0);
                                 pMarginLayout->addWidget(pWidget, 10);
                                 pWidget = pMarginWidget;
                              }

                              pTabWidget->addTab(pWidget, QString::fromStdString(pageName));
                           }

                           string errorMessage = string("The ") + plugInName + string(" plug-in does not have "
                              "a properties name.  Its widget will not be added to the properties dialog.");
                           VERIFYNR_MSG(pageName.empty() == false, errorMessage.c_str());
                        }
                        else
                        {
                           pStack->addWidget(pWidget);
                        }
                     }
                  }
               }
            }
         }

         if ((pTabWidget != NULL) && (pTabWidget->count() > 0))
         {
            pStack->addWidget(pTabWidget);
         }
      }
   }

   if (pStack->count() == 0)
   {
      QLabel* pNoPropertiesLabel = new QLabel("No properties are available", this);
      pNoPropertiesLabel->setAlignment(Qt::AlignCenter);
      pNoPropertiesLabel->setMinimumSize(250, 100);

      pStack->addWidget(pNoPropertiesLabel);
   }

   // Connections
   connect(mpButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(processButtonClick(QAbstractButton*)));
}

PropertiesDlg::~PropertiesDlg()
{
   // NOTE: The QWidget* returned by Properties::getWidget() is not being re-parented
   // because this causes a crash if the implementor of the Properties interface does
   // not destroy the widget that they created in their destructor

   Service<PlugInManagerServices> pManager;
   for (vector<Properties*>::iterator iter = mPlugIns.begin(); iter != mPlugIns.end(); ++iter)
   {
      pManager->destroyPlugIn(dynamic_cast<PlugIn*>(*iter));
   }
}

bool PropertiesDlg::applyChanges()
{
   if (mPlugIns.empty() == true)
   {
      return false;
   }

   bool bSuccess = true;
   for (vector<Properties*>::iterator iter = mPlugIns.begin(); bSuccess && (iter != mPlugIns.end()); ++iter)
   {
      Properties* pProperties = *iter;
      if (pProperties != NULL)
      {
         bSuccess = pProperties->applyChanges();
      }
      else
      {
         bSuccess = false;
      }
   }

   return bSuccess;
}

void PropertiesDlg::processButtonClick(QAbstractButton* pButton)
{
   if (pButton == NULL)
   {
      return;
   }

   QDialogButtonBox::StandardButton button = mpButtonBox->standardButton(pButton);
   if (button == QDialogButtonBox::Ok)
   {
      if (applyChanges() == true)
      {
         accept();
      }
   }
   else if (button == QDialogButtonBox::Apply)
   {
      applyChanges();
   }
   else if (button == QDialogButtonBox::Cancel)
   {
      reject();
   }
}
