/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "GcpGeoreference.h"
#include "GcpGui.h"
#include "GcpList.h"
#include "RasterElement.h"

using namespace std;

GcpGui::GcpGui(int maxOrder, const vector<string>& gcpLists, RasterElement* pRasterElement, QWidget* pParent) :
   QWidget(pParent),
   mMaxOrder(maxOrder),
   mpRasterElement(pRasterElement)
{
   // Gcp list combo
   QLabel* pGcpListLabel = new QLabel("GCP List:", this);
   mpGcpListCombo = new QComboBox(this);
   mpGcpListCombo->setEditable(false);

   // Polynomial order
   QLabel* pOrderLabel = new QLabel("Polynomial Order:", this);
   mpOrderLabel = new QLabel(this);
   mpOrderSpin = new QSpinBox(this);
   mpOrderSpin->setMinimum(1);
   mpOrderSpin->setMaximum(mMaxOrder);
   mpOrderSpin->setSingleStep(1);
   mpOrderSpin->setFixedWidth(50);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(10);
   pGrid->addWidget(pGcpListLabel, 0, 0);
   pGrid->addWidget(mpGcpListCombo, 0, 1, 1, 2);
   pGrid->addWidget(pOrderLabel, 1, 0);
   pGrid->addWidget(mpOrderSpin, 1, 1);
   pGrid->addWidget(mpOrderLabel, 2, 0, 1, 3);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(2, 10);

   // Initialization
   vector<string>::const_iterator iter;
   for (iter = gcpLists.begin(); iter != gcpLists.end(); ++iter)
   {
      string gcpList = *iter;
      if (gcpList.empty() == false)
      {
         mpGcpListCombo->addItem(QString::fromStdString(gcpList));
      }
   }

   if (gcpLists.empty() == false)
   {
      setCurrentGcpList(gcpLists.front().c_str());
   }

   // Connections
   connect(mpGcpListCombo, SIGNAL(activated(const QString&)), this, SLOT(setCurrentGcpList(const QString&)));
   connect(mpOrderSpin, SIGNAL(valueChanged(int)), this, SLOT(validateOrder(int)));
}

GcpGui::~GcpGui()
{
}

bool GcpGui::validateInput()
{
   return validateGcpList(mpGcpListCombo->currentText());
}

bool GcpGui::validateGcpList(const QString& strGcpList)
{
   bool bIsValid = false;
   if (strGcpList.isEmpty() == true)
   {
      return bIsValid;
   }

   string gcpListName = strGcpList.toStdString();
   GcpList* pGcpList = static_cast<GcpList*>(mpModel->getElement(gcpListName, "GcpList", mpRasterElement));

   int iMaxOrder = 0;
   if (pGcpList != NULL)
   {
      iMaxOrder = getMaxOrder(pGcpList->getSelectedPoints().size());
   }

   if (iMaxOrder <= 0)
   {
      bIsValid = false;
   }
   else
   {
      bIsValid = true;
   }

   bIsValid &= validateOrder(iMaxOrder);
   return bIsValid;
}

bool GcpGui::setCurrentGcpList(const QString& strGcpList)
{
   bool bIsValid = false;
   if (strGcpList.isEmpty() == true)
   {
      return bIsValid;
   }

   string gcpListName = strGcpList.toStdString();
   GcpList* pGcpList = static_cast<GcpList*>(mpModel->getElement(gcpListName, "GcpList", mpRasterElement));

   int iMaxOrder = 0;
   if (pGcpList != NULL)
   {
      iMaxOrder = getMaxOrder(pGcpList->getSelectedPoints().size());
   }

   if (iMaxOrder <= 0)
   {
      mpOrderSpin->setRange(0, 0);
      mpOrderSpin->setEnabled(false);
      mpOrderSpin->setValue(0);
      bIsValid = false;
   }
   else
   {
      mpOrderSpin->setRange(1, iMaxOrder);
      mpOrderSpin->setEnabled(true);
      mpOrderSpin->setValue(iMaxOrder);
      bIsValid = true;
   }

   bIsValid &= validateOrder(iMaxOrder);
   return bIsValid;
}

bool GcpGui::validateOrder(int iOrder)
{
   bool bIsValid = false;
   int iGcpsRequired = 3;
   if (iOrder > 0)
   {
      iGcpsRequired = COEFFS_FOR_ORDER(iOrder);
   }

   QString strRequired;
   QString strGcpList = mpGcpListCombo->currentText();

   string gcpListName = strGcpList.toStdString();

   GcpList* pGcpList = static_cast<GcpList*>(mpModel->getElement(gcpListName, "GcpList", mpRasterElement));
   if (pGcpList == NULL)
   {
      strRequired.sprintf("%d GCPs required, invalid GCP List", iGcpsRequired);
      bIsValid = false;
   }
   else
   {
      int iGcps = static_cast<int>(pGcpList->getSelectedPoints().size());
      strRequired.sprintf("%d GCPs required, %d present", iGcpsRequired, iGcps);
      if (iGcps < iGcpsRequired)
      {
         bIsValid = false;
      }
      else
      {
         bIsValid = true;
      }
   }

   mpOrderLabel->setText(strRequired);
   return bIsValid;
}

unsigned short GcpGui::getOrder() const
{
   return mpOrderSpin->value();
}

string GcpGui::getGcpListName() const
{
   string listName = "";

   QString strListName = mpGcpListCombo->currentText();
   if (strListName.isEmpty() == false)
   {
      listName = strListName.toStdString();
   }

   return listName;
}

int GcpGui::getMaxOrder(int numGcps)
{
   // numBasisValues = (order+1)*(order+2)/2, solve for order
   // add fudge factor to prevent x.9999999 being truncated to x
   int maxOrder = static_cast<int>((-3.0 + sqrt(9.0 + 8.0 * (numGcps - 1))) / 2.0 + 0.00000001);
   if (maxOrder < 0)
   {
      maxOrder = 0;
   }

   if (maxOrder > mMaxOrder)
   {
      maxOrder = mMaxOrder;
   }

   return maxOrder;
}
