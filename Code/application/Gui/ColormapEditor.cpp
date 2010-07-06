/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColormapEditor.h"
#include "CustomColorButton.h"
#include "HistogramPlotImp.h"
#include "RasterLayer.h"

#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

using namespace std;

ColormapEditor::ColormapEditor(HistogramPlotImp& parent) :
   QDialog(&parent),
   mHistogram(parent),
   mIsApplied(false),
   mNeedDetach(true)
{
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowTitle("Colormap Editor");
   setObjectName("ColorMapEditor");
   resize(QSize(396, 244).expandedTo(minimumSizeHint()));

   mHistogram.attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ColormapEditor::histogramDeleted));

   mpVboxLayout = new QVBoxLayout(this);
   mpVboxLayout->setSpacing(6);
   mpVboxLayout->setMargin(9);
   mpVboxLayout->setObjectName("mpVboxLayout");
   mpHboxLayout = new QHBoxLayout();
   mpHboxLayout->setSpacing(6);
   mpHboxLayout->setMargin(0);
   mpHboxLayout->setObjectName("mpHboxLayout");
   mpPrimariesLabel = new QLabel(this);
   mpPrimariesLabel->setObjectName("mpPrimariesLabel");
   mpPrimariesLabel->setText("Control-Colors:");

   mpHboxLayout->addWidget(mpPrimariesLabel);

   mpPrimariesSpinBox = new QSpinBox(this);
   mpPrimariesSpinBox->setObjectName("mpPrimariesSpinBox");
   mpPrimariesSpinBox->setMaximum(20);
   mpPrimariesSpinBox->setMinimum(2);
   mpPrimariesSpinBox->setValue(2);

   mpHboxLayout->addWidget(mpPrimariesSpinBox);

   mpUniformButton = new QPushButton(this);
   mpUniformButton->setObjectName("mpUniformButton");
   mpUniformButton->setText("Distribute");

   mpHboxLayout->addWidget(mpUniformButton);

   mpSpacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

   mpHboxLayout->addItem(mpSpacerItem);

   mpIndicesLabel = new QLabel(this);
   mpIndicesLabel->setObjectName("mpIndicesLabel");
   mpIndicesLabel->setText("Gradient Steps:");

   mpHboxLayout->addWidget(mpIndicesLabel);

   mpIndicesSpinBox = new QSpinBox(this);
   mpIndicesSpinBox->setObjectName("mpIndicesSpinBox");
   mpIndicesSpinBox->setMaximum(1000);
   mpIndicesSpinBox->setMinimum(2);
   mpIndicesSpinBox->setValue(mInitialIndices);

   mpHboxLayout->addWidget(mpIndicesSpinBox);

   mpVboxLayout->addLayout(mpHboxLayout);

   mpPrimaryView = new QFrame(this);
   mpPrimaryView->setFrameShape(QFrame::StyledPanel);
   mpPrimaryView->setFrameShadow(QFrame::Sunken);

   mpPrimaryLayout = new QGridLayout(mpPrimaryView);
   mpPrimaryLayout->setColumnStretch(0, 1);

   mpVboxLayout->addWidget(mpPrimaryView);

   mpRangeMinSlider = new QSlider(this);
   mpRangeMinSlider->setObjectName("mpRangeMinSlider");
   mpRangeMinSlider->setMaximum(mSliderRange-1);
   mpRangeMinSlider->setSingleStep(mSliderRange/100);
   mpRangeMinSlider->setPageStep(mSliderRange/10);
   mpRangeMinSlider->setOrientation(Qt::Horizontal);

   mpVboxLayout->addWidget(mpRangeMinSlider);

   mpDisplay = new QLabel(this);
   mpDisplay->setObjectName("mpDisplay");
   mpDisplay->setScaledContents(true);
   mpDisplay->setMinimumHeight(20);
   QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
   sizePolicy.setHorizontalStretch(0);
   sizePolicy.setVerticalStretch(0);
   sizePolicy.setHeightForWidth(mpDisplay->sizePolicy().hasHeightForWidth());
   mpDisplay->setSizePolicy(sizePolicy);

   mpVboxLayout->addWidget(mpDisplay);

   mpRangeMaxSlider = new QSlider(this);
   mpRangeMaxSlider->setObjectName("mpRangeMaxSlider");
   mpRangeMaxSlider->setMaximum(mSliderRange-1);
   mpRangeMaxSlider->setSingleStep(mSliderRange/100);
   mpRangeMaxSlider->setPageStep(mSliderRange/10);
   mpRangeMaxSlider->setValue(mSliderRange-1);
   mpRangeMaxSlider->setOrientation(Qt::Horizontal);

   mpVboxLayout->addWidget(mpRangeMaxSlider);

   mpHboxLayout1 = new QHBoxLayout();
   mpHboxLayout1->setSpacing(6);
   mpHboxLayout1->setMargin(0);
   mpHboxLayout1->setObjectName("mpHboxLayout1");
   mpSaveButton = new QPushButton(this);
   mpSaveButton->setObjectName("mpSaveButton");
   mpSaveButton->setText("Save");

   mpHboxLayout1->addWidget(mpSaveButton);

   mpLoadButton = new QPushButton(this);
   mpLoadButton->setObjectName("mpLoadButton");
   mpLoadButton->setText("Load");

   mpHboxLayout1->addWidget(mpLoadButton);

   mpApplyButton = new QPushButton(this);
   mpApplyButton->setObjectName("mpApplyButton");
   mpApplyButton->setText("Apply");

   mpHboxLayout1->addWidget(mpApplyButton);

   mpSpacerItem1 = new QSpacerItem(16, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

   mpHboxLayout1->addItem(mpSpacerItem1);

   mpOkButton = new QPushButton(this);
   mpOkButton->setObjectName("mpOkButton");
   mpOkButton->setText("OK");

   mpHboxLayout1->addWidget(mpOkButton);

   mpCloseButton = new QPushButton(this);
   mpCloseButton->setObjectName("mpCloseButton");
   mpCloseButton->setText("Close");

   mpHboxLayout1->addWidget(mpCloseButton);

   mpVboxLayout->addLayout(mpHboxLayout1);

   connect(mpOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(reject()));
   connect(mpPrimariesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(numPrimariesChanged(int)));
   connect(mpIndicesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateColormap()));
   connect(mpRangeMinSlider, SIGNAL(valueChanged(int)), this, SLOT(rangePositionChanged(int)));
   connect(mpRangeMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(rangePositionChanged(int)));
   connect(mpApplyButton, SIGNAL(clicked()), this, SLOT(applyColormap()));
   connect(mpSaveButton, SIGNAL(clicked()), this, SLOT(saveColormap()));
   connect(mpLoadButton, SIGNAL(clicked()), this, SLOT(loadColormap()));
   connect(mpUniformButton, SIGNAL(clicked()), this, SLOT(distributeUniformly()));

   numPrimariesChanged(2);
}

