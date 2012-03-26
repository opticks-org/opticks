/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtCore/QFileInfo>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QColorDialog>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QIcon>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>
#include <QtGui/QVBoxLayout>

#include "AnnotationElementAdapter.h"
#include "AnnotationLayerAdapter.h"
#include "AppVerify.h"
#include "DataDescriptorAdapter.h"
#include "DataElementImp.h"
#include "DesktopServices.h"
#include "glCommon.h"
#include "Layer.h"
#include "LayerList.h"
#include "OverviewWindow.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataViewImp.h"
#include "SpatialDataWindow.h"
#include "TrailObject.h"
#include "TrailObjectImp.h"
#include "ZoomPanWidget.h"

using namespace std;

OverviewWindow::OverviewWindow(SpatialDataViewImp* pView, QWidget* parent) :
   QDialog(parent),
   mpView(pView),
   mpOverview(NULL),
   mpSelectionWidget(NULL),
   mpTrailLayer(NULL),
   mpTrail(NULL),
   mZoomThreshold(100)
{
   // get trail defaults from ConfigurationSettings
   mTrailColor = SpatialDataWindow::getSettingOverviewTrailColor();
   mZoomThreshold = SpatialDataWindow::getSettingOverviewTrailThreshold();

   // View widget
   mpOverview = createOverview();
   mpSelectionWidget = new ZoomPanWidget(mpOverview, this);

   // Overview trail object
   mpTrail = createSnailTrail(mpOverview);

   // Context menu
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Overview Window";

   QAction* pClearAction = new QAction("Clear the Trail", this);
   pClearAction->setAutoRepeat(false);
   pDesktop->initializeAction(pClearAction, shortcutContext);
   addAction(pClearAction);

   QAction* pSeparatorAction = new QAction(this);
   pSeparatorAction->setSeparator(true);
   addAction(pSeparatorAction);

   QAction* pColorAction = new QAction("Change Trail color...", this);
   pColorAction->setAutoRepeat(false);
   pDesktop->initializeAction(pColorAction, shortcutContext);
   addAction(pColorAction);

   QAction* pOpacityAction = new QAction("Change Trail opacity...", this);
   pOpacityAction->setAutoRepeat(false);
   pDesktop->initializeAction(pOpacityAction, shortcutContext);
   addAction(pOpacityAction);

   QAction* pThresholdAction = new QAction("Change zoom threshold...", this);
   pThresholdAction->setAutoRepeat(false);
   pDesktop->initializeAction(pThresholdAction, shortcutContext);
   addAction(pThresholdAction);

   QAction* pSeparator2Action = new QAction(this);
   pSeparator2Action->setSeparator(true);
   addAction(pSeparator2Action);

   QAction* pSnapshotAction = new QAction("Take a snapshot", this);
   pSnapshotAction->setAutoRepeat(false);
   pSnapshotAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
   pSnapshotAction->setToolTip("Take a snapshot");
   pSnapshotAction->setStatusTip("Copies a snapshot of the view into the clipboard");
   addAction(pSnapshotAction);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(0);
   pLayout->addWidget(mpSelectionWidget, 10);

   // Initialization
   setModal(false);
   setContextMenuPolicy(Qt::DefaultContextMenu);
   setMinimumSize(200, 200);
   int maxWidth = 400;
   int maxHeight = 400;
   setMaximumSize(maxWidth, maxHeight);
   updateSelectionBox();

   if (mpTrail != NULL)
   {
      mpTrail->setStencilBufferSize(maxWidth, maxHeight);
   }

   const std::vector<LocationType> selectionBox = mpSelectionWidget->getSelection();
   updateView(selectionBox);

   // Set the window icon
   if (mpView != NULL)
   {
      QIcon viewIcon = mpView->windowIcon();
      if (viewIcon.isNull() == false)
      {
         setWindowIcon(viewIcon);
      }
   }

   // Set the caption of the dialog
   QString strCaption = "Overview";
   if (mpView != NULL)
   {
      QString strName = QString::fromStdString(mpView->getName());
      if (strName.isEmpty() == false)
      {
         QFileInfo fileInfo(strName);
         QString strFilename = fileInfo.fileName();
         if (strFilename.isEmpty() == false)
         {
            strCaption += ": " + strFilename;
         }
      }
   }

   setWindowTitle(strCaption);

   // Connections
   VERIFYNR(connect(mpSelectionWidget, SIGNAL(selectionChanged(const std::vector<LocationType>&)), this,
      SLOT(updateView(const std::vector<LocationType>&))));
   VERIFYNR(connect(mpView, SIGNAL(displayAreaChanged()), this, SLOT(updateSelectionBox())));
   VERIFYNR(connect(pClearAction, SIGNAL(triggered()), this, SLOT(clearTrail())));
   VERIFYNR(connect(pColorAction, SIGNAL(triggered()), this, SLOT(changeTrailColor())));
   VERIFYNR(connect(pOpacityAction, SIGNAL(triggered()), this, SLOT(changeTrailOpacity())));
   VERIFYNR(connect(pThresholdAction, SIGNAL(triggered()), this, SLOT(changeTrailThreshold())));
   VERIFYNR(connect(pSnapshotAction, SIGNAL(triggered()), this, SLOT(takeSnapshot())));
}

