/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QSplitter>
#include <QtGui/QTreeWidget>

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "DataFusionTools.h"
#include "DockWindow.h"
#include "DataFusionDlg.h"
#include "DatasetPage.h"
#include "Executable.h"
#include "FusionAlgorithmInputsPage.h"
#include "FusionLayersSelectPage.h"
#include "GcpList.h"
#include "GcpLayer.h"
#include "GeoAlgorithms.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MouseMode.h"
#include "ObjectResource.h"
#include "PlugIn.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Poly2D.h"
#include "Polywarp.h"
#include "ProgressTracker.h"
#include "RasterLayer.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "switchOnEncoding.h"
#include "TiePointList.h"
#include "TiePointPage.h"
#include "TypesFile.h"
#include "Undo.h"
#include "UtilityServices.h"
#include "WorkspaceWindow.h"

#include <list>
#include <string>
using namespace std;

const string DataFusionDlg::PRIMARY_CHIP_WIDGET_NAME = "primary";
const string DataFusionDlg::SECONDARY_CHIP_WIDGET_NAME = "secondary";
const string DataFusionDlg::ANNOTATION_LAYER_NAME = "Fused Layers";

DataFusionDlg::DataFusionDlg(PlugIn* pPlugIn, ProgressTracker &progressTracker, QWidget* pParent) :
   QDialog(pParent),
   mpDescriptionLabel(NULL),
   mpFlicker(NULL),
   mpSbs(NULL),
   mpPlugIn(pPlugIn),
   mProgressTracker(progressTracker)
{
   setWindowTitle("Quick-Look Data Fusion");
   setModal(false);

   // Description label
   mpDescriptionLabel = new QLabel(this);

   // Horizontal line
   QFrame* pDescriptionLine = new QFrame(this);
   pDescriptionLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Page widget stack
   mpStack = new QStackedWidget(this);
   mpDatasetPage = new DatasetPage(mpStack);
   mpTiePointPage = new TiePointPage(mpStack);
   mpLayersPage = new FusionLayersSelectPage(mpStack);
   mpInputsPage = new FusionAlgorithmInputsPage(mpStack);

   // Horizontal line
   QFrame* pButtonLine = new QFrame(this);
   pButtonLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   mpBackButton = new QPushButton("< &Back", this);
   mpNextButton = new QPushButton("&Next >", this);
   mpFinishButton = new QPushButton("&Finish", this);

   QPushButton* pCancelButton = new QPushButton("&Cancel", this);
   pCancelButton->setDefault(true);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(mpBackButton);
   pButtonLayout->addWidget(mpNextButton);
   pButtonLayout->addWidget(mpFinishButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpDescriptionLabel);
   pLayout->addWidget(pDescriptionLine);
   pLayout->addWidget(mpStack, 10);
   pLayout->addWidget(pButtonLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   mModified.insert(mpDatasetPage, true);
   mModified.insert(mpTiePointPage, true);
   mModified.insert(mpLayersPage, true);
   mModified.insert(mpInputsPage, true);

   mpStack->addWidget(mpDatasetPage);
   mpStack->addWidget(mpTiePointPage);
   mpStack->addWidget(mpLayersPage);
   mpStack->addWidget(mpInputsPage);
   showPage(mpDatasetPage);
   mpFinishButton->hide();

   // required so the wizard shows up the correct size
   resize(minimumSizeHint());

   // connections
   VERIFYNR(connect(mpDatasetPage, SIGNAL(modified()), this, SLOT(updateModifiedFlags())));
   VERIFYNR(connect(mpTiePointPage, SIGNAL(modified()), this, SLOT(updateModifiedFlags())));
   VERIFYNR(connect(mpLayersPage, SIGNAL(modified()), this, SLOT(updateModifiedFlags())));
   VERIFYNR(connect(mpInputsPage, SIGNAL(modified()), this, SLOT(updateModifiedFlags())));
   VERIFYNR(connect(mpInputsPage, SIGNAL(executeAlgorithm()), this, SLOT(fuse())));
   VERIFYNR(connect(mpBackButton, SIGNAL(clicked()), this, SLOT(back())));
   VERIFYNR(connect(mpNextButton, SIGNAL(clicked()), this, SLOT(next())));
   VERIFYNR(connect(mpFinishButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));
}

DataFusionDlg::~DataFusionDlg()
{
   if (mpFlicker != NULL)
   {
      mpFlicker->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataFusionDlg::windowDeleted));
   }

   if (mpSbs != NULL)
   {
      mpSbs->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataFusionDlg::windowDeleted));
      delete mpSbs->getWidget();
   }
}

RasterElement* DataFusionDlg::getPrimaryChip() const
{
   RasterElement* pRaster = NULL;
   if (mpFlicker != NULL)
   {
      const SpatialDataView* pView = mpFlicker->getSpatialDataView();
      if (pView != NULL)
      {
         const LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            pRaster = pLayerList->getPrimaryRasterElement();
         }
      }
   }
   // SBS view is up and the RasterElement wasn't retrieved, so attempt to get it from the SBS view
   if (mpSbs != NULL && pRaster == NULL)
   {
      QWidget* pWidget = mpSbs->getWidget();
      if (pWidget != NULL && pWidget->inherits("QSplitter"))
      {
         QSplitter* pSplitter = static_cast<QSplitter*>(pWidget);
         SpatialDataView* pPrimaryView =
            dynamic_cast<SpatialDataView*>(pSplitter->widget(0));
         if (pPrimaryView != NULL)
         {
            const LayerList* pLayerList = pPrimaryView->getLayerList();
            if (pLayerList != NULL)
            {
               pRaster = pLayerList->getPrimaryRasterElement();
            }
         }
      }
   }
   return pRaster;
}

