/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QEvent>
#include <QtGui/QActionGroup>
#include <QtGui/QToolButton>
#include <QtGui/QWidgetAction>

#include "AoiElement.h"
#include "AoiMergeDlg.h"
#include "AoiLayer.h"
#include "AoiLayerImp.h"
#include "AoiToolBar.h"
#include "AoiToolBarImp.h"
#include "AppVerify.h"
#include "BitMaskImp.h"
#include "ColorMenu.h"
#include "DesktopServices.h"
#include "GraphicObjectImp.h"
#include "GraphicObjectTypeGrid.h"
#include "GraphicGroup.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "PixmapGrid.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "SymbolTypeGrid.h"
#include "Undo.h"
#include "WorkspaceWindow.h"

#include <string>
#include <vector>
using namespace std;
XERCES_CPP_NAMESPACE_USE

AoiToolBarImp::AoiToolBarImp(const string& id, QWidget* pParent) :
   ToolBarImp(id, "AOI", pParent),
   mpDraw(NULL),
   mpErase(NULL),
   mpToggle(NULL),
   mpAoiMoveMode(NULL),
   mpAoiShowLabels(NULL),
   mpAoiShowPointLabels(NULL),
   mpAddMode(NULL),
   mpTool(NULL),
   mpEraseAll(NULL),
   mpToggleAll(NULL),
   mpMerge(NULL),
   mpSymbolButton(NULL),
   mpColorMenu(NULL),
   mpAoiLayer(NULL)
{
   string shortcutContext = windowTitle().toStdString();

   QActionGroup* pSelectionGroup = new QActionGroup(this);
   if (pSelectionGroup != NULL)
   {
      pSelectionGroup->setExclusive(true);

      string mouseModeContext = shortcutContext + string("/Mouse Mode");

      mpDraw = pSelectionGroup->addAction(QIcon(":/icons/DrawPixel"), "Draw Mode");
      mpDraw->setAutoRepeat(false);
      mpDraw->setCheckable(true);
      mpDraw->setStatusTip("Sets the selection mode to add selected pixels to the AOI");
      addButton(mpDraw, mouseModeContext);
      VERIFYNR(connect(mpDraw, SIGNAL(triggered()), this, SLOT(selectionObjectChanged())));

      mpErase = pSelectionGroup->addAction(QIcon(":/icons/ErasePixel"), "Erase Mode");
      mpErase->setAutoRepeat(false);
      mpErase->setCheckable(true);
      mpErase->setStatusTip("Sets the selection mode to remove selected pixels from the AOI");
      addButton(mpErase, mouseModeContext);
      VERIFYNR(connect(mpErase, SIGNAL(triggered()), this, SLOT(selectionObjectChanged())));

      mpToggle = pSelectionGroup->addAction(QIcon(":/icons/TogglePixel"), "Toggle Mode");
      mpToggle->setAutoRepeat(false);
      mpToggle->setCheckable(true);
      mpToggle->setStatusTip("Sets the selection mode to toggle the state of the selected pixel in the AOI");
      addButton(mpToggle, mouseModeContext);
      VERIFYNR(connect(mpToggle, SIGNAL(triggered()), this, SLOT(selectionObjectChanged())));

      mpAoiMoveMode = pSelectionGroup->addAction(QIcon(":/icons/Pan"), "Move AOI Object");
      mpAoiMoveMode->setAutoRepeat(false);
      mpAoiMoveMode->setCheckable(true);
      mpAoiMoveMode->setStatusTip("Moves the vector AOI object");
      addButton(mpAoiMoveMode, mouseModeContext);
      VERIFYNR(connect(mpAoiMoveMode, SIGNAL(triggered()), this, SLOT(selectionObjectChanged())));
   }

   addSeparator();

   mpAoiShowLabels = new QAction(QIcon(":/icons/AoiShowLabels"), "Show Name Label", this);
   mpAoiShowLabels->setAutoRepeat(false);
   mpAoiShowLabels->setCheckable(true);
   mpAoiShowLabels->setStatusTip("Displays the AOI name in the layer");
   mpAoiShowLabels->setChecked(true);
   addButton(mpAoiShowLabels, shortcutContext);
   VERIFYNR(connect(mpAoiShowLabels, SIGNAL(toggled(bool)), this, SLOT(changeShowLabelState())));

   mpAoiShowPointLabels = new QAction(QIcon(":/icons/AoiShowPointLabels"), "Show Shape Label", this);
   mpAoiShowPointLabels->setAutoRepeat(false);
   mpAoiShowPointLabels->setCheckable(true);
   mpAoiShowPointLabels->setStatusTip("Displays the shape name in the layer");
   mpAoiShowPointLabels->setChecked(true);
   addButton(mpAoiShowPointLabels, shortcutContext);
   VERIFYNR(connect(mpAoiShowPointLabels, SIGNAL(triggered(bool)), this, SLOT(setShowPointLabelState(bool))));

   // AOI add mode
   mpAddMode = new AoiAddModeButton(this);
   mpAddMode->setStatusTip("Specifies how new AOI shapes are added to the view");
   mpAddMode->setToolTip("Add Mode");
   addWidget(mpAddMode);
   VERIFYNR(connect(mpAddMode, SIGNAL(valueChanged(AoiAddMode)), this, SLOT(setAddMode(AoiAddMode))));

   // Selection tool
   mpTool = new GraphicObjectTypeButton(GraphicObjectTypeGrid::VIEW_AOI, this);
   mpTool->setStatusTip("Specifies how pixels are selected or deselected in the AOI");
   mpTool->setToolTip("AOI Pixel Selection Tool");
   addWidget(mpTool);
   VERIFYNR(connect(mpTool, SIGNAL(valueChanged(GraphicObjectType)), this, SLOT(setSelectionTool(GraphicObjectType))));

   addSeparator();

   mpEraseAll = new QAction(QIcon(":/icons/EraseAll"), "Erase All", this);
   mpEraseAll->setAutoRepeat(false);
   mpEraseAll->setStatusTip("Deselects all pixels on the current area of interest");
   addButton(mpEraseAll, shortcutContext);
   VERIFYNR(connect(mpEraseAll, SIGNAL(triggered()), this, SLOT(clearAoi())));

   mpToggleAll = new QAction(QIcon(":/icons/ToggleAll"), "Toggle All", this);
   mpToggleAll->setAutoRepeat(false);
   mpToggleAll->setStatusTip("Selects or deselects all pixels on the current area of interest");
   addButton(mpToggleAll, shortcutContext);
   VERIFYNR(connect(mpToggleAll, SIGNAL(triggered()), this, SLOT(invertAoi())));

   addSeparator();

   mpMerge = new QAction(QIcon(":/icons/Merge"), "Merge", this);
   mpMerge->setAutoRepeat(false);
   mpMerge->setStatusTip("Combines selected pixels from multiple areas of interest");
   addButton(mpMerge, shortcutContext);
   VERIFYNR(connect(mpMerge, SIGNAL(triggered()), this, SLOT(mergeAoi())));

   addSeparator();

   // Marker symbol
   mpSymbolButton = new SymbolTypeButton(this);
   mpSymbolButton->setSyncIcon(false);
   mpSymbolButton->setIcon(QIcon(":/icons/Shape"));
   mpSymbolButton->setBorderedSymbols(true);
   mpSymbolButton->setStatusTip("Changes the pixel marker shape for the current area of interest");
   mpSymbolButton->setToolTip("Marker Symbol");
   VERIFYNR(connect(mpSymbolButton, SIGNAL(valueChanged(SymbolType)), this, SLOT(setAoiSymbol(SymbolType))));
   addWidget(mpSymbolButton);

   // Marker color
   mpColorMenu = new ColorMenu(this);
   if (mpColorMenu != NULL)
   {
      QAction* pColorAction = mpColorMenu->menuAction();
      if (pColorAction != NULL)
      {
         pColorAction->setIcon(QIcon(":/icons/AoiColor"));
         pColorAction->setStatusTip("Changes the pixel marker color for the current area of interest");
         pColorAction->setToolTip("Marker Color");
         VERIFYNR(connect(pColorAction, SIGNAL(triggered()), mpColorMenu, SLOT(setCustomColor())));

         addAction(pColorAction);
      }

      VERIFYNR(connect(mpColorMenu, SIGNAL(aboutToShow()), this, SLOT(initializeColorMenu())));
      VERIFYNR(connect(mpColorMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setAoiColor(const QColor&))));
   }

   setSelectionTool(AoiToolBar::getSettingSelectionTool(), DRAW);
}

