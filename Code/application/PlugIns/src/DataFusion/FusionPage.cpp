/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "FusionPage.h"

#include "AppVerify.h"
#include "Slot.h"
#include "SpatialDataView.h"

FusionPage::FusionPage(QWidget* pParent) :
   QWidget(pParent),
   mpPrimaryView(NULL),
   mpSecondaryView(NULL)
{
}

FusionPage::~FusionPage()
{
}

void FusionPage::viewDeleted(Subject &subject, const std::string &signal, const boost::any &v)
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(&subject);
   if (pView != NULL)
   {
      if (pView == mpPrimaryView)
      {
         mpPrimaryView = NULL;
         emit modified();
      }
      else if (pView == mpSecondaryView)
      {
         mpSecondaryView = NULL;
         emit modified();
      }
   }
}

void FusionPage::setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary)
{
   if (mpPrimaryView != pPrimary || mpSecondaryView != pSecondary) // something changed
   {
      if (mpPrimaryView != NULL)
      {
         mpPrimaryView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionPage::viewDeleted));
      }
      if (mpSecondaryView != NULL)
      {
         mpSecondaryView->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionPage::viewDeleted));
      }
      if (mpPrimaryView != pPrimary)
      {
         mpPrimaryView = pPrimary;
         emit modified();
      }

      if (mpSecondaryView != pSecondary)
      {
         mpSecondaryView = pSecondary;
         emit modified();
      }

      if (mpPrimaryView != NULL)
      {
         mpPrimaryView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionPage::viewDeleted));
      }
      if (mpSecondaryView != NULL)
      {
         mpSecondaryView->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionPage::viewDeleted));
      }
   }
}