RasterElement* DataFusionDlg::getSecondaryChip() const
{
   RasterElement* pRaster = NULL;
   if (mpFlicker != NULL)
   {
      const SpatialDataView* pView = mpFlicker->getSpatialDataView();
      if (pView != NULL)
      {
         Layer* pLayer = pView->getTopMostLayer(RASTER);
         if (pLayer != NULL)
         {
            pRaster = dynamic_cast<RasterElement*>(pLayer->getDataElement());
         }
      }
   }
   // SBS view is up and the RasterElement wasn't retrieved, so attempt to get it from the SBS view
   if (mpSbs != NULL && pRaster == NULL)
   {
      QWidget* pWidget = mpSbs->getWidget();
      if (pWidget != NULL && pWidget->inherits("QSplitter"))
      {
         QSplitter* pSplitter = static_cast<QSplitter*>(pWidget);
         SpatialDataView* pSecondaryView =
            dynamic_cast<SpatialDataView*>(pSplitter->widget(1));
         if (pSecondaryView != NULL)
         {
            const LayerList* pLayerList = pSecondaryView->getLayerList();
            if (pLayerList != NULL)
            {
               pRaster = pLayerList->getPrimaryRasterElement();
            }
         }
      }
   }
   return pRaster;
}

void DataFusionDlg::windowDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   SpatialDataWindow* pSdw = dynamic_cast<SpatialDataWindow*>(&subject);
   WorkspaceWindow* pWW = dynamic_cast<WorkspaceWindow*>(&subject);
   if (pSdw != NULL)
   {
      if (pSdw == mpFlicker)
      {
         mpFlicker = NULL;
      }
   }
   else if (pWW != NULL)
   {
      if (pWW == mpSbs)
      {
         mpSbs = NULL;
      }
   }
}

void DataFusionDlg::showPage(QWidget* pPage)
{
   FusionPage* pFusionPage = dynamic_cast<FusionPage*>(pPage);
   VERIFYNRV(pFusionPage != NULL);

   // pPage is the page about to be shown. Prepare it by setting views as needed.
   SpatialDataView* pPrimaryView = mpDatasetPage->getPrimaryView();
   SpatialDataView* pSecondaryView = mpDatasetPage->getSecondaryView();

   if (pFusionPage == mpDatasetPage)
   {
      mpDescriptionLabel->setText("Select Primary and Secondary Images");
   }
   else if (pFusionPage == mpTiePointPage)
   {
      mpDescriptionLabel->setText("Select Tie Points");
   }
   else if (pFusionPage == mpLayersPage)
   {
      string descText = "Select Annotation Layers & AOI Layers (Flicker View Only)";

      if (NN(mpDatasetPage))
      {
         const SpatialDataView* pPrimaryView = mpDatasetPage->getPrimaryView();
         if (NN(pPrimaryView))
         {
            string viewName = pPrimaryView->getName();
            descText = descText + ":\n" + viewName;
         }
      }
      mpDescriptionLabel->setText(descText.c_str());
   }
   else if (pFusionPage == mpInputsPage)
   {
      mpDescriptionLabel->setText("Select Region of Interest (ROI) and Output Views");
   }

   // page about to be shown was modified
   if (mModified[pFusionPage] == true)
   {
      pFusionPage->setViews(pPrimaryView, pSecondaryView);
   }

   if (pPrimaryView != NULL)
   {
      pPrimaryView->setMouseMode(pFusionPage->getPreferredPrimaryMouseMode());
      pPrimaryView->setActiveLayer(pFusionPage->getPreferredPrimaryActiveLayer());
   }

   if (pSecondaryView != NULL)
   {
      pSecondaryView->setMouseMode(pFusionPage->getPreferredSecondaryMouseMode());
      pSecondaryView->setActiveLayer(pFusionPage->getPreferredSecondaryActiveLayer());
   }

   mpBackButton->setEnabled(pPage != mpDatasetPage);
   mpNextButton->setVisible(pPage != mpInputsPage);
   mpFinishButton->setVisible(pPage == mpInputsPage);
   mpStack->setCurrentWidget(pPage);
   enableWizardButtons();
}

void DataFusionDlg::back()
{
   QWidget* pPageToShow = NULL;

   QWidget* pPage = mpStack->currentWidget();
   if (pPage == mpTiePointPage)
   {
      pPageToShow = mpDatasetPage;
   }
   else if (pPage == mpLayersPage)
   {
      pPageToShow = mpTiePointPage;
   }
   else if (pPage == mpInputsPage)
   {
      pPageToShow = mpLayersPage;
   }

   // if currently on page 3 (Layer Page) and the bypass state is set, go back to page 1, not page 2
   if (pPage == mpLayersPage && mpDatasetPage->canBypassTiePointStep())
   {
      pPageToShow = mpDatasetPage;
   }

   showPage(pPageToShow);
}