ColormapEditor::~ColormapEditor()
{
   if (mNeedDetach)
   {
      mHistogram.detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &ColormapEditor::histogramDeleted));
   }
   mHistogram.setAlternateColormap(NULL);
}

void ColormapEditor::numPrimariesChanged(int newCount)
{
   if (newCount == mPrimaries.size())
   {
      return;
   }
   else if (newCount > static_cast<int>(mPrimaries.size()))
   {
      while (newCount > static_cast<int>(mPrimaries.size()))
      {
         QSlider* pSlider = new QSlider(mpPrimaryView);
         pSlider->setOrientation(Qt::Horizontal);
         pSlider->setMaximum(mSliderRange-1);
         pSlider->setSingleStep(mSliderRange/100);
         pSlider->setPageStep(mSliderRange/10);
         if (!mPrimaries.empty())
         {
            pSlider->setValue(mSliderRange-1);
         }
         CustomColorButton* pColorChip = new CustomColorButton(mpPrimaryView);

         QColor newColor = Qt::white;
         if (mPrimaries.empty())
         {
            newColor = Qt::black;
         }
         pColorChip->setColor(newColor);
         mpPrimaryLayout->addWidget(pSlider, mPrimaries.size(), 0);
         mpPrimaryLayout->addWidget(pColorChip, mPrimaries.size(), 1);
         Primary primary = { pColorChip, pSlider, pSlider->value(), newColor };
         mPrimaries.push_back(primary);
         connect(pSlider, SIGNAL(valueChanged(int)), this, SLOT(primaryPositionChanged(int)));
         connect(pColorChip, SIGNAL(colorChanged(const QColor&)), this, SLOT(primaryColorChanged(const QColor&)));
      }
   }
   else // newCount < mPrimaries.size()
   {
      int currentCount = static_cast<int>(mPrimaries.size());
      while (newCount < currentCount)
      {
         delete mPrimaries[currentCount-1].mpButton;
         delete mPrimaries[currentCount-1].mpSlider;
         --currentCount;
      }
      mPrimaries.resize(newCount);
   }
   updateColormap();
}