OverviewWindow::~OverviewWindow()
{
   // Destroy the annotation layer and element
   if (mpTrailLayer != NULL)
   {
      DataElementImp* pElement = dynamic_cast<DataElementImp*>(mpTrailLayer->getDataElement());
      mpOverview->deleteLayer(mpTrailLayer);

      delete pElement;
   }
}

bool OverviewWindow::eventFilter(QObject* o, QEvent* e)
{
   if ((e->type() == QEvent::MouseButtonDblClick) || (e->type() == QEvent::Wheel))
   {
      return true;
   }
   else if (e->type() == QEvent::ContextMenu)
   {
      QContextMenuEvent* pMenuEvent = static_cast<QContextMenuEvent*>(e);
      contextMenuEvent(pMenuEvent);
      return true;
   }

   return QDialog::eventFilter(o, e);
}

void OverviewWindow::showEvent(QShowEvent* e)
{
   QDialog::showEvent(e);

   if (mpSelectionWidget != NULL)
   {
      mpSelectionWidget->zoomExtents();
   }

   clearTrail();
   emit visibilityChanged(true);
}

void OverviewWindow::resizeEvent(QResizeEvent* e)
{
   QDialog::resizeEvent(e);

   if (mpSelectionWidget != NULL)
   {
      mpSelectionWidget->zoomExtents();
   }

   clearTrail();
}

void OverviewWindow::closeEvent(QCloseEvent* e)
{
   QDialog::closeEvent(e);
   emit visibilityChanged(false);
}

void OverviewWindow::contextMenuEvent(QContextMenuEvent* pEvent)
{
   if (pEvent != NULL)
   {
      const QPoint& menuPos = pEvent->globalPos();
      QMenu::exec(actions(), menuPos);
   }
}

SpatialDataViewImp* OverviewWindow::createOverview()
{
   SpatialDataViewImp* pOverview = NULL;
   if (mpView != NULL)
   {
      pOverview = dynamic_cast<SpatialDataViewImp*>(mpView->copy());
      VERIFYRV(pOverview != NULL, NULL);
      pOverview->installEventFilter(this);

      LayerList* pLayerList = NULL;
      pLayerList = pOverview->getLayerList();
      if (pLayerList != NULL)
      {
         // get primary raster layer from data view
         LayerList* pSDVlist = mpView->getLayerList();
         VERIFYRV(pSDVlist != NULL, NULL);
         DataElement* pPrimElem = pSDVlist->getPrimaryRasterElement();
         VERIFYRV(pPrimElem != NULL, NULL);
         Layer* pPrimLayer = pSDVlist->getLayer(RASTER, pPrimElem);
         VERIFYRV(pPrimLayer != NULL, NULL);
         string primName(pPrimLayer->getName());

         vector<Layer*> layers;
         pLayerList->getLayers(layers);
         for (unsigned int i = 0; i < layers.size(); i++)
         {
            Layer* pLayer = NULL;
            pLayer = layers.at(i);
            string layerName(pLayer->getName());
            if (pLayer->getLayerType()==RASTER && layerName==primName)
            {
               pPrimLayer->linkLayer(pLayer);

               // reset the scale to what is in the model
               DataElement* pElement = pLayer->getDataElement();
               VERIFYRV(pElement != NULL, NULL);

               const RasterDataDescriptor* pDescriptor =
                  dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
               VERIFYRV(pDescriptor != NULL, NULL);

               pLayer->setYScaleFactor(pDescriptor->getYPixelSize());
               pLayer->setXScaleFactor(pDescriptor->getXPixelSize());
            }
            else
            {
               pOverview->deleteLayer(pLayer);
            }
         }
         pOverview->resetOrientation();
      }
   }

   return pOverview;
}