void DataFusionDlg::next()
{
   FusionPage* pFusionPage = dynamic_cast<FusionPage*>(mpStack->currentWidget());
   VERIFYNRV(pFusionPage != NULL);

   // Show the next page
   QWidget* pPageToShow = NULL;
   if (pFusionPage == mpDatasetPage)
   {
      pPageToShow = mpTiePointPage;

      SpatialDataView* pPrimaryView = mpDatasetPage->getPrimaryView();
      SpatialDataView* pSecondaryView = mpDatasetPage->getSecondaryView();

      // if on the DatasetPage and the user chose to bypass the TiePoint Step, skip that step
      if (mpDatasetPage->canBypassTiePointStep() == true)
      {
         pPageToShow = mpLayersPage;
         // skipping the tie point page, so set its modified flag to FALSE
         mModified[mpTiePointPage] = false;
      }

      // if the DatasetPage was changed (ie. different views), save their mouse modes
      if (mModified[mpDatasetPage] == true)
      {
         mPrimaryMouseMode = "";
         if (pPrimaryView != NULL)
         {
            MouseMode* pMode = pPrimaryView->getCurrentMouseMode();
            if (pMode != NULL)
            {
               pMode->getName(mPrimaryMouseMode);
            }
         }
         mSecondaryMouseMode = "";
         if (pSecondaryView != NULL)
         {
            MouseMode* pMode = pSecondaryView->getCurrentMouseMode();
            if (pMode != NULL)
            {
               pMode->getName(mSecondaryMouseMode);
            }
         }
      }
   }
   else if (pFusionPage == mpTiePointPage)
   {
      pPageToShow = mpLayersPage;
   }
   else if (pFusionPage == mpLayersPage)
   {
      pPageToShow = mpInputsPage;
   }

   // prior to showing the next page, mark the current page's modified flag to false
   if (mModified[pFusionPage] == true)
   {
      mModified[pFusionPage] = false;
   }

   // note that if pFusionPage was modified, all pages after it are also considered modified

   showPage(pPageToShow);
}

void DataFusionDlg::closeEvent(QCloseEvent* pEvent)
{
   if (mpSbs != NULL)
   {
      int iResult = QMessageBox::warning(this, windowTitle(), "If you close the wizard, "
         "the Side-By-Side view will close.  Close anyway?", QMessageBox::Yes, QMessageBox::No);
      if (iResult == QMessageBox::No || mpDesktop->deleteWindow(mpSbs) == false)
      {
         pEvent->ignore();
         return;
      }
   }

   if (mpPlugIn != NULL)
   {
      Executable* pExecutable = dynamic_cast<Executable*>(mpPlugIn);
      if (pExecutable != NULL)
      {
         if (pExecutable->abort() == false)
         {
            pEvent->ignore();
            return;
         }
      }
   }

   if (mpDatasetPage != NULL)
   {
      SpatialDataView* pView = mpDatasetPage->getPrimaryView();
      if (pView != NULL)
      {
         pView->setMouseMode(mPrimaryMouseMode);
      }
      pView = mpDatasetPage->getSecondaryView();
      if (pView != NULL)
      {
         pView->setMouseMode(mSecondaryMouseMode);
      }
   }

   QDialog::closeEvent(pEvent);
}

void DataFusionDlg::accept()
{
   if (mModified[mpInputsPage] == true) // the algorithm hasn't been run yet, so run it
   {
      fuse();
   }

   setResult(QDialog::Accepted);

   bool bSuccess = close();
   if (bSuccess == false)
   {
      setResult(-1);
   }
}

void DataFusionDlg::reject()
{
   setResult(QDialog::Rejected);

   bool bSuccess = close();
   if (bSuccess == false)
   {
      setResult(-1);
   }
}

void DataFusionDlg::updateModifiedFlags()
{
   const FusionPage* pPage = static_cast<const FusionPage*>(sender());
   VERIFYNRV(pPage != NULL);
   if (pPage == static_cast<FusionPage*>(mpDatasetPage))
   {
      // set the modified flags
      mModified[mpDatasetPage] = true;
      mModified[mpTiePointPage] = true;
      mModified[mpLayersPage] = true;
      mModified[mpInputsPage] = true;
   }
   if (pPage == static_cast<FusionPage*>(mpTiePointPage))
   {
      // set the modified flags
      mModified[mpTiePointPage] = true;
      mModified[mpLayersPage] = true;
      mModified[mpInputsPage] = true;
   }
   if (pPage == static_cast<FusionPage*>(mpLayersPage))
   {
      // set the modified flags
      mModified[mpLayersPage] = true;
      mModified[mpInputsPage] = true;
   }
   if (pPage == static_cast<FusionPage*>(mpInputsPage))
   {
      // set the modified flags
      mModified[mpInputsPage] = true;
   }

   enableWizardButtons();
}

void DataFusionDlg::enableWizardButtons()
{
   FusionPage* pCurrentPage = dynamic_cast<FusionPage*>(mpStack->currentWidget());
   VERIFYNRV(pCurrentPage != NULL);

   bool bEnable = true;
   if (mModified[pCurrentPage] == true)
   {
      bEnable = pCurrentPage->isValid();
   }

   if (pCurrentPage == mpInputsPage)
   {
      mpFinishButton->setEnabled(bEnable);
   }
   else
   {
      mpNextButton->setEnabled(bEnable);
   }
}