void ColormapEditor::applyColormap()
{
   if (!mIsApplied)
   {
      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(mHistogram.getLayer());
      VERIFYNRV(pLayer != NULL);
      string name = "Custom";
      if (!mName.empty())
      {
         name = mName;
      }

      try
      {
         // Need a try/catch block because ColorMap constructor throws on failure.
         // Need to create a ColorMap instead of using mColormap because mColormap's
         // name does not match the name to use and ColorMap does not have a setName method.
         pLayer->setColorMap(ColorMap(name, mColormap.getTable()));
      }
      catch (const std::runtime_error&)
      {
         VERIFYNRV_MSG(false, "Invalid colormap creation attempted");
      }

      mIsApplied = true;
   }
}

void ColormapEditor::saveColormap()
{
   QString filename = QFileDialog::getSaveFileName(this, "Save Colormap", QString(), "*.cgr");
   QString mapName = filename;
   if (filename.isNull())
   {
      return;
   }

   if (filename.endsWith(".cgr"))
   {
      mapName = filename.mid(0, filename.lastIndexOf('.'));
   }
   else
   {
      filename.append(".cgr");
   }
   mName = mapName.mid(mapName.lastIndexOf('/') + 1, mapName.length()).toStdString();
   ColorMap cmap(mName, makeGradient());
   cmap.saveToFile(filename.toStdString());
}

