/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "AppVerify.h"
#include "GraphicObjectWidget.h"

#include <limits>
using namespace std;

GraphicObjectWidget::GraphicObjectWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Lower left
   QLabel* pLowerLeftXLabel = new QLabel("Lower Left X:", this);
   mpLowerLeftXSpin = new QDoubleSpinBox(this);
   mpLowerLeftXSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pLowerLeftYLabel = new QLabel("Lower Left Y:", this);
   mpLowerLeftYSpin = new QDoubleSpinBox(this);
   mpLowerLeftYSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   // Upper right
   QLabel* pUpperRightXLabel = new QLabel("Upper Right X:", this);
   mpUpperRightXSpin = new QDoubleSpinBox(this);
   mpUpperRightXSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   QLabel* pUpperRightYLabel = new QLabel("Upper Right Y:", this);
   mpUpperRightYSpin = new QDoubleSpinBox(this);
   mpUpperRightYSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());

   // Rotation
   QLabel* pRotationLabel = new QLabel("Rotation:", this);
   mpRotationSpin = new QDoubleSpinBox(this);
   mpRotationSpin->setRange(-360.0, 360.0);
   mpRotationSpin->setDecimals(1);
   mpRotationSpin->setSuffix(" Degrees");

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pLowerLeftXLabel, 0, 0);
   pGrid->addWidget(mpLowerLeftXSpin, 0, 1);
   pGrid->addWidget(pLowerLeftYLabel, 1, 0);
   pGrid->addWidget(mpLowerLeftYSpin, 1, 1);
   pGrid->addWidget(pUpperRightXLabel, 2, 0);
   pGrid->addWidget(mpUpperRightXSpin, 2, 1);
   pGrid->addWidget(pUpperRightYLabel, 3, 0);
   pGrid->addWidget(mpUpperRightYSpin, 3, 1);
   pGrid->addWidget(pRotationLabel, 4, 0);
   pGrid->addWidget(mpRotationSpin, 4, 1);
   pGrid->setRowStretch(5, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpLowerLeftXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
   VERIFYNR(connect(mpLowerLeftYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
   VERIFYNR(connect(mpUpperRightXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
   VERIFYNR(connect(mpUpperRightYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
   VERIFYNR(connect(mpRotationSpin, SIGNAL(valueChanged(double)), this, SIGNAL(rotationChanged(double))));
}

GraphicObjectWidget::~GraphicObjectWidget()
{
}

LocationType GraphicObjectWidget::getLowerLeft() const
{
   return LocationType(mpLowerLeftXSpin->value(), mpLowerLeftYSpin->value());
}

LocationType GraphicObjectWidget::getUpperRight() const
{
   return LocationType(mpUpperRightXSpin->value(), mpUpperRightYSpin->value());
}

double GraphicObjectWidget::getRotation() const
{
   return mpRotationSpin->value();
}

void GraphicObjectWidget::setLowerLeft(const LocationType& lowerLeft)
{
   if (lowerLeft != getLowerLeft())
   {
      VERIFYNR(disconnect(mpLowerLeftXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
      VERIFYNR(disconnect(mpLowerLeftYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
      mpLowerLeftXSpin->setValue(lowerLeft.mX);
      mpLowerLeftYSpin->setValue(lowerLeft.mY);
      VERIFYNR(connect(mpLowerLeftXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
      VERIFYNR(connect(mpLowerLeftYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyLowerLeftChange())));
      emit lowerLeftChanged(lowerLeft);
   }
}

void GraphicObjectWidget::setUpperRight(const LocationType& upperRight)
{
   if (upperRight != getUpperRight())
   {
      VERIFYNR(disconnect(mpUpperRightXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
      VERIFYNR(disconnect(mpUpperRightYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
      mpUpperRightXSpin->setValue(upperRight.mX);
      mpUpperRightYSpin->setValue(upperRight.mY);
      VERIFYNR(connect(mpUpperRightXSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
      VERIFYNR(connect(mpUpperRightYSpin, SIGNAL(valueChanged(double)), this, SLOT(notifyUpperRightChange())));
      emit upperRightChanged(upperRight);
   }
}

void GraphicObjectWidget::setRotation(double rotation)
{
   if (rotation != getRotation())
   {
      mpRotationSpin->setValue(rotation);
   }
}

void GraphicObjectWidget::notifyLowerLeftChange()
{
   LocationType lowerLeft = getLowerLeft();
   emit lowerLeftChanged(lowerLeft);
}

void GraphicObjectWidget::notifyUpperRightChange()
{
   LocationType upperRight = getUpperRight();
   emit upperRightChanged(upperRight);
}