AoiToolBarImp::~AoiToolBarImp()
{}

Layer* AoiToolBarImp::getAoiLayer() const
{
   return mpAoiLayer;
}

bool AoiToolBarImp::setAoiLayer(Layer* pLayer)
{
   if (pLayer == mpAoiLayer)
   {
      return false;
   }

   if (pLayer != NULL)
   {
      if (pLayer->getLayerType() != AOI_LAYER)
      {
         return false;
      }
   }

   if (mpAoiLayer != NULL)
   {
      AoiLayerImp* pAoiLayerImp = dynamic_cast<AoiLayerImp*>(mpAoiLayer);
      if (pAoiLayerImp != NULL)
      {
         VERIFYNR(disconnect(this, SIGNAL(graphicObjectTypeChanged(GraphicObjectType)),
            pAoiLayerImp, SLOT(setCurrentGraphicObjectType(GraphicObjectType))));
         VERIFYNR(disconnect(this, SIGNAL(modeChanged(ModeType)), pAoiLayerImp, SLOT(setMode(ModeType))));
         VERIFYNR(disconnect(pAoiLayerImp, SIGNAL(currentTypeChanged(GraphicObjectType)), this,
            SLOT(setSelectionTool(GraphicObjectType))));
         VERIFYNR(disconnect(pAoiLayerImp, SIGNAL(modeChanged(ModeType)), this, SLOT(setSelectionMode(ModeType))));
         VERIFYNR(disconnect(pAoiLayerImp, SIGNAL(showLabelsChanged(bool)), this, SLOT(setShowPointLabelState(bool))));
      }

      mpAoiLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AoiToolBarImp::aoiLayerDeleted));
   }

   mpAoiLayer = static_cast<AoiLayer*>(pLayer);

   if (mpAoiLayer != NULL)
   {
      AoiLayerImp* pAoiLayerImp = dynamic_cast<AoiLayerImp*>(mpAoiLayer);
      if (pAoiLayerImp != NULL)
      {
         VERIFYNR(connect(this, SIGNAL(graphicObjectTypeChanged(GraphicObjectType)),
            pAoiLayerImp, SLOT(setCurrentGraphicObjectType(GraphicObjectType))));
         VERIFYNR(connect(this, SIGNAL(modeChanged(ModeType)), pAoiLayerImp, SLOT(setMode(ModeType))));
         VERIFYNR(connect(pAoiLayerImp, SIGNAL(currentTypeChanged(GraphicObjectType)), this,
            SLOT(setSelectionTool(GraphicObjectType))));
         VERIFYNR(connect(pAoiLayerImp, SIGNAL(modeChanged(ModeType)), this, SLOT(setSelectionMode(ModeType))));
         VERIFYNR(connect(pAoiLayerImp, SIGNAL(showLabelsChanged(bool)), this, SLOT(setShowPointLabelState(bool))));

         pAoiLayerImp->setShowLabels(mpAoiShowPointLabels->isChecked());
      }

      mpAoiLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &AoiToolBarImp::aoiLayerDeleted));
      disconnect(mpSymbolButton, SIGNAL(valueChanged(SymbolType)), this, SLOT(setAoiSymbol(SymbolType)));
      mpSymbolButton->setCurrentValue(pAoiLayerImp->getSymbol());
      VERIFYNR(connect(mpSymbolButton, SIGNAL(valueChanged(SymbolType)), this, SLOT(setAoiSymbol(SymbolType))));
   }

   selectionObjectChanged();
   return true;
}

