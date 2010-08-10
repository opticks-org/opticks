/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AdvancedOptionsWidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QSpinBox>

#include <avcodec.h>

AdvancedOptionsWidget::AdvancedOptionsWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Advanced options widgets
   QLabel* pMeMethodLabel = new QLabel("Motion Estimation Method", this);
   mpMeMethod = new QComboBox(this);
   mpMeMethod->addItems(QStringList() << "Zero" << "PHODS" << "Log" << "X1" << "EPZS" << "Full");
   mpMeMethod->setEditable(false);
   QLabel* pGopSizeLabel = new QLabel("GOP Size", this);
   mpGopSize = new QSpinBox(this);
   mpGopSize->setMinimum(0);
   mpGopSize->setMaximum(30);
   QLabel* pQCompressLabel = new QLabel("Q-scale Compression", this);
   mpQCompress = new QDoubleSpinBox(this);
   mpQCompress->setToolTip("Amount of q-scale change between easy & hard scenes");
   mpQCompress->setMinimum(0.0);
   mpQCompress->setMaximum(1.0);
   mpQCompress->setSingleStep(0.1);
   QLabel* pQBlurLabel = new QLabel("Q-scale Smoothing", this);
   mpQBlur = new QDoubleSpinBox(this);
   mpQBlur->setToolTip("Amount of q-scale smoothing over time");
   mpQBlur->setMinimum(0.0);
   mpQBlur->setMaximum(1.0);
   mpQBlur->setSingleStep(0.1);
   QLabel* pQuantizerLabel = new QLabel("Quantizer Range", this);
   mpQMinimum = new QSpinBox(this);
   mpQMinimum->setMinimum(2);
   mpQMinimum->setMaximum(31);
   mpQMaximum = new QSpinBox(this);
   mpQMaximum->setMinimum(2);
   mpQMaximum->setMaximum(31);
   QLabel* pQDiffMaximumLabel = new QLabel("Maximum Quantizer Difference", this);
   mpQDiffMaximum = new QSpinBox(this);
   mpQDiffMaximum->setMinimum(2);
   mpQDiffMaximum->setMaximum(31);
   QLabel* pMaxBFramesLabel = new QLabel("Maximum B-Frames", this);
   mpMaxBFrames = new QSpinBox(this);
   mpMaxBFrames->setMinimum(0);
   mpMaxBFrames->setMaximum(30);
   QLabel* pBQuantLabel = new QLabel("B-Frame Quantizer Factor/Offset", this);
   mpBQuantFactor = new QDoubleSpinBox(this);
   mpBQuantFactor->setToolTip("Q-scale factor between IP and B frames");
   mpBQuantFactor->setMinimum(0.0);
   mpBQuantFactor->setMaximum(31.0);
   mpBQuantFactor->setSingleStep(0.1);
   mpBQuantOffset = new QDoubleSpinBox(this);
   mpBQuantOffset->setToolTip("Q-scale offset between IP and B frames");
   mpBQuantOffset->setMinimum(-1.0);
   mpBQuantOffset->setMaximum(31.0);
   mpBQuantOffset->setSingleStep(0.1);
   QLabel* pDiaSizeLabel = new QLabel("Motion Search Range", this);
   mpDiaSize = new QSpinBox(this);
   mpDiaSize->setToolTip("-1 for fast encode, 2-4 for better quality and compression");
   mpDiaSize->setMinimum(-10);
   mpDiaSize->setMaximum(10);
   QGroupBox* pFlagsGroup = new QGroupBox("Flags", this);
   mpQScale = new QCheckBox("Fixed Quantizer Scale", pFlagsGroup);
   mpQPel = new QCheckBox("Quarter Pixel Motion Estimation", pFlagsGroup);
   mpGmc = new QCheckBox("Global Motion Compensation", pFlagsGroup);
   mpNormalizeAqp = new QCheckBox("Normalize Adaptive Quantization", pFlagsGroup);
   mpTrellis = new QCheckBox("Trellis Quantization", pFlagsGroup);
   mpAcPred = new QCheckBox("MPEG-4 AC Prediction", pFlagsGroup);
   mpCbpRd = new QCheckBox("CBP Rate Distortion", pFlagsGroup);
   mpQpRd = new QCheckBox("QP Rate Distortion", pFlagsGroup);
   mpObmc = new QCheckBox("Overlapped Block Motion Compensation", pFlagsGroup);
   mpClosedGop = new QCheckBox("Close GOP", pFlagsGroup);
   QGridLayout* pFlagsLayout = new QGridLayout(pFlagsGroup);
   pFlagsLayout->addWidget(mpQScale, 0, 0);
   pFlagsLayout->addWidget(mpQPel, 1, 0);
   pFlagsLayout->addWidget(mpGmc, 2, 0);
   pFlagsLayout->addWidget(mpNormalizeAqp, 3, 0);
   pFlagsLayout->addWidget(mpTrellis, 4, 0);
   pFlagsLayout->addWidget(mpAcPred, 0, 1);
   pFlagsLayout->addWidget(mpCbpRd, 1, 1);
   pFlagsLayout->addWidget(mpQpRd, 2, 1);
   pFlagsLayout->addWidget(mpObmc, 3, 1);
   pFlagsLayout->addWidget(mpClosedGop, 4, 1);
   pFlagsGroup->setLayout(pFlagsLayout);

   // Layout
   QGridLayout* pAdvancedLayout = new QGridLayout(this);
   pAdvancedLayout->setMargin(0);
   pAdvancedLayout->setSpacing(5);
   pAdvancedLayout->addWidget(pMeMethodLabel, 0, 0);
   pAdvancedLayout->addWidget(mpMeMethod, 0, 1);
   pAdvancedLayout->addWidget(pGopSizeLabel, 1, 0);
   pAdvancedLayout->addWidget(mpGopSize, 1, 1);
   pAdvancedLayout->addWidget(pQCompressLabel, 2, 0);
   pAdvancedLayout->addWidget(mpQCompress, 2, 1);
   pAdvancedLayout->addWidget(pQBlurLabel, 3, 0);
   pAdvancedLayout->addWidget(mpQBlur, 3, 1);
   pAdvancedLayout->addWidget(pQuantizerLabel, 4, 0);
   pAdvancedLayout->addWidget(mpQMinimum, 4, 1);
   pAdvancedLayout->addWidget(mpQMaximum, 4, 2);
   pAdvancedLayout->addWidget(pQDiffMaximumLabel, 5, 0);
   pAdvancedLayout->addWidget(mpQDiffMaximum, 5, 1);
   pAdvancedLayout->addWidget(pMaxBFramesLabel, 6, 0);
   pAdvancedLayout->addWidget(mpMaxBFrames, 6, 1);
   pAdvancedLayout->addWidget(pBQuantLabel, 7, 0);
   pAdvancedLayout->addWidget(mpBQuantFactor, 7, 1);
   pAdvancedLayout->addWidget(mpBQuantOffset, 7, 2);
   pAdvancedLayout->addWidget(pDiaSizeLabel, 8, 0);
   pAdvancedLayout->addWidget(mpDiaSize, 8, 1);
   pAdvancedLayout->addWidget(pFlagsGroup, 9, 0, 1, 3);
   pAdvancedLayout->setColumnStretch(3, 10);
}