void OverviewWindow::updateView(const vector<LocationType>& selectionArea)
{
   if ((mpView == NULL) || (selectionArea.size() != 4))
   {
      return;
   }

   LayerList* pLayerList = mpView->getLayerList();
   VERIFYNRV(pLayerList != NULL);
   Layer* pLayer = pLayerList->getLayer(RASTER, pLayerList->getPrimaryRasterElement());
   VERIFYNRV(pLayer != NULL);
   LocationType worldLl;
   LocationType worldUr;
   pLayer->translateDataToWorld(selectionArea[0].mX, selectionArea[0].mY, worldLl.mX, worldLl.mY);
   pLayer->translateDataToWorld(selectionArea[2].mX, selectionArea[2].mY, worldUr.mX, worldUr.mY);
   
   // Update the view
   mpView->zoomToBox(worldLl, worldUr);
   mpView->repaint();
}

void OverviewWindow::updateSelectionBox()
{
   if ((mpView == NULL) || (mpSelectionWidget == NULL))
   {
      return;
   }

   // Get the pixel coordinates at the view corners
   LocationType lowerLeft;
   LocationType upperLeft;
   LocationType upperRight;
   LocationType lowerRight;
   mpView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

   // Disconnect the signal to update the view since the values are obtained from the view
   disconnect(mpSelectionWidget, SIGNAL(selectionChanged(const std::vector<LocationType>&)), this,
      SLOT(updateView(const std::vector<LocationType>&)));

   LayerList* pLayerList = mpView->getLayerList();
   VERIFYNRV(pLayerList != NULL);
   Layer* pLayer = pLayerList->getLayer(RASTER, pLayerList->getPrimaryRasterElement());
   VERIFYNRV(pLayer != NULL);

   // Set the selection area
   vector<LocationType> selectionArea;
   LocationType dataCoord;
   pLayer->translateWorldToData(lowerLeft.mX, lowerLeft.mY, dataCoord.mX, dataCoord.mY);
   selectionArea.push_back(dataCoord);
   pLayer->translateWorldToData(lowerRight.mX, lowerRight.mY, dataCoord.mX, dataCoord.mY);
   selectionArea.push_back(dataCoord);
   pLayer->translateWorldToData(upperRight.mX, upperRight.mY, dataCoord.mX, dataCoord.mY);
   selectionArea.push_back(dataCoord);
   pLayer->translateWorldToData(upperLeft.mX, upperLeft.mY, dataCoord.mX, dataCoord.mY);
   selectionArea.push_back(dataCoord);

   // update snail trail if zoom factor greater than or equal zoom threshold
   if ((mpTrail != NULL) && (static_cast<int>(mpView->getZoomPercentage() + 0.5) >= mZoomThreshold))
   {
      // Translate each point into a screen location
      if (mpOverview != NULL)
      {
         mpOverview->translateWorldToScreen(lowerLeft.mX, lowerLeft.mY, lowerLeft.mX, lowerLeft.mY);
         mpOverview->translateWorldToScreen(lowerRight.mX, lowerRight.mY, lowerRight.mX, lowerRight.mY);
         mpOverview->translateWorldToScreen(upperLeft.mX, upperLeft.mY, upperLeft.mX, upperLeft.mY);
         mpOverview->translateWorldToScreen(upperRight.mX, upperRight.mY, upperRight.mX, upperRight.mY);

         mpTrail->addToStencil(lowerLeft, lowerRight, upperLeft, upperRight);
      }
   }

   mpSelectionWidget->setSelection(selectionArea);

   // Reconnect the signal to update the view
   connect(mpSelectionWidget, SIGNAL(selectionChanged(const std::vector<LocationType>&)), this,
      SLOT(updateView(const std::vector<LocationType>&)));
}