void ColormapEditor::loadColormap()
{
   QString filename = QFileDialog::getOpenFileName(this, "Load Colormap", QString(), "*.cgr");
   if (filename.isNull())
   {
      return;
   }

   ColorMap::Gradient gradient;
   try
   {
      ColorMap cmap(filename.toStdString());

      const ColorMap::Gradient* pGradient = cmap.getGradientDefinition();
      if (pGradient == NULL)
      {
         QString message = "The selected file:\n" + filename + "\ncould not be loaded as a color gradient file.";
         QMessageBox::critical(this, "Error Loading Colormap", message, "Ok");
         return;
      }
      gradient = *pGradient;
   }
   catch (std::runtime_error&)
   {
      QString message = "The selected file:\n" + filename + "\ncould not be loaded as a colormap file.";
      QMessageBox::critical(this, "Error Loading Colormap", message, "Ok");
      return;
   }


   this->mpIndicesSpinBox->setValue(gradient.mNumIndices);
   this->mpPrimariesSpinBox->setValue(gradient.mControls.size());

   vector<Primary>::iterator pPrimary;
   for (pPrimary = mPrimaries.begin(); pPrimary != mPrimaries.end(); ++pPrimary)
   {
      disconnect(pPrimary->mpSlider, SIGNAL(valueChanged(int)), this, SLOT(primaryPositionChanged(int)));
   }

   int rangeMin = (gradient.mStartPosition * mSliderRange + (gradient.mNumIndices-1)/2) / (gradient.mNumIndices-1);
   int rangeMax = (gradient.mStopPosition * mSliderRange + (gradient.mNumIndices-1)/2) / (gradient.mNumIndices-1);
   mpRangeMinSlider->setValue(rangeMin);
   mpRangeMaxSlider->setValue(rangeMax);
   int sliderIndexRange = gradient.mStopPosition - gradient.mStartPosition;
   for (unsigned int i = 0; i < gradient.mControls.size(); ++i)
   {
      mPrimaries[i].mColor = QColor(gradient.mControls[i].mColor.mRed,
         gradient.mControls[i].mColor.mGreen, gradient.mControls[i].mColor.mBlue);
      mPrimaries[i].mpButton->setColor(mPrimaries[i].mColor);
      int pos = mSliderRange * (gradient.mControls[i].mPosition - gradient.mStartPosition) / sliderIndexRange;
      mPrimaries[i].mValue = pos;
      mPrimaries[i].mpSlider->setValue(pos);
   }

   for (pPrimary = mPrimaries.begin(); pPrimary != mPrimaries.end(); ++pPrimary)
   {
      connect(pPrimary->mpSlider, SIGNAL(valueChanged(int)), this, SLOT(primaryPositionChanged(int)));
   }

   updateColormap();
}

void ColormapEditor::primaryColorChanged(const QColor& clrNew)
{
   CustomColorButton* pButton = dynamic_cast<CustomColorButton*>(sender());
   VERIFYNRV(pButton != NULL);

   unsigned int i = 0;
   for (i = 0; i < mPrimaries.size(); ++i)
   {
      if (pButton == mPrimaries[i].mpButton)
      {
         break;
      }
   }

   VERIFYNRV(i < mPrimaries.size());

   mPrimaries[i].mColor = clrNew;

   updateColormap();
}

void ColormapEditor::primaryPositionChanged(int newValue)
{
   QSlider* pThisSlider = dynamic_cast<QSlider*>(sender());
   VERIFYNRV(pThisSlider != NULL);
   QSlider* pPrevSlider = NULL;
   QSlider* pNextSlider = NULL;
   unsigned int i = 0;
   for (i = 0; i < mPrimaries.size(); ++i)
   {
      if (mPrimaries[i].mpSlider == pThisSlider)
      {
         if (i != 0)
         {
            pPrevSlider = mPrimaries[i-1].mpSlider;
         }

         if (i != mPrimaries.size()-1)
         {
            pNextSlider = mPrimaries[i+1].mpSlider;
         }

         break;
      }
   }

   VERIFYNRV(i < mPrimaries.size());

   if (pPrevSlider != NULL && pThisSlider->value() < pPrevSlider->value())
   {
      pThisSlider->setValue(pPrevSlider->value());
   }
   if (pNextSlider != NULL && pThisSlider->value() > pNextSlider->value())
   {
      pThisSlider->setValue(pNextSlider->value());
   }

   mPrimaries[i].mValue = pThisSlider->value();

   updateColormap();
}

void ColormapEditor::rangePositionChanged(int newValue)
{
   QSlider* pThisSlider = dynamic_cast<QSlider*>(sender());
   VERIFYNRV(pThisSlider != NULL);

   if (pThisSlider == mpRangeMinSlider)
   {
      if (newValue > mpRangeMaxSlider->value())
      {
         pThisSlider->setValue(mpRangeMaxSlider->value());
      }
   }
   else if (pThisSlider == mpRangeMaxSlider)
   {
      if (newValue < mpRangeMinSlider->value())
      {
         pThisSlider->setValue(mpRangeMinSlider->value());
      }
   }
   else
   {
      // Only the range sliders should be attached to this slot
      VERIFYNRV(false);
   }

   updateColormap();
}