AdvancedOptionsWidget::~AdvancedOptionsWidget()
{}

std::string AdvancedOptionsWidget::getMeMethod() const
{
   return mpMeMethod->currentText().toStdString();
}

void AdvancedOptionsWidget::setMeMethod(const std::string& method)
{
   int idx = mpMeMethod->findText(QString::fromStdString(method));
   if (idx > -1)
   {
      mpMeMethod->setCurrentIndex(idx);
   }
}

int AdvancedOptionsWidget::getGopSize() const
{
   return mpGopSize->value();
}

void AdvancedOptionsWidget::setGopSize(int size)
{
   if (size >= mpGopSize->minimum() && size <= mpGopSize->maximum())
   {
      mpGopSize->setValue(size);
   }
}

float AdvancedOptionsWidget::getQCompress() const
{
   return static_cast<float>(mpQCompress->value());
}

void AdvancedOptionsWidget::setQCompress(float val)
{
   if (val >= mpQCompress->minimum() && val <= mpQCompress->maximum())
   {
      mpQCompress->setValue(val);
   }
}

float AdvancedOptionsWidget::getQBlur() const
{
   return static_cast<float>(mpQBlur->value());
}

void AdvancedOptionsWidget::setQBlur(float val)
{
   if (val >= mpQBlur->minimum() && val <= mpQBlur->maximum())
   {
      mpQBlur->setValue(val);
   }
}

int AdvancedOptionsWidget::getQMinimum() const
{
   return mpQMinimum->value();
}