void DataFusionDlg::fuse()
{
   if (mModified[mpInputsPage] == true)
   {
      Service<ModelServices> pModel;
      VERIFYNRV(pModel.get() != NULL);
      Service<DesktopServices> pDesktop;
      VERIFYNRV(pDesktop.get() != NULL);
      Service<UtilityServices> pUtils;
      VERIFYNRV(pUtils.get() != NULL);

      // set up the ProgressTracker - initialize each time Fuse is called since the algorithm runs multiple times
      DataFusionTools::setAbortFlag(false);

      mProgressTracker.initialize("Executing DataFusion Algorithm", "app",
                                  "7098CB37-A6AC-4c8d-9AC9-E49E4D7841AC");

      vector<ProgressTracker::Stage> vStages;
      vStages.push_back(ProgressTracker::Stage("Creating warp matrices...", "app",
                                               "5D631403-8694-4a59-B4CE-756DE4350699", 5));
      if (mpInputsPage->flicker() || mpInputsPage->sbs())
      {
         vStages.push_back(ProgressTracker::Stage("Creating warped images...", "app",
                                                  "9997EE33-A219-40bf-B0F3-759F7FDE7E7A", 150));
      }
      mProgressTracker.subdivideCurrentStage(vStages);

      // Georeference the scenes
      SpatialDataView* pPrimaryView = mpDatasetPage->getPrimaryView();
      if (pPrimaryView == NULL)
      {
         string msg = "Error DataFusionDlg: Primary data set does not exist. "
            "Go back to the Dataset Select Page and select a new Primary Dataset.";
         mProgressTracker.report(msg, 0, ERRORS);
         return;
      }

      LayerList* pLayerList = pPrimaryView->getLayerList();
      VERIFYNRV(pLayerList != NULL);
      RasterElement* pPrimaryRaster = dynamic_cast<RasterElement*>(pLayerList->getPrimaryRasterElement());
      VERIFYNRV(pPrimaryRaster != NULL);
      SpatialDataWindow *pPrimaryWindow = 
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pPrimaryRaster->getName(), SPATIAL_DATA_WINDOW));
      VERIFYNRV(pPrimaryWindow != NULL);
      VERIFYNRV(pDesktop->setCurrentWorkspaceWindow(pPrimaryWindow));
      if (!pPrimaryRaster->isGeoreferenced())
      {
         ExecutableResource georef("Georeference", "", NULL, false);
         georef->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pPrimaryRaster);
         bool bSuccess = georef->execute();
         if (!bSuccess || !pPrimaryRaster->isGeoreferenced())
         {
            string msg = "ERROR DataFusionDlg: Georeference of primary scene failed!";
            mProgressTracker.report(msg, 0, ERRORS, true);
            return;
         }
      }
      SpatialDataView *pSecondaryView = mpDatasetPage->getSecondaryView();
      if (pSecondaryView == NULL)
      {
         string msg = "Error DataFusionDlg: Secondary data set does not exist. "
            "Go back to the Dataset Select Page and select a new Secondary Dataset.";
         mProgressTracker.report(msg, 0, ERRORS);
         return;
      }
      pLayerList = pSecondaryView->getLayerList();
      VERIFYNRV(pLayerList != NULL);
      RasterElement* pSecondaryRaster = pLayerList->getPrimaryRasterElement();
      VERIFYNRV(pSecondaryRaster != NULL);
      SpatialDataWindow *pSecondaryWindow = 
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pSecondaryRaster->getName(), SPATIAL_DATA_WINDOW));
      VERIFYNRV(pSecondaryWindow != NULL);
      VERIFYNRV(pDesktop->setCurrentWorkspaceWindow(pSecondaryWindow));
      if (!pSecondaryRaster->isGeoreferenced())
      {
         ExecutableResource georef("Georeference", "", NULL, false);
         georef->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pSecondaryRaster);
         bool bSuccess = georef->execute();
         if (!bSuccess || !pSecondaryRaster->isGeoreferenced())
         {
            string msg = "ERROR DataFusionDlg: Georeference of secondary scene failed!";
            mProgressTracker.report(msg, 0, ERRORS, true);
            return;
         }
      }

      LocationType gsdPrimaryPoint(GeoAlgorithms::getXaxisGSD(pPrimaryRaster),
                                   GeoAlgorithms::getYaxisGSD(pPrimaryRaster));
      double gsdPrimary = gsdPrimaryPoint.length();


      LocationType gsdSecondaryPoint(GeoAlgorithms::getXaxisGSD(pSecondaryRaster),
                                     GeoAlgorithms::getYaxisGSD(pSecondaryRaster));
      double gsdSecondary = gsdSecondaryPoint.length();

      // zoomFactor is a number representing how many secondary image pixels in each direction fit in each primary pixel
      int zoomFactor = 1;
      
      if (gsdSecondary != 0 && gsdPrimary > gsdSecondary)
      {
         zoomFactor = static_cast<int>(gsdPrimary/gsdSecondary);
      }

      // Normalize the zoom factor
      if (zoomFactor > 10)
      {
         zoomFactor = 10;
      }

      int x1, y1, x2, y2;
      if (mpInputsPage->getRoiBoundingBox(x1, y1, x2, y2) == false)
      {
         string msg = "ERROR DataFusionDlg: No AOI Layer or AOI was found for the primary image."
            " Perhaps you deleted or hid the layer?";
         mProgressTracker.report(msg, 0, ERRORS, true);
         return;
      }

      // set the vectors with the coordinates
      // corner point coefficients from Primary Image
      Vector<double> xP(4);
      xP[0] = x1; 
      xP[1] = x1;
      xP[2] = x2;
      xP[3] = x2;
      Vector<double> yP(4);
      yP[0] = y1;
      yP[1] = y2;
      yP[2] = y2;
      yP[3] = y1;

      LocationType primaryLlc = pPrimaryRaster->convertPixelToGeocoord(LocationType(x1, y1));
      LocationType primaryUlc = pPrimaryRaster->convertPixelToGeocoord(LocationType(x1, y2));
      LocationType primaryUrc = pPrimaryRaster->convertPixelToGeocoord(LocationType(x2, y2));
      LocationType primaryLrc = pPrimaryRaster->convertPixelToGeocoord(LocationType(x2, y1));

      double latOffset = 0, lonOffset = 0;

      // if the tie points are bypassed, we can use the default offset of 0,0 to avoid many changes
      if (mpDatasetPage->canBypassTiePointStep() == false)
      {
         const TiePointList* pTiePoints = mpTiePointPage->getTiePoints();
         if (pTiePoints == NULL)
         {
            string msg = "Error DataFusionDlg: No Tie Points are available!";
            mProgressTracker.report(msg.c_str(), 0, ERRORS, true);
            return;
         }

         const vector<TiePoint>& tiePoints = pTiePoints->getTiePoints();
         // if there aren't any tie points and the user did not bypass the page, that's a bug
         VERIFYNRV(tiePoints.size() > 0);

         for (vector<TiePoint>::const_iterator it = tiePoints.begin(); it != tiePoints.end(); ++it)
         {
            const TiePoint& pt = *it;

            LocationType primary(pt.mReferencePoint.mX, pt.mReferencePoint.mY);
            LocationType secondary(pt.mMissionOffset.mX, pt.mMissionOffset.mY);

            LocationType primaryGeo = pPrimaryRaster->convertPixelToGeocoord(primary);
            LocationType secondaryGeo = pSecondaryRaster->convertPixelToGeocoord(secondary);

            lonOffset += secondaryGeo.mY - primaryGeo.mY;
            latOffset += secondaryGeo.mX - primaryGeo.mX;
         }
         latOffset /= tiePoints.size();
         lonOffset /= tiePoints.size();
      }

      LocationType offset(lonOffset, latOffset);

      // use secondary RasterElement's georeferencing to get pixel values for sX and sY
      // corner point coefficients from Secondary Image
      // and also 'snap to' the tie point(s) by using the offset
      Vector<double> xS(4);
      xS[0] = pSecondaryRaster->convertGeocoordToPixel(primaryLlc - offset).mX;
      xS[1] = pSecondaryRaster->convertGeocoordToPixel(primaryUlc - offset).mX;
      xS[2] = pSecondaryRaster->convertGeocoordToPixel(primaryUrc - offset).mX;
      xS[3] = pSecondaryRaster->convertGeocoordToPixel(primaryLrc - offset).mX;
      Vector<double> yS(4);
      yS[0] = pSecondaryRaster->convertGeocoordToPixel(primaryLlc - offset).mY;
      yS[1] = pSecondaryRaster->convertGeocoordToPixel(primaryUlc - offset).mY;
      yS[2] = pSecondaryRaster->convertGeocoordToPixel(primaryUrc - offset).mY;
      yS[3] = pSecondaryRaster->convertGeocoordToPixel(primaryLrc - offset).mY;

      Vector<double> P, Q; // warp matrices

      const DataDescriptor *pDescriptor = pPrimaryRaster->getDataDescriptor();
      try
      {
         polywarp(xS, yS, xP, yP, P, Q, zoomFactor, mProgressTracker);
         mProgressTracker.nextStage();

         if (pDescriptor == NULL)
         {
            throw FusionException("ERROR DataFusionDlg: Unable to get Data descriptor! Bailing out!",
                                  __LINE__, __FILE__);
         }

         if (mpFlicker != NULL) // blast the old one
         {
            if (pDesktop->deleteWindow(mpFlicker) == false)
            {
               throw FusionException("ERROR DataFusionDlg: Unable to close Flicker window.",
                                     __LINE__, __FILE__);
            }
         }
         if (mpSbs != NULL) // blast the old one
         {
            if (pDesktop->deleteWindow(mpSbs) == false)
            {
               throw FusionException("ERROR DataFusionDlg: Unable to close Side-By-Side window.",
                                     __LINE__, __FILE__);
            }
         }

         RasterElement* pChip = createChip(pPrimaryRaster, static_cast<unsigned int>(x1), static_cast<unsigned int>(x2),
                                        static_cast<unsigned int>(y1), static_cast<unsigned int>(y2),
                                        "_chip", 255, zoomFactor);
         if (pChip == NULL)
         {
            throw FusionException("ERROR DataFusionDlg: Unable to create the primary image chip!", __LINE__, __FILE__);
         }

         RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pChip->getDataDescriptor());

         // Set the number of columns and rows in the secondary image.
         // This should be the actual number of columns and rows in the primary image multiplied by zoomFactor.
         unsigned int newx = zoomFactor * pDescriptor->getColumnCount();
         unsigned int newy = zoomFactor * pDescriptor->getRowCount();
         RasterElement* pSPrime = createSecondaryImageChip(pSecondaryRaster, P, Q, newx, newy, x1, y1, zoomFactor,
                                                         mpInputsPage->inMemory());
         bool bAborted = DataFusionTools::getAbortFlag();
         if (pSPrime == NULL || bAborted)
         {
            pModel->destroyElement(pChip); // destroy primary chip
            if (pSPrime == NULL)
            {
               throw FusionException("Unable to create the secondary image chip!", __LINE__, __FILE__);
            }
            else if (bAborted)
            {
               // don't throw an exception since an error did not occur. Throwing is only for unrecoverable errors.
               mProgressTracker.report("User aborted algorithm!", 0, ABORT, true);
               return;
            }
         }

         DataDescriptor* pChipDesc = const_cast<DataDescriptor*>(pChip->getDataDescriptor());
         REQUIRE(pChipDesc != NULL);

         if (mpInputsPage->flicker()) // creating a flicker view
         {
            string windowName = getNewFilename(pPrimaryRaster->getName(), "_flicker");
            mpFlicker = static_cast<SpatialDataWindow*>(pDesktop->createWindow(windowName, SPATIAL_DATA_WINDOW));
            if (mpFlicker != NULL)
            {
               mpFlicker->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataFusionDlg::windowDeleted));

               SpatialDataView* pFlickerView = mpFlicker->getSpatialDataView();
               REQUIRE(pFlickerView != NULL);

               UndoLock lock(pFlickerView);

               pFlickerView->setPrimaryRasterElement(pChip);
               RasterLayer* pPrimaryChipLayer = static_cast<RasterLayer*>(
                  pFlickerView->createLayer(RASTER, pChip));
               RasterLayer* pSecondaryChipLayer =
                  static_cast<RasterLayer*>(pFlickerView->createLayer(RASTER, pSPrime));

               // ensure copying by lat-long works properly
               ExecutableResource georefRes("GCP Georeference", "", NULL, false);
               georefRes->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pChip);
               int order = 1;
               georefRes->getInArgList().setPlugInArgValue<int>("Order", &order);
               string gcpList = "Corner Coordinates";
               georefRes->getInArgList().setPlugInArgValue<string>("GCP List", &gcpList);

               if (georefRes->execute() == true)
               {
                  LatLonLayer* pLayer =
                     static_cast<LatLonLayer*>(pFlickerView->createLayer(LAT_LONG, pChip));
                  pLayer->setStyle(LATLONSTYLE_CROSS);
               }
               georefRes.release(); // the RasterElement manages the PlugIn now.

               QApplication::sendPostedEvents();
               pFlickerView->zoomExtents();

               // Layers failed to copy, user said that not copying layers was wrong
               if (copyLayersToView(*pFlickerView, zoomFactor) == false)
               {
                  if (mpFlicker != NULL)
                  {
                     if (pDesktop->deleteWindow(mpFlicker) == false)
                     {
                        return;
                     }
                  }
                  if (mpSbs != NULL)
                  {
                     if (pDesktop->deleteWindow(mpSbs) == false)
                     {
                        return;
                     }
                  }
                  return;
               }
               // bring the secondary image chip to the very front so copied layers do not obstruct it
               pFlickerView->setFrontLayer(pSecondaryChipLayer);

               if (mpInputsPage->copyColormap(*pPrimaryView) == true)
               {
                  // copy any color maps from the primary image
                  RasterLayer* pSourceLayer = static_cast<RasterLayer*>(pPrimaryView->getTopMostLayer(RASTER));
                  REQUIRE(pSourceLayer != NULL); // at the moment we only allow fusing data cubes
                  copyColorMap(*pPrimaryChipLayer, *pSourceLayer);
               }

               if (mpInputsPage->copyColormap(*pSecondaryView) == true)
               {
                  RasterLayer* pSourceLayer = static_cast<RasterLayer*>(pSecondaryView->getTopMostLayer(RASTER));
                  REQUIRE(pSourceLayer != NULL); // at the moment we only allow fusing data cubes
                  copyColorMap(*pSecondaryChipLayer, *pSourceLayer);
               }
            }
            else
            {
               throw FusionException("ERROR DataFusionDlg: Unable to create Flicker window!", __LINE__, __FILE__);
            }
         }
         if (mpInputsPage->sbs())
         {
            string windowName = getNewFilename(pPrimaryRaster->getName(), "_sbs");
            mpSbs = static_cast<WorkspaceWindow*>(pDesktop->createWindow(windowName, WORKSPACE_WINDOW));
            if (mpSbs != NULL)
            {
               QSplitter *pWidget = new QSplitter(Qt::Horizontal, NULL);

               SpatialDataView* pPPrimeView =
                  static_cast<SpatialDataView*>(pDesktop->createView(PRIMARY_CHIP_WIDGET_NAME, SPATIAL_DATA_VIEW, pWidget));
               if (pPPrimeView != NULL)
               {
                  UndoLock lock(pPPrimeView);

                  pPPrimeView->setPrimaryRasterElement(pChip);
                  pPPrimeView->createLayer(RASTER, pChip);
               }

               SpatialDataView* pSPrimeView =
                  static_cast<SpatialDataView*>(pDesktop->createView(SECONDARY_CHIP_WIDGET_NAME, SPATIAL_DATA_VIEW, pWidget));
               if (pSPrimeView != NULL)
               {
                  UndoLock lock(pSPrimeView);

                  pSPrimeView->setPrimaryRasterElement(pSPrime);
                  pSPrimeView->createLayer(RASTER, pSPrime);
               }

               pPPrimeView->linkView(pSPrimeView, MIRRORED_LINK);
               pSPrimeView->linkView(pPPrimeView, MIRRORED_LINK);
               mpSbs->setWidget(pWidget);
               QSize sz = pWidget->sizeHint();
               sz.setWidth(sz.width());
               QApplication::sendPostedEvents();
               pPPrimeView->zoomExtents();
               pSPrimeView->zoomExtents();
               mpSbs->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &DataFusionDlg::windowDeleted));
            }
            else
            {
               throw FusionException("ERROR DataFusionDlg: Unable to create Flicker window!", __LINE__, __FILE__);
            }
         }

         if (mpInputsPage->openOverlayTools())
         {
            Window* pWindow = mpDesktop->getWindow("Flicker Window", DOCK_WINDOW);
            if (pWindow != NULL) // the FlickerControls Plug-In was already invoked, so show the Window
            {
               DockWindow* pDockWindow = static_cast<DockWindow*>(pWindow);
               if (pDockWindow != NULL)
               {
                  pDockWindow->show();
               }
               else
               {
                  throw FusionException("Error DataFusionDlg: Dock Window type is incorrect!");
               }
            }
            else // no DockWindow, so invoke the FlickerControls
            {
               ExecutableResource flickerPlugIn("Flicker Controls");
               if (flickerPlugIn->execute() == true)
               {
                  // leave the plug-in out there so the DockWindow Plug-In stays around until the user destroys it
                  flickerPlugIn->releasePlugIn();
               }
            }
         }
         mModified[mpInputsPage] = false;
      }
      catch(AssertException& exc)
      {
         // AssertException is a bug. Report line numbers to user.
         mProgressTracker.report(exc.getText(), 0, ERRORS, true);
      }
      catch(FusionException& exc)
      {
         // don't report line numbers to the user
         mProgressTracker.report(exc.toString(false), 0, ERRORS, true);
      }
   }
}