void ColormapEditor::distributeUniformly()
{
   // first, set them all to max, in reverse order - this keeps 
   // primaryPositionChanged from interfering
   vector<Primary>::reverse_iterator pPrimaryRev;
   for (pPrimaryRev = mPrimaries.rbegin(); pPrimaryRev != mPrimaries.rend(); ++pPrimaryRev)
   {
      disconnect(pPrimaryRev->mpSlider, SIGNAL(valueChanged(int)), this, SLOT(primaryPositionChanged(int)));
      pPrimaryRev->mpSlider->setValue(mSliderRange-1);
   }

   int index = 0;
   int maxIndex = mPrimaries.size() - 1;
   int max2 = maxIndex / 2;
   vector<Primary>::iterator pPrimary;
   for (pPrimary = mPrimaries.begin(); pPrimary != mPrimaries.end(); ++pPrimary, ++index)
   {
      pPrimary->mpSlider->setValue(((mSliderRange - 1) * index + max2) / maxIndex);
      connect(pPrimary->mpSlider, SIGNAL(valueChanged(int)), this, SLOT(primaryPositionChanged(int)));
      pPrimary->mValue = pPrimary->mpSlider->value();
   }

   updateColormap();
}

void ColormapEditor::accept()
{
   applyColormap();
   QDialog::accept();
}

void ColormapEditor::updateColormap()
{
   try
   {
      mColormap = ColorMap("Custom", makeGradient());
   }
   catch (const std::runtime_error&)
   {
      VERIFYNRV_MSG(false, "Invalid colormap creation attempted");
   }

   const vector<ColorType>& colorMap = mColormap.getTable();
   int size = colorMap.size();

   // Create the display of the colormap
   QImage image(size, 1, QImage::Format_ARGB32);
   vector<unsigned int> data(size);
   for (int i = 0; i < size; ++i)
   {
      data[i] = qRgb(colorMap[i].mRed, colorMap[i].mGreen, colorMap[i].mBlue);
   }
   memcpy(image.bits(), &data[0], size * sizeof(unsigned int));

   // scale it ourselves non-smoothly to defeat Qt's hard-coded smooth rescaling
   image = image.scaled(max(size, 500), mpDisplay->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
   mpDisplay->setPixmap(QPixmap::fromImage(image));

   // Instruct the histogram to display the prospective colormap
   mHistogram.setAlternateColormap(&mColormap);

   mIsApplied = false;
}

void ColormapEditor::histogramDeleted(Subject& subject, const std::string& signal, const boost::any& v)
{
   mNeedDetach = false;
   delete this;
}

ColorMap::Gradient ColormapEditor::makeGradient() const
{
   int size = mpIndicesSpinBox->value();
   ColorMap::Gradient gradient;
   gradient.mNumIndices = mpIndicesSpinBox->value();
   int rangeMin = mpRangeMinSlider->value();
   int rangeMax = mpRangeMaxSlider->value();
   int sliderRange = rangeMax-rangeMin;
   gradient.mStartPosition = (rangeMin * (size - 1) + mSliderRange / 2) / mSliderRange;
   gradient.mStopPosition = (rangeMax * (size - 1) + mSliderRange / 2) / mSliderRange;
   vector<Primary>::const_iterator pPrimary;
   for (pPrimary = mPrimaries.begin(); pPrimary != mPrimaries.end(); ++pPrimary)
   {
      int index = gradient.mStartPosition + 
         (pPrimary->mValue * (gradient.mStopPosition - gradient.mStartPosition) + sliderRange / 2) / mSliderRange;

      ColorMap::Gradient::Control control;
      control.mColor = ColorType(pPrimary->mColor.red(), pPrimary->mColor.green(), pPrimary->mColor.blue());
      control.mPosition = index;
      gradient.mControls.push_back(control);
   }

   return gradient;
}