void AdvancedOptionsWidget::setQMinimum(int val)
{
   if (val >= mpQMinimum->minimum() && val <= mpQMinimum->maximum())
   {
      mpQMinimum->setValue(val);
   }
}

int AdvancedOptionsWidget::getQMaximum() const
{
   return mpQMaximum->value();
}

void AdvancedOptionsWidget::setQMaximum(int val)
{
   if (val >= mpQMaximum->minimum() && val <= mpQMaximum->maximum())
   {
      mpQMaximum->setValue(val);
   }
}

int AdvancedOptionsWidget::getQDiffMaximum() const
{
   return mpQDiffMaximum->value();
}

void AdvancedOptionsWidget::setQDiffMaximum(int val)
{
   if (val >= mpQDiffMaximum->minimum() && val <= mpQDiffMaximum->maximum())
   {
      mpQDiffMaximum->setValue(val);
   }
}

int AdvancedOptionsWidget::getMaxBFrames() const
{
   return mpMaxBFrames->value();
}

void AdvancedOptionsWidget::setMaxBFrames(int val)
{
   if (val >= mpMaxBFrames->minimum() && val <= mpMaxBFrames->maximum())
   {
      mpMaxBFrames->setValue(val);
   }
}

float AdvancedOptionsWidget::getBQuantFactor() const
{
   return static_cast<float>(mpBQuantFactor->value());
}

void AdvancedOptionsWidget::setBQuantFactor(float val)
{
   if (val >= mpBQuantFactor->minimum() && val <= mpBQuantFactor->maximum())
   {
      mpBQuantFactor->setValue(val);
   }
}

float AdvancedOptionsWidget::getBQuantOffset() const
{
   return static_cast<float>(mpBQuantOffset->value());
}

void AdvancedOptionsWidget::setBQuantOffset(float val)
{
   if (val >= mpBQuantOffset->minimum() && val <= mpBQuantOffset->maximum())
   {
      mpBQuantOffset->setValue(val);
   }
}

int AdvancedOptionsWidget::getDiaSize() const
{
   return mpDiaSize->value();
}

void AdvancedOptionsWidget::setDiaSize(int val)
{
   if (val >= mpDiaSize->minimum() && val <= mpDiaSize->maximum())
   {
      mpDiaSize->setValue(val);
   }
}

int AdvancedOptionsWidget::getFlags() const
{
   int flags = 0;
   if (mpQScale->isChecked())
   {
      flags |= CODEC_FLAG_QSCALE;
   }
   if (mpQPel->isChecked())
   {
      flags |= CODEC_FLAG_QPEL;
   }
   if (mpGmc->isChecked())
   {
      flags |= CODEC_FLAG_GMC;
   }
   if (mpNormalizeAqp->isChecked())
   {
      flags |= CODEC_FLAG_NORMALIZE_AQP;
   }
   if (mpTrellis->isChecked())
   {
      flags |= CODEC_FLAG_TRELLIS_QUANT;
   }
   if (mpAcPred->isChecked())
   {
      flags |= CODEC_FLAG_AC_PRED;
   }
   if (mpCbpRd->isChecked())
   {
      flags |= CODEC_FLAG_CBP_RD;
   }
   if (mpQpRd->isChecked())
   {
      flags |= CODEC_FLAG_QP_RD;
   }
   if (mpObmc->isChecked())
   {
      flags |= CODEC_FLAG_OBMC;
   }
   if (mpClosedGop->isChecked())
   {
      flags |= CODEC_FLAG_CLOSED_GOP;
   }

   return flags;
}

void AdvancedOptionsWidget::setFlags(int val)
{
   mpQScale->setChecked(val & CODEC_FLAG_QSCALE);
   mpQPel->setChecked(val & CODEC_FLAG_QPEL);
   mpGmc->setChecked(val & CODEC_FLAG_GMC);
   mpNormalizeAqp->setChecked(val & CODEC_FLAG_NORMALIZE_AQP);
   mpTrellis->setChecked(val & CODEC_FLAG_TRELLIS_QUANT);
   mpAcPred->setChecked(val & CODEC_FLAG_AC_PRED);
   mpCbpRd->setChecked(val & CODEC_FLAG_CBP_RD);
   mpQpRd->setChecked(val & CODEC_FLAG_QP_RD);
   mpObmc->setChecked(val & CODEC_FLAG_OBMC);
   mpClosedGop->setChecked(val & CODEC_FLAG_CLOSED_GOP);
}