TrailObjectImp* OverviewWindow::createSnailTrail(SpatialDataViewImp* pOverview)
{
   VERIFYRV(pOverview != NULL, NULL);

   // Annotation layer
   DataDescriptorAdapter descriptor("Snail Trail", "AnnotationElement", NULL);
   mpTrailLayer = new AnnotationLayerAdapter(SessionItemImp::generateUniqueId(), "Snail Trail", 
                      new AnnotationElementAdapter(descriptor, SessionItemImp::generateUniqueId()));
   pOverview->addLayer(mpTrailLayer);
   TrailObjectImp* pTrailImp(NULL);
   if (mpTrailLayer != NULL)
   {
      TrailObject* pTrail = dynamic_cast<TrailObject*>(mpTrailLayer->addObject(TRAIL_OBJECT));
      VERIFYRV(pTrail != NULL, NULL);
      double minX;
      double minY;
      double maxX;
      double maxY;
      pOverview->getExtents(minX, minY, maxX, maxY);
      pTrail->setAlpha(0.3);
      pTrail->setBoundingBox(LocationType(minX, minY), LocationType(maxX, maxY));
      pTrail->setFillStyle(SOLID_FILL);
      pTrail->setHatchStyle(SOLID);
      pTrail->setFillState(true);
      pTrail->setLineState(false);
      pTrail->setFillColor(mTrailColor);

      pTrailImp = dynamic_cast<TrailObjectImp*>(pTrail);
   }

   return pTrailImp;
}

void OverviewWindow::changeTrailColor()
{
   QColor oldColor = COLORTYPE_TO_QCOLOR(mTrailColor);
   QColor newColor = QColorDialog::getColor(oldColor, this);
   if (newColor.isValid())
   {
      mTrailColor.mRed = newColor.red();
      mTrailColor.mGreen = newColor.green();
      mTrailColor.mBlue = newColor.blue();
      mpTrail->setFillColor(mTrailColor);
      updateSelectionBox();
   }
}

void OverviewWindow::clearTrail()
{
   if (mpTrail != NULL)
   {
      mpTrail->clearStencil();
      updateSelectionBox();
   }
}

void OverviewWindow::changeTrailOpacity()
{
   int oldAlpha = static_cast<int>(mTrailColor.mAlpha / 2.550f + 0.5);
   bool bOk;
   int newAlpha = QInputDialog::getInteger(this, "Set Overview Trail Opacity",
      "Set from 0% (transparent) to 100% (opaque)", oldAlpha, 0, 100, 1, &bOk);
   if (bOk)
   {
      newAlpha = static_cast<int>(newAlpha * 2.550f + 0.5);
      if (newAlpha > 255)
      {
         newAlpha = 255;
      }

      if (newAlpha < 0)
      {
         newAlpha = 0;
      }

      mTrailColor.mAlpha = newAlpha;
      mpTrail->setFillColor(mTrailColor);
      updateSelectionBox();
   }
}

void OverviewWindow::changeTrailThreshold()
{
   bool bOk;
   int newZoom = QInputDialog::getInteger(this, "Set Trail Zoom Threshold",
      "Set lowest zoom percentage for marking Trail.\nThis action will reset the Overview Trail.", 
      mZoomThreshold, 1, 1000, 10, &bOk);
   if (bOk)
   {
      mZoomThreshold = newZoom;
      clearTrail();
   }
}

void OverviewWindow::takeSnapshot()
{
   View* pView = dynamic_cast<View*>(mpOverview);
   if (pView != NULL)
   {
      QImage image;
      pView->getCurrentImage(image);
      if (image.isNull() == false)
      {
         QClipboard* pClipboard = QApplication::clipboard();
         pClipboard->setImage(image);
      }
   }
}
