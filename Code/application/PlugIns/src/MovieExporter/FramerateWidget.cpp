/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "FramerateWidget.h"

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QSpinBox>

FramerateWidget::FramerateWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Framerate widgets
   QLabel* pFramerateLabel = new QLabel("Frame Rate:", this);
   mpNumeratorSpin = new QSpinBox(this);
   mpNumeratorSpin->setRange(1, 100000);
   QLabel* pFramerateSlash = new QLabel("frames /", this);
   mpDenominatorSpin = new QSpinBox(this);
   mpDenominatorSpin->setRange(1, 100000);
   QLabel* pFramerateUnits = new QLabel("s", this);
   mpFramerateCombo = new QComboBox(this);
   setFramerates(std::vector<boost::rational<int> >());

   // Layout
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pFramerateLabel, 0, 0);
   pLayout->addWidget(mpNumeratorSpin, 0, 1);
   pLayout->addWidget(pFramerateSlash, 0, 2);
   pLayout->addWidget(mpDenominatorSpin, 0, 3);
   pLayout->addWidget(pFramerateUnits, 0, 4);
   pLayout->addWidget(mpFramerateCombo, 1, 1, 1, 3);
   pLayout->setColumnStretch(5, 10);

   // Connections
   VERIFYNR(connect(mpFramerateCombo, SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(framerateChanged(const QString&))));

   // Initialization after connections so slots are called
   setFramerate(boost::rational<int>(1, 1));
}

FramerateWidget::~FramerateWidget()
{}

void FramerateWidget::setFramerates(const std::vector<boost::rational<int> >& framerates)
{
   mpFramerateCombo->clear();

   for (std::vector<boost::rational<int> >::const_iterator iter = framerates.begin(); iter != framerates.end(); ++iter)
   {
      QString framerate = QString("%1fps - %2/%3")
         .arg(boost::rational_cast<double>(*iter), 0, 'f', 2)
         .arg(iter->numerator())
         .arg(iter->denominator());
      mpFramerateCombo->addItem(framerate);
   }

   if (mpFramerateCombo->count() == 0)
   {
      QStringList defaultFramerates;
      defaultFramerates <<
         "Custom..." <<
         " 1.00fps - 1/1" <<
         "15.00fps - 15/1" <<
         "24.00fps - 24/1" <<
         "29.97fps - 30000/1001" <<
         "30.00fps - 30/1";

      mpFramerateCombo->addItems(defaultFramerates);
   }

   mpFramerateCombo->setCurrentIndex(0);
}

void FramerateWidget::setFramerate(boost::rational<int> framerate)
{
   int num = framerate.numerator();
   int den = framerate.denominator();
   if ((num >= mpNumeratorSpin->minimum()) && (den >= mpDenominatorSpin->minimum()))
   {
      int index = mpFramerateCombo->findText(QString("%1/%2").arg(num).arg(den), Qt::MatchEndsWith);
      if (index != -1)
      {
         mpFramerateCombo->setCurrentIndex(index);
      }
      else
      {
         index = mpFramerateCombo->findText("Custom...");
         if (index != -1)
         {
            mpFramerateCombo->setCurrentIndex(index);
            mpNumeratorSpin->setValue(num);
            mpDenominatorSpin->setValue(den);
         }
      }
   }
}

boost::rational<int> FramerateWidget::getFramerate() const
{
   return boost::rational<int>(mpNumeratorSpin->value(), mpDenominatorSpin->value());
}

void FramerateWidget::framerateChanged(const QString& framerate)
{
   bool customFramerate = (framerate == "Custom...");
   if (customFramerate == false)
   {
      QStringList rationalAndDecimal = framerate.split(" - ");
      if (rationalAndDecimal.size() == 2)
      {
         QStringList split = rationalAndDecimal[1].split("/");
         if (split.size() == 2)
         {
            mpNumeratorSpin->setValue(split[0].toInt());
            mpDenominatorSpin->setValue(split[1].toInt());
         }
      }
   }

   mpNumeratorSpin->setEnabled(customFramerate);
   mpDenominatorSpin->setEnabled(customFramerate);
}