void DataFusionDlg::copyColorMap(RasterLayer& destination, const RasterLayer& source) const
{
   // copy parameters from source into destination
   destination.setColorMap(source.getColorMapName(), source.getColorMap());
   destination.setDisplayMode(source.getDisplayMode());

   destination.setStretchType(GRAYSCALE_MODE, source.getStretchType(GRAYSCALE_MODE));
   destination.setStretchType(RGB_MODE, source.getStretchType(RGB_MODE));

   destination.setAlpha(source.getAlpha());
   destination.setComplexComponent(source.getComplexComponent());

   double lower, upper;
   source.getStretchValues(RED, lower, upper); // get values
   // convert values
   lower = source.convertStretchValue(RED, lower, RAW_VALUE);
   upper = source.convertStretchValue(RED, upper, RAW_VALUE);
   // set method & values
   destination.setStretchUnits(RED, RAW_VALUE);
   destination.setStretchValues(RED, lower, upper);

   source.getStretchValues(GREEN, lower, upper); // get values
   // convert values
   lower = source.convertStretchValue(GREEN, lower, RAW_VALUE);
   upper = source.convertStretchValue(GREEN, upper, RAW_VALUE);
   // set method & values
   destination.setStretchUnits(GREEN, RAW_VALUE);
   destination.setStretchValues(GREEN, lower, upper);

   source.getStretchValues(BLUE, lower, upper); // get values
   // convert values
   lower = source.convertStretchValue(BLUE, lower, RAW_VALUE);
   upper = source.convertStretchValue(BLUE, upper, RAW_VALUE);
   // set method & values
   destination.setStretchUnits(BLUE, RAW_VALUE);
   destination.setStretchValues(BLUE, lower, upper);

   source.getStretchValues(GRAY, lower, upper); // get values
   // convert values
   lower = source.convertStretchValue(GRAY, lower, RAW_VALUE);
   upper = source.convertStretchValue(GRAY, upper, RAW_VALUE);
   // set method & values
   destination.setStretchUnits(GRAY, RAW_VALUE);
   destination.setStretchValues(GRAY, lower, upper);
}