void AoiToolBarImp::setSelectionTool(GraphicObjectType eTool, ModeType eMode)
{
   // Mode
   if (eMode == DRAW)
   {
      mpDraw->activate(QAction::Trigger);
   }
   else if (eMode == ERASE)
   {
      mpErase->activate(QAction::Trigger);
   }
   else if (eMode == TOGGLE)
   {
      mpToggle->activate(QAction::Trigger);
   }
   else if (eTool == MOVE_OBJECT)
   {
      mpAoiMoveMode->activate(QAction::Trigger);
   }

   setSelectionTool(eTool);
}

GraphicObjectType AoiToolBarImp::getSelectionTool() const
{
   GraphicObjectType type = mpTool->getCurrentValue();
   if (mpAoiMoveMode->isChecked() == true)
   {
      type = MOVE_OBJECT;
   }

   return type;
}

ModeType AoiToolBarImp::getSelectionMode() const
{
   ModeType eMode;
   if (mpDraw->isChecked() == true)
   {
      eMode = DRAW;
   }
   else if (mpErase->isChecked() == true)
   {
      eMode = ERASE;
   }
   else if (mpToggle->isChecked() == true)
   {
      eMode = TOGGLE;
   }
   else if (mpAoiMoveMode->isChecked() == true)
   {
      eMode = AOI_MOVE;
   }

   return eMode;
}

