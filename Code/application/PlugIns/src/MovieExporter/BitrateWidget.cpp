/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "BitrateWidget.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QSlider>

BitrateWidget::BitrateWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Bitrate widgets
   QLabel* pBitrateLabel = new QLabel("Bitrate:", this);

   mpBitrateSlider = new QSlider(Qt::Horizontal, this);
   mpBitrateSlider->setMinimum(1);
   mpBitrateSlider->setMaximum(9800);
   mpBitrateSlider->setPageStep(100);
   mpBitrateSlider->setSingleStep(1);
   mpBitrateSlider->setTracking(true);
   mpBitrateSlider->setTickInterval(1000);
   mpBitrateSlider->setTickPosition(QSlider::TicksBelow);

   mpBitrateLabel = new QLabel(this);
   mpBitrateLabel->setMinimumWidth(QFontMetrics(mpBitrateLabel->font()).width("10000 kbit/s"));

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pBitrateLabel);
   pLayout->addWidget(mpBitrateSlider, 10);
   pLayout->addWidget(mpBitrateLabel);

   // Connections
   VERIFYNR(connect(mpBitrateSlider, SIGNAL(valueChanged(int)), this, SLOT(bitrateChanged(int))));

   // Initialization after connections so that slots are called
   setBitrate(1);
}

BitrateWidget::~BitrateWidget()
{}

void BitrateWidget::setBitrate(int bitrate)
{
   if ((bitrate >= mpBitrateSlider->minimum()) && (bitrate <= mpBitrateSlider->maximum()))
   {
      mpBitrateSlider->setValue(bitrate);
   }
}

int BitrateWidget::getBitrate() const
{
   return mpBitrateSlider->value();
}

void BitrateWidget::bitrateChanged(int value)
{
   // Update the value label
   mpBitrateLabel->setText(QString::number(value) + " kbit/s");
}