string DataFusionDlg::getNewFilename(const string& filename, const char* pAppend)
{
   string newName = filename;
   if (pAppend != NULL)
   {
      if (filename.empty() == true)
      {
         return newName;
      }

      int loc = filename.rfind('.');
      if (loc > 0)
      {
         newName = newName.insert(loc, pAppend);
      }
      else
      {
         newName = newName.append(pAppend);
      }
   }
   return newName;
}

RasterElement* DataFusionDlg::createChip(RasterElement* pRaster,
                                              unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2,
                                              const std::string& appendage,
                                              int alphaValue, int zoomFactor) // default to opaque
{
   Service<ModelServices> pModel;

   int fromBand = 0;
   unsigned int toBand = 0;

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());

   if (NN(pDescriptor))
   {
      vector<DimensionDescriptor> bands, chipRows, chipCols;
   
      for (unsigned int i = y1; i <= y2; ++i)
      {
         chipRows.push_back(pDescriptor->getActiveRow(i));
      }

      // Columns
      for (unsigned int i = x1; i <= x2; ++i)
      {
         chipCols.push_back(pDescriptor->getActiveColumn(i));
      }

      string pPrimeName = pRaster->getName();
      pPrimeName = getNewFilename(pPrimeName, appendage.c_str());

      DataElement* pElement = pModel->getElement(pPrimeName, "RasterElement", NULL);
      if (pElement != NULL)
      {
         pModel->destroyElement(pElement);
      }

      return pRaster->createChip(NULL, pPrimeName, chipRows, chipCols, bands);
   }
   return NULL;
}