AoiAddMode AoiToolBarImp::getAddMode() const
{
   return mpAddMode->getCurrentValue();
}

bool AoiToolBarImp::getAoiShowLabels() const
{
   return mpAoiShowLabels->isChecked();
}

bool AoiToolBarImp::getAoiShowPointLabels() const
{
   return mpAoiShowPointLabels->isChecked();
}

void AoiToolBarImp::aoiLayerDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<AoiLayer*>(&subject) == mpAoiLayer)
   {
      setAoiLayer(NULL);
      setEnabled(false);
   }
}

void AoiToolBarImp::mergeAoi()
{
   AoiMergeDlg mergeDlg(this);

   int iReturn = -1;
   iReturn = mergeDlg.exec();
   if (iReturn == QDialog::Rejected)
   {
      return;
   }

   QStringList strlAoiNames = mergeDlg.getMergeAoiNames();
   QString strNewAoi = mergeDlg.getOutputAoiName();
   bool bCombine = mergeDlg.combinePixels();

   int iCount = 0;
   iCount = strlAoiNames.count();
   if (iCount < 2)
   {
      return;
   }

   // Get the current spatial data view
   Service<DesktopServices> pDesktop;

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
   if (pView == NULL)
   {
      return;
   }

   UndoGroup group(pView, "Merge AOIs");

   // Get the parent raster element for the AOIs
   RasterElement* pRasterElement = NULL;

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList != NULL)
   {
      pRasterElement = pLayerList->getPrimaryRasterElement();
   }

   Service<ModelServices> pModel;

   AoiElement* pTargetAoi = static_cast<AoiElement*>(pModel->getElement(strNewAoi.toStdString(), "AoiElement",
      pRasterElement));
   if (pTargetAoi == NULL)
   {
      pTargetAoi = static_cast<AoiElement*>(pModel->createElement(strNewAoi.toStdString(), "AoiElement",
         pRasterElement));
      if (pTargetAoi == NULL)
      {
         return;
      }

      AoiLayerImp* pLayer = dynamic_cast<AoiLayerImp*>(pView->createLayer(AOI_LAYER, pTargetAoi,
         strNewAoi.toStdString()));
      if (pLayer != NULL)
      {
         // Get the group to set the layer in the group
         pLayer->getGroup();
      }
   }
   else if (pLayerList != NULL)
   {
      Layer* pLayer = pLayerList->getLayer(AOI_LAYER, pTargetAoi, strNewAoi.toStdString());
      if (pLayer != NULL)
      {
         pView->setFrontLayer(pLayer);
      }
   }

   BitMaskImp intersectMask;

   for (int i = 0; i < iCount; i++)
   {
      QString strName = strlAoiNames[i];
      if (strName.isEmpty() == false)
      {
         AoiElement* pAoi = static_cast<AoiElement*>(pModel->getElement(strName.toStdString(), "AoiElement",
            pRasterElement));
         if (pAoi != NULL && pAoi != pTargetAoi)
         {
            if (bCombine)
            {
               GraphicGroup* pGroup = pAoi->getGroup();
               VERIFYNRV(pGroup != NULL);
               const std::list<GraphicObject*>& objects = pGroup->getObjects();
               for (std::list<GraphicObject*>::const_iterator iter = objects.begin();
                  iter != objects.end(); ++iter)
               {
                  GraphicObject* pObj = *iter;
                  VERIFYNRV(pObj != NULL);
                  GraphicGroup* pTargetGroup = pTargetAoi->getGroup();
                  VERIFYNRV(pTargetGroup != NULL);
                  GraphicObjectImp* pTargetObj = dynamic_cast<GraphicObjectImp*>(
                     pTargetGroup->addObject(pObj->getGraphicObjectType()));
                  VERIFYNRV(pTargetObj != NULL);
                  pTargetObj->replicateObject(pObj);
               }
            }
            else
            {
               const BitMask* pMask = pAoi->getSelectedPoints();
               VERIFYNRV(pMask != NULL);
               if (i == 0)
               {
                  intersectMask.merge(*pMask);
               }
               else
               {
                  intersectMask.intersect(*pMask);
               }
            }
         }
      }
   }

   if (!bCombine)
   {
      pTargetAoi->clearPoints();
      pTargetAoi->addPoints(&intersectMask);
   }
}

