/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>

#include "AppVerify.h"
#include "DesktopServices.h"
#include "LabeledSection.h"
#include "LocationType.h"
#include "MouseMode.h"
#include "MouseModeTestGui.h"
#include "SpatialDataView.h"
#include "ToolBar.h"

using namespace std;

MouseModeTestGui::MouseModeTestGui(SpatialDataView* pView, QWidget* pParent) :
   QDialog(pParent),
   mpView(pView, SIGNAL_NAME(View, MouseModeChanged), Slot(this, &MouseModeTestGui::updateMouseMode)),
   mpCustomMouseMode(NULL),
   mpCustomAction(NULL)
{
   // Mouse modes
   LabeledSection* pMouseModeSection = new LabeledSection("Mouse Mode", this);

   QWidget* pMouseModeWidget = new QWidget(pMouseModeSection);
   mpNoModeRadio = new QRadioButton("None", pMouseModeWidget);
   mpLayerRadio = new QRadioButton("Layer", pMouseModeWidget);
   mpMeasurementRadio = new QRadioButton("Measurement", pMouseModeWidget);
   mpPanRadio = new QRadioButton("Pan", pMouseModeWidget);
   mpRotateRadio = new QRadioButton("Rotate", pMouseModeWidget);
   mpZoomInRadio = new QRadioButton("Zoom In", pMouseModeWidget);
   mpZoomOutRadio = new QRadioButton("Zoom Out", pMouseModeWidget);
   mpZoomBoxRadio = new QRadioButton("Zoom Rectangle", pMouseModeWidget);
   mpCustomRadio = new QRadioButton("Custom", pMouseModeWidget);
   QCheckBox* pCustomModeCheck = new QCheckBox("Enable Custom Mode", pMouseModeWidget);

   QButtonGroup* pMouseModeGroup = new QButtonGroup(this);
   pMouseModeGroup->setExclusive(true);
   pMouseModeGroup->addButton(mpNoModeRadio);
   pMouseModeGroup->addButton(mpLayerRadio);
   pMouseModeGroup->addButton(mpMeasurementRadio);
   pMouseModeGroup->addButton(mpPanRadio);
   pMouseModeGroup->addButton(mpRotateRadio);
   pMouseModeGroup->addButton(mpZoomInRadio);
   pMouseModeGroup->addButton(mpZoomOutRadio);
   pMouseModeGroup->addButton(mpZoomBoxRadio);
   pMouseModeGroup->addButton(mpCustomRadio);

   pMouseModeSection->setSectionWidget(pMouseModeWidget);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);
   pButtonBox->setStandardButtons(QDialogButtonBox::Close);
   pButtonBox->setOrientation(Qt::Horizontal);

   // Layout
   QGridLayout* pMouseModeGrid = new QGridLayout(pMouseModeWidget);
   pMouseModeGrid->setMargin(0);
   pMouseModeGrid->setSpacing(5);
   pMouseModeGrid->addWidget(mpNoModeRadio, 0, 0);
   pMouseModeGrid->addWidget(mpLayerRadio, 1, 0);
   pMouseModeGrid->addWidget(mpMeasurementRadio, 2, 0);
   pMouseModeGrid->addWidget(mpPanRadio, 3, 0);
   pMouseModeGrid->addWidget(mpRotateRadio, 4, 0);
   pMouseModeGrid->addWidget(mpZoomInRadio, 5, 0);
   pMouseModeGrid->addWidget(mpZoomOutRadio, 6, 0);
   pMouseModeGrid->addWidget(mpZoomBoxRadio, 7, 0);
   pMouseModeGrid->addWidget(mpCustomRadio, 8, 0);
   pMouseModeGrid->addWidget(pCustomModeCheck, 0, 1);
   pMouseModeGrid->setRowStretch(0, 10);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pMouseModeSection, 10);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pButtonBox);

   // Initialization
   setWindowTitle("Mouse Mode Test");
   setModal(false);
   enableCustomMouseMode(false);
   resize(400, 300);

   if (mpView.get() != NULL)
   {
      const MouseMode* pMouseMode = mpView->getCurrentMouseMode();
      setMouseMode(pMouseMode);
   }

   // Connections
   VERIFYNR(connect(pMouseModeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this,
      SLOT(setMouseMode(QAbstractButton*))));
   VERIFYNR(connect(pCustomModeCheck, SIGNAL(toggled(bool)), this, SLOT(enableCustomMouseMode(bool))));
   VERIFYNR(connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject())));
}

MouseModeTestGui::~MouseModeTestGui()
{
   // Remove the custom mouse mode from the view
   enableCustomMouseMode(false);

   // Delete the custom mouse mode
   if (mpCustomMouseMode != NULL)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->deleteMouseMode(mpCustomMouseMode);
   }
}