RasterElement* DataFusionDlg::createSecondaryImageChip(RasterElement* pSecondaryRaster,
                                                     const Vector<double>& P, const Vector<double>& Q,
                                                     unsigned int newCols, unsigned int newRows,
                                                     int llX, int llY, int zoomFactor, bool inMemory)
{
   if (pSecondaryRaster == NULL)
   {
      return NULL;
   }

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pSecondaryRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   RasterElement* pRaster = NULL;
   EncodingType dataEncoding = pDescriptor->getDataType();

   switchOnEncoding(dataEncoding, pRaster = poly_2D,
                      NULL, // NULL for the data type
                      pSecondaryRaster, P, Q, newCols, newRows, llX, llY, zoomFactor, mProgressTracker,
                      inMemory);
   mProgressTracker.nextStage();

   RasterDataDescriptor* pNewDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
   VERIFYRV(pNewDescriptor != NULL, NULL);

   pNewDescriptor->setYPixelSize(1.0 / zoomFactor);
   pNewDescriptor->setXPixelSize(1.0 / zoomFactor);

   return pRaster;
}

bool DataFusionDlg::copyLayersToView(SpatialDataView& view, int scaleFactor) const
{
   SpatialDataView* pPrimaryView = mpInputsPage->getPrimaryView();
   REQUIRE(pPrimaryView != NULL);

   vector<Layer*> layersToCopy = mpLayersPage->getSelectedLayers();

   const QString question = "Some of the layers selected no longer exist.\n"
                            "Do you want to continue the fusion process anyway and fuse only the available layers?";

   QMessageBox qBox(windowTitle(), question,
                    QMessageBox::Warning, QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No | QMessageBox::Escape, QMessageBox::Cancel, NULL);

   if (mpLayersPage->areAllSelectedLayersAvailable() == false)
   {
      int response = qBox.exec();
      if (response != QMessageBox::Yes)
      {
         return false; // notify caller to perform cleanup
      }
   }

   LayerList* pLayerList = view.getLayerList();
   REQUIRE(pLayerList != NULL);

   // this method returns bool but was already called to create the image chips.
   int x1, y1, x2, y2;
   mpInputsPage->getRoiBoundingBox(x1, y1, x2, y2);

   LocationType delta(x1, y1);

   // actually copy layers over
   for (vector<Layer*>::const_iterator lit = layersToCopy.begin(); lit != layersToCopy.end(); ++lit)
   {
      Layer* pLayer = *lit;
      if (pLayer != NULL)
      {
         LayerType eLayerType = pLayer->getLayerType();
         const GraphicLayer* pGraphic = dynamic_cast<const GraphicLayer*>(pLayer);
         // FusionLayersSelectPage will only provide layers that can be copied by this method.

         RasterElement* pRaster = dynamic_cast<RasterElement*>(pLayer->getDataElement());
         if (pRaster != NULL)
         {
            unsigned int newCols = 0;
            unsigned int newRows = 0;

            RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
            REQUIRE(pDescriptor != NULL);

            newCols = pDescriptor->getColumnCount();
            newRows = pDescriptor->getRowCount();

            LocationType offset(pLayer->getXOffset(), pLayer->getYOffset());
            // make RasterElement on the same coordinates as scene by applying offset
            LocationType rmStart = offset;
            LocationType rmEnd = rmStart + LocationType(newCols, newRows);

            rmStart.mX = max(rmStart.mX, static_cast<double>(x1));
            rmStart.mY = max(rmStart.mY, static_cast<double>(y1));
            rmEnd.mX = min(rmEnd.mX, static_cast<double>(x2));
            rmEnd.mY = min(rmEnd.mY, static_cast<double>(y2));

            // compute new offsets
            LocationType newOffset;
            newOffset.mX = max(offset.mX - x1, 0.0);
            newOffset.mY = max(offset.mY - y1, 0.0);

            // convert from scene coordinates to RasterElement coordinates by removing offset
            rmStart -= offset;
            rmEnd -= offset;
            if (rmEnd.mX >= newCols)
            {
               rmEnd.mX = newCols-1;
            }
            if (rmEnd.mY >= newRows)
            {
               rmEnd.mY = newRows-1;
            }

            REQUIRE(rmStart.mX >= 0);
            REQUIRE(rmStart.mY >= 0);
            REQUIRE(rmEnd.mX >= 0);
            REQUIRE(rmEnd.mY >= 0);

            string newName(pRaster->getName() + "_resampled");
            const vector<DimensionDescriptor> &srcRows = pDescriptor->getRows();
            vector<DimensionDescriptor> rows = RasterUtilities::subsetDimensionVector(srcRows, srcRows[rmStart.mY], srcRows[rmEnd.mY]);
            const vector<DimensionDescriptor> &srcCols = pDescriptor->getColumns();
            vector<DimensionDescriptor> cols = RasterUtilities::subsetDimensionVector(srcCols, srcCols[rmStart.mX], srcCols[rmEnd.mX]);

            RasterElement* pResampled = pRaster->createChip(NULL, "_resampled", rows, cols);

            if (pResampled == NULL)
            {
               string msg("WARNING DataFusionDlg: Unable to create chip of " + pRaster->getName() + "!");
               mProgressTracker.report(msg, 0, WARNING, true);
               continue;
            }
            Layer* pCopiedLayer = view.createLayer(eLayerType, pResampled);
            pCopiedLayer->setXOffset(newOffset.mX);
            pCopiedLayer->setYOffset(newOffset.mY);
            REQUIRE(pCopiedLayer != NULL);

            // TODO: Upgrade to not just allow RasterLayer
            if (eLayerType == RASTER)
            {
               RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pCopiedLayer);
               REQUIRE(pCopiedLayer != NULL);
               RasterLayer* pSourceLayer = dynamic_cast<RasterLayer*>(pLayer);
               REQUIRE(pSourceLayer != NULL);

               copyColorMap(*pRaster, *pSourceLayer);
            }
         }
         else if (pGraphic != NULL)
         {
            Layer* pCopy = pGraphic->copy(string(), true);
            REQUIRE(pCopy != NULL);

            GraphicLayer* pGraphicCopy = static_cast<GraphicLayer*>(pCopy);
            typedef list<GraphicObject*> GraphicList;

            GraphicList graphics;
            pGraphicCopy->getObjects(graphics);
            for (GraphicList::iterator it = graphics.begin(); it != graphics.end(); ++it)
            {
               // normalize GraphicObject to match origin of the chip
               GraphicObject *pObj = *it;
               if (pObj != NULL)
               {
                  LocationType ll = pObj->getLlCorner(), ur = pObj->getUrCorner();
                  ll -= delta;
                  ur -= delta;

                  // account for cube being enlarged by scaleFactor x scaleFactor
                  ll = ll * scaleFactor; // *= operator is not implemented
                  ur = ur * scaleFactor;

                  pObj->setBoundingBox(ll, ur);
               }
            }
            pGraphicCopy->deselectAllObjects();
            view.addLayer(pCopy);
         }
      }
   }
   view.refresh();
   return true;
}