void AoiToolBarImp::setSelectionTool(GraphicObjectType eTool)
{
   if (eTool != MOVE_OBJECT)
   {
      mpTool->setCurrentValue(eTool);
      // Activate the draw mode if necessary
      if (mpAoiMoveMode->isChecked() == true)
      {
         mpDraw->setChecked(true);
      }
      selectionObjectChanged();
   }
}

void AoiToolBarImp::setSelectionMode(ModeType eMode)
{
   if (eMode != getSelectionMode())
   {
      if (eMode == DRAW)
      {
         mpDraw->setChecked(true);
      }
      else if (eMode == ERASE)
      {
         mpErase->setChecked(true);
      }
      else if (eMode == TOGGLE)
      {
         mpToggle->setChecked(true);
      }
      else if (eMode == AOI_MOVE)
      {
         mpAoiMoveMode->setChecked(true);
      }

      selectionObjectChanged();
   }
}

void AoiToolBarImp::setAddMode(AoiAddMode mode)
{
   if (mpAddMode != NULL)
   {
      mpAddMode->setCurrentValue(mode);
   }
}

void AoiToolBarImp::clearAoi()
{
   AoiElement* pAoi = NULL;
   if (mpAoiLayer != NULL)
   {
      pAoi = dynamic_cast<AoiElement*>(mpAoiLayer->getDataElement());
   }

   VERIFYNRV(pAoi != NULL);
   pAoi->clearPoints();
}

void AoiToolBarImp::invertAoi()
{
   AoiElement* pAoi = NULL;
   if (mpAoiLayer != NULL)
   {
      pAoi = dynamic_cast<AoiElement*>(mpAoiLayer->getDataElement());
   }

   VERIFYNRV(pAoi != NULL);
   pAoi->toggleAllPoints();
}

void AoiToolBarImp::setAoiSymbol(SymbolType markerSymbol)
{
   if (mpAoiLayer != NULL)
   {
      mpAoiLayer->setSymbol(markerSymbol);
   }
}

void AoiToolBarImp::initializeColorMenu()
{
   AoiLayerImp* pLayer = dynamic_cast<AoiLayerImp*>(mpAoiLayer);
   if (pLayer != NULL)
   {
      QColor clrMarker = pLayer->getColor();
      mpColorMenu->setSelectedColor(clrMarker);
   }
}

