/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>

#include "AppConfig.h"
#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "FileBrowser.h"
#include "GraphicImageWidget.h"
#include "PlotWidget.h"
#include "PlotWindow.h"
#include "PlotWindowImp.h"

#include <string>
#include <vector>
using namespace std;

GraphicImageWidget::GraphicImageWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Image File
   mpFileLabel = new QLabel("Filename:", this);
   mpFileBrowser = new FileBrowser(this);

   // Image widget
   mpWidgetLabel = new QLabel("Plot Window:", this);
   mpWidgetCombo = new QComboBox(this);
   mpWidgetCombo->setEditable(false);

   // Opacity
   mpOpacityLabel = new QLabel("Opacity:", this);
   mpOpacitySpin = new QSpinBox(this);
   mpOpacitySpin->setRange(0, 100);
   mpOpacitySpin->setSuffix("%");

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(mpFileLabel, 0, 0);
   pGrid->addWidget(mpFileBrowser, 0, 1);
   pGrid->addWidget(mpWidgetLabel, 1, 0);
   pGrid->addWidget(mpWidgetCombo, 1, 1);
   pGrid->addWidget(mpOpacityLabel, 2, 0);
   pGrid->addWidget(mpOpacitySpin, 2, 1, Qt::AlignLeft);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(1, 10);

   // Initialization
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      QString browseDir = QString::fromStdString(pSupportFilesPath->getFullPathAndName() + SLASH + "Annotation");
      mpFileBrowser->setBrowseDirectory(browseDir);
   }

   mpFileBrowser->setBrowseFileFilters("Image Files (*.bmp *.jpg);;All Files (*)");

   Service<DesktopServices> pDesktop;

   vector<Window*> plotWindows;
   pDesktop->getWindows(PLOT_WINDOW, plotWindows);
   for (vector<Window*>::iterator iter = plotWindows.begin(); iter != plotWindows.end(); ++iter)
   {
      PlotWindowImp* pWindow = dynamic_cast<PlotWindowImp*>(*iter);
      if (pWindow != NULL)
      {
         if (pWindow->isVisible() == true)
         {
            string name = pWindow->getDisplayName();
            if (name.empty() == true)
            {
               name = pWindow->getName();
            }

            if (name.empty() == false)
            {
               QString windowName = QString::fromStdString(name);
               mpWidgetCombo->addItem(windowName);

               PlotWidget* pPlotWidget = pWindow->getCurrentPlot();
               if (pPlotWidget != NULL)
               {
                  mWidgets.insert(windowName, pPlotWidget->getWidget());
               }
            }
         }
      }
   }

   setImageSource(RAW_DATA);

   // Connections
   VERIFYNR(connect(mpFileBrowser, SIGNAL(filenameChanged(const QString&)), this,
      SIGNAL(imageFileChanged(const QString&))));
   VERIFYNR(connect(mpWidgetCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(notifyImageWidgetChange())));
   VERIFYNR(connect(mpOpacitySpin, SIGNAL(valueChanged(int)), this, SIGNAL(opacityChanged(int))));
}

GraphicImageWidget::~GraphicImageWidget()
{
}

void GraphicImageWidget::setImageSource(ImageSourceType imageSource)
{
   mpFileLabel->setVisible(imageSource == FILE);
   mpFileBrowser->setVisible(imageSource == FILE);
   mpWidgetLabel->setVisible(imageSource == WIDGET);
   mpWidgetCombo->setVisible(imageSource == WIDGET);
}

GraphicImageWidget::ImageSourceType GraphicImageWidget::getImageSource() const
{
   ImageSourceType imageSource = RAW_DATA;
   if (mpFileLabel->isVisible() == true)
   {
      imageSource = FILE;
   }
   else if (mpWidgetLabel->isVisible() == true)
   {
      imageSource = WIDGET;
   }

   return imageSource;
}

QString GraphicImageWidget::getImageFile() const
{
   return mpFileBrowser->getFilename();
}

QWidget* GraphicImageWidget::getImageWidget() const
{
   QWidget* pWidget = NULL;

   QString widgetName = mpWidgetCombo->currentText();
   if (widgetName.isEmpty() == false)
   {
      QMap<QString, QWidget*>::const_iterator iter = mWidgets.find(widgetName);
      if (iter != mWidgets.end())
      {
         pWidget = iter.value();
      }
   }

   return pWidget;
}

int GraphicImageWidget::getOpacity() const
{
   return mpOpacitySpin->value();
}

void GraphicImageWidget::setOpacityVisible(bool bVisible)
{
   mpOpacityLabel->setVisible(bVisible);
   mpOpacitySpin->setVisible(bVisible);
}

bool GraphicImageWidget::isOpacityVisible() const
{
   return mpOpacityLabel->isVisible();
}

void GraphicImageWidget::setImageFile(const QString& filename)
{
   if (filename != getImageFile())
   {
      mpFileBrowser->setFilename(filename);
   }
}

void GraphicImageWidget::setImageWidget(QWidget* pWidget)
{
   int index = -1;
   if (pWidget != NULL)
   {
      QMap<QString, QWidget*>::const_iterator iter;
      for (iter = mWidgets.begin(); iter != mWidgets.end(); ++iter)
      {
         QWidget* pCurrentWidget = iter.value();
         if (pCurrentWidget == pWidget)
         {
            index = mpWidgetCombo->findText(iter.key());
            break;
         }
      }
   }

   mpWidgetCombo->setCurrentIndex(index);
}

void GraphicImageWidget::setOpacity(int opacity)
{
   if (opacity != getOpacity())
   {
      mpOpacitySpin->setValue(opacity);
   }
}

void GraphicImageWidget::notifyImageWidgetChange()
{
   QWidget* pWidget = getImageWidget();
   emit imageWidgetChanged(pWidget);
}