bool MouseModeTestGui::eventFilter(QObject* pObject, QEvent* pEvent)
{
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if ((pEvent->type() == QEvent::MouseButtonPress) || (pEvent->type() == QEvent::MouseButtonRelease))
      {
         QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);

         if (mpView.get() != NULL)
         {
            QWidget* pViewWidget = mpView->getWidget();
            if (pObject == pViewWidget)
            {
               MouseMode* pMouseMode = mpView->getCurrentMouseMode();
               if ((pMouseMode != NULL) && (pMouseMode == mpCustomMouseMode))
               {
                  const QPoint& ptMouse = pMouseEvent->pos();

                  LocationType pixelCoord;
                  mpView->translateScreenToWorld(ptMouse.x(), ptMouse.y(), pixelCoord.mX, pixelCoord.mY);

                  double dMinX = 0.0;
                  double dMinY = 0.0;
                  double dMaxX = 0.0;
                  double dMaxY = 0.0;
                  mpView->getExtents(dMinX, dMinY, dMaxX, dMaxY);

                  if ((pixelCoord.mX >= dMinX) && (pixelCoord.mX <= dMaxX) &&
                     (pixelCoord.mY >= dMinY) && (pixelCoord.mY <= dMaxY))
                  {
                     mpView->flipHorizontal();
                     mpView->refresh();
                  }
               }
            }
         }
      }
   }

   return QDialog::eventFilter(pObject, pEvent);
}

void MouseModeTestGui::updateMouseMode(Subject& subject, const string& signal, const boost::any& value)
{
   const MouseMode* pMouseMode = boost::any_cast<const MouseMode*>(value);
   setMouseMode(pMouseMode);
}

void MouseModeTestGui::setMouseMode(QAbstractButton* pButton)
{
   if (mpView.get() == NULL)
   {
      return;
   }

   if (pButton == mpNoModeRadio)
   {
      mpView->setMouseMode(NULL);
   }
   else if (pButton == mpLayerRadio)
   {
      mpView->setMouseMode("LayerMode");
   }
   else if (pButton == mpMeasurementRadio)
   {
      mpView->setMouseMode("MeasurementMode");
   }
   else if (pButton == mpPanRadio)
   {
      mpView->setMouseMode("PanMode");
   }
   else if (pButton == mpRotateRadio)
   {
      mpView->setMouseMode("RotateMode");
   }
   else if (pButton == mpZoomInRadio)
   {
      mpView->setMouseMode("ZoomInMode");
   }
   else if (pButton == mpZoomOutRadio)
   {
      mpView->setMouseMode("ZoomOutMode");
   }
   else if (pButton == mpZoomBoxRadio)
   {
      mpView->setMouseMode("ZoomBoxMode");
   }
   else if (pButton == mpCustomRadio)
   {
      mpView->setMouseMode(mpCustomMouseMode);
   }
}

void MouseModeTestGui::setMouseMode(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      mpNoModeRadio->setChecked(true);
   }
   else if (pMouseMode == mpCustomMouseMode)
   {
      mpCustomRadio->setChecked(true);
   }
   else
   {
      string modeName;
      pMouseMode->getName(modeName);

      if (modeName == "LayerMode")
      {
         mpLayerRadio->setChecked(true);
      }
      else if (modeName == "MeasurementMode")
      {
         mpMeasurementRadio->setChecked(true);
      }
      else if (modeName == "PanMode")
      {
         mpPanRadio->setChecked(true);
      }
      else if (modeName == "RotateMode")
      {
         mpRotateRadio->setChecked(true);
      }
      else if (modeName == "ZoomInMode")
      {
         mpZoomInRadio->setChecked(true);
      }
      else if (modeName == "ZoomOutMode")
      {
         mpZoomOutRadio->setChecked(true);
      }
      else if (modeName == "ZoomBoxMode")
      {
         mpZoomBoxRadio->setChecked(true);
      }
   }
}

void MouseModeTestGui::enableCustomMouseMode(bool bEnable)
{
   if (mpView.get() == NULL)
   {
      return;
   }

   Service<DesktopServices> pDesktop;
   if (bEnable == true)
   {
      // Create the custom action
      if (mpCustomAction == NULL)
      {
         static const char* const pMouseModeIcon[] =
         {
            "16 16 2 1",
            "# c #000000",
            ". c #c0c0c0",
            ".##...#.........",
            ".##...#.........",
            ".#.#..#.........",
            ".#..#.#.........",
            ".#...##.........",
            "....######......",
            "....#...........",
            "....###.........",
            "....#...........",
            "....######......",
            ".......#.......#",
            ".......#.......#",
            "........#..#..#.",
            "........#.#.#.#.",
            ".........#...#..",
            "................"
         };

         QPixmap modePixmap(pMouseModeIcon);
         modePixmap.setMask(modePixmap.createHeuristicMask());
         QIcon modeIcon(modePixmap);

         mpCustomAction = new QAction(modeIcon, "Custom Mouse Mode", this);
      }

      // Create the custom mouse mode
      if (mpCustomMouseMode == NULL)
      {
         mpCustomMouseMode = pDesktop->createMouseMode("CustomMouseMode", NULL, NULL, -1, -1, mpCustomAction);
         VERIFYNRV(mpCustomMouseMode != NULL);
      }

      // Add a toolbar button
      ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Tests", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->addButton(mpCustomAction);
      }

      // Add the mouse mode to the view
      mpView->addMouseMode(mpCustomMouseMode);
      mpView->getWidget()->installEventFilter(this);

      // Enable the radio button
      mpCustomRadio->setEnabled(true);
   }
   else
   {
      // Disable the radio button
      mpCustomRadio->setEnabled(false);
      if (mpCustomRadio->isChecked() == true)
      {
         mpNoModeRadio->setChecked(true);
      }

      // Remove the mouse mode from the view
      mpView->setMouseMode(NULL);
      mpView->removeMouseMode(mpCustomMouseMode);
      mpView->getWidget()->removeEventFilter(this);

      // Remove the toolbar button
      ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Tests", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->removeItem(mpCustomAction);
      }
   }
}