void AoiToolBarImp::setAoiColor(const QColor& markerColor)
{
   AoiLayerImp* pLayer = dynamic_cast<AoiLayerImp*>(mpAoiLayer);
   if (pLayer != NULL)
   {
      pLayer->setColor(markerColor);
   }
}

void AoiToolBarImp::changeShowLabelState()
{
   if (mpAoiLayer != NULL)
   {
      View* pView = mpAoiLayer->getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

void AoiToolBarImp::setShowPointLabelState(bool showPointLabel)
{
   mpAoiShowPointLabels->setChecked(showPointLabel);

   if (mpAoiLayer != NULL)
   {
      mpAoiLayer->setShowLabels(showPointLabel);

      View* pView = mpAoiLayer->getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

void AoiToolBarImp::selectionObjectChanged()
{
   emit graphicObjectTypeChanged(getSelectionTool());
   emit modeChanged(getSelectionMode());
}

AoiAddModeGrid::AoiAddModeGrid(QWidget* pParent) :
   PixmapGrid(pParent)
{
   setNumRows(1);
   setNumColumns(3);

   setPixmap(0, 0, QPixmap(":/icons/AoiAddAppend"),
      QString::fromStdString(StringUtilities::toXmlString(APPEND_AOI)),
      QString::fromStdString(StringUtilities::toDisplayString(APPEND_AOI)));
   setPixmap(0, 1, QPixmap(":/icons/AoiAddReplace"),
      QString::fromStdString(StringUtilities::toXmlString(REPLACE_AOI)),
      QString::fromStdString(StringUtilities::toDisplayString(REPLACE_AOI)));
   setPixmap(0, 2, QPixmap(":/icons/AoiAddNew"),
      QString::fromStdString(StringUtilities::toXmlString(NEW_AOI)),
      QString::fromStdString(StringUtilities::toDisplayString(NEW_AOI)));

   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(APPEND_AOI)));

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void AoiAddModeGrid::setCurrentValue(AoiAddMode value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

AoiAddMode AoiAddModeGrid::getCurrentValue() const
{
   AoiAddMode retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<AoiAddMode>(curText);
   }
   return retValue;
}

void AoiAddModeGrid::translateChange(const QString& strText)
{
   AoiAddMode curType = StringUtilities::fromXmlString<AoiAddMode>(strText.toStdString());
   emit valueChanged(curType);
}

AoiAddModeButton::AoiAddModeButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   AoiAddModeGrid* pGrid = new AoiAddModeGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(AoiAddMode)), this, SIGNAL(valueChanged(AoiAddMode))));
}

void AoiAddModeButton::setCurrentValue(AoiAddMode value)
{
   AoiAddModeGrid* pGrid = dynamic_cast<AoiAddModeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

AoiAddMode AoiAddModeButton::getCurrentValue() const
{
   AoiAddMode retValue;
   AoiAddModeGrid* pGrid = dynamic_cast<AoiAddModeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}


bool AoiToolBarImp::toXml(XMLWriter* pXml) const
{
   if (!ToolBarImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("showLabels", mpAoiShowLabels->isChecked());
   pXml->addAttr("showPointLabels", mpAoiShowPointLabels->isChecked());
   pXml->addAttr("addMode", mpAddMode->getCurrentValue());
   pXml->addAttr("selectionTool", mpTool->getCurrentValue());
   return true;
}

bool AoiToolBarImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !ToolBarImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   bool bEnabled = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("showLabels"))));
   mpAoiShowLabels->setChecked(bEnabled);
   bEnabled = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("showPointLabels"))));
   mpAoiShowPointLabels->setChecked(bEnabled);
   AoiAddMode aMode = StringUtilities::fromXmlString<AoiAddMode>(A(pElem->getAttribute(X("addMode"))));
   mpAddMode->setCurrentValue(aMode);
   GraphicObjectType oType = StringUtilities::fromXmlString<GraphicObjectType>(
      A(pElem->getAttribute(X("selectionTool"))));
   mpTool->setCurrentValue(oType);

   return true;
}
