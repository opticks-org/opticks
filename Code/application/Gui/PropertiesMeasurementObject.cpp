/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLineWidget.h"
#include "GraphicMeasurementWidget.h"
#include "GraphicObject.h"
#include "GraphicObjectImp.h"
#include "GraphicTextWidget.h"
#include "GraphicUtilities.h"
#include "LabeledSection.h"
#include "MeasurementObjectImp.h"
#include "PropertiesMeasurementObject.h"
#include "Undo.h"
#include "View.h"

using namespace std;

PropertiesMeasurementObject::PropertiesMeasurementObject() :
   LabeledSectionGroup(NULL),
   mpGraphicLayer(NULL)
{
   // Display
   mpDisplayWidget = new GraphicMeasurementWidget();
   mpDisplaySection = new LabeledSection(mpDisplayWidget, "Display");

   // Line
   mpLineWidget = new GraphicLineWidget();
   mpLineSection = new LabeledSection(mpLineWidget, "Line");

   // Text
   mpTextWidget = new GraphicTextWidget();
   mpTextSection = new LabeledSection(mpTextWidget, "Text");

   // Initialization
   addSection(mpDisplaySection);
   addSection(mpLineSection);
   addSection(mpTextSection, 1000);
   addStretch(1);
   setSizeHint(450, 450);

   // Connections
   VERIFYNR(mDistancePrecisionModifier.attachSignal(mpDisplayWidget, SIGNAL(distancePrecisionChanged(int))));
   VERIFYNR(mBearingPrecisionModifier.attachSignal(mpDisplayWidget, SIGNAL(bearingPrecisionChanged(int))));
   VERIFYNR(mEndPointsPrecisionModifier.attachSignal(mpDisplayWidget, SIGNAL(endPointsPrecisionChanged(int))));
   VERIFYNR(mLineStateModifier.attachSignal(mpLineWidget, SIGNAL(stateChanged(bool))));
   VERIFYNR(mLineStyleModifier.attachSignal(mpLineWidget, SIGNAL(styleChanged(LineStyle))));
   VERIFYNR(mLineWidthModifier.attachSignal(mpLineWidget, SIGNAL(widthChanged(unsigned int))));
   VERIFYNR(mLineColorModifier.attachSignal(mpLineWidget, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(mLineScaledModifier.attachSignal(mpLineWidget, SIGNAL(scaledChanged(bool))));
   VERIFYNR(mTextModifier.attachSignal(mpTextWidget, SIGNAL(textChanged(const QString&))));
   VERIFYNR(mAlignmentModifier.attachSignal(mpTextWidget, SIGNAL(alignmentChanged(int))));
   VERIFYNR(mFontModifier.attachSignal(mpTextWidget, SIGNAL(fontChanged(const QFont&))));
   VERIFYNR(mTextColorModifier.attachSignal(mpTextWidget, SIGNAL(colorChanged(const QColor&))));

   VERIFYNR(connect(&mDistancePrecisionModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mBearingPrecisionModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mEndPointsPrecisionModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineStateModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineStyleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineWidthModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineColorModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineScaledModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mTextModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mAlignmentModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mFontModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mTextColorModifier, SIGNAL(modified()), this, SIGNAL(modified())));
}

PropertiesMeasurementObject::~PropertiesMeasurementObject()
{
   // Remove all section widgets, which sets their parent to NULL
   clear();

   // Delete the section widgets
   delete mpDisplaySection;
   delete mpLineSection;
   delete mpTextSection;
}

bool PropertiesMeasurementObject::initialize(SessionItem* pSessionItem)
{
   mpGraphicLayer = NULL;

   GraphicObject* pGraphicObject = dynamic_cast<GraphicObject*>(pSessionItem);
   if (pGraphicObject != NULL)
   {
      list<GraphicObject*> objects;
      objects.push_back(pGraphicObject);

      if (initialize(objects) == true)
      {
         GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pGraphicObject);
         if (pObjectImp != NULL)
         {
            mpGraphicLayer = dynamic_cast<GraphicLayerImp*>(pObjectImp->getLayer());
         }

         return true;
      }
   }

   return false;
}

bool PropertiesMeasurementObject::initialize(const list<GraphicObject*>& graphicObjects)
{
   mObjects = graphicObjects;
   mpGraphicLayer = NULL;

   // Add the required sections

   // Display
   mpDisplayWidget->setDistancePrecision(getDistancePrecision(mObjects));
   mpDisplayWidget->setBearingPrecision(getBearingPrecision(mObjects));
   mpDisplayWidget->setEndPointsPrecision(getEndPointsPrecision(mObjects));

   // Line
   mpLineWidget->setHideLineState(true);
   mpLineWidget->setLineState(GraphicUtilities::getLineState(mObjects));
   mpLineWidget->setLineStyle(GraphicUtilities::getLineStyle(mObjects));
   mpLineWidget->setLineWidth(GraphicUtilities::getLineWidth(mObjects));
   mpLineWidget->setLineColor(GraphicUtilities::getLineColor(mObjects));
   mpLineWidget->setLineScaled(GraphicUtilities::getLineScaled(mObjects));

   // Text
   mpTextWidget->setText(QString::fromStdString(GraphicUtilities::getText(mObjects)));
   mpTextWidget->setAlignment(GraphicUtilities::getTextAlignment(mObjects));
   mpTextWidget->setTextFont(GraphicUtilities::getFont(mObjects));
   mpTextWidget->setColor(COLORTYPE_TO_QCOLOR(GraphicUtilities::getTextColor(mObjects)));

   // Reset the modification flags
   resetModifiers();

   return true;
}

bool PropertiesMeasurementObject::applyChanges()
{
   View* pView = NULL;
   if (mpGraphicLayer != NULL)
   {
      pView = mpGraphicLayer->getView();
   }

   string actionText = "Set " + getName();
   UndoGroup group(pView, actionText);

   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pGraphicObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pGraphicObject != NULL)
      {
         // Display
         MeasurementObjectImp* pMeasObj = dynamic_cast<MeasurementObjectImp*>(pGraphicObject);
         if (pMeasObj != NULL)
         {
            if (mDistancePrecisionModifier.isModified() == true)
            {
               pMeasObj->setDistancePrecision(mpDisplayWidget->getDistancePrecision());
            }

            if (mBearingPrecisionModifier.isModified() == true)
            {
               pMeasObj->setBearingPrecision(mpDisplayWidget->getBearingPrecision());
            }

            if (mEndPointsPrecisionModifier.isModified() == true)
            {
               pMeasObj->setEndPointsPrecision(mpDisplayWidget->getEndPointsPrecision());
            }
         }

         // Line
         if (mLineStateModifier.isModified() == true)
         {
            pGraphicObject->setLineState(mpLineWidget->getLineState());
         }

         if (mLineStyleModifier.isModified() == true)
         {
            pGraphicObject->setLineStyle(mpLineWidget->getLineStyle());
         }

         if (mLineWidthModifier.isModified() == true)
         {
            pGraphicObject->setLineWidth(mpLineWidget->getLineWidth());
         }

         if (mLineColorModifier.isModified() == true)
         {
            pGraphicObject->setLineColor(mpLineWidget->getLineColor());
         }

         if (mLineScaledModifier.isModified() == true)
         {
            pGraphicObject->setLineScaled(mpLineWidget->getLineScaled());
         }

         // Text
         if (mTextModifier.isModified() == true)
         {
            pGraphicObject->setText(mpTextWidget->getText().toStdString());
         }

         if (mAlignmentModifier.isModified() == true)
         {
            int alignment = pGraphicObject->getTextAlignment();
            int newAlignment = alignment &= 0xfffffff0;
            newAlignment |= mpTextWidget->getAlignment();

            pGraphicObject->setTextAlignment(newAlignment);
         }

         if (mFontModifier.isModified() == true)
         {
            pGraphicObject->setFont(mpTextWidget->getTextFont());
         }

         if (mTextColorModifier.isModified() == true)
         {
            pGraphicObject->setTextColor(QCOLOR_TO_COLORTYPE(mpTextWidget->getColor()));
         }
      }
   }

   resetModifiers();

   if (pView != NULL)
   {
      pView->refresh();
   }

   return true;
}

const string& PropertiesMeasurementObject::getName()
{
   static string name = "Measurement Object Properties";
   return name;
}

const string& PropertiesMeasurementObject::getPropertiesName()
{
   static string propertiesName = "Measurement Object";
   return propertiesName;
}

const string& PropertiesMeasurementObject::getDescription()
{
   static string description = "General setting properties of a measurement object";
   return description;
}

const string& PropertiesMeasurementObject::getShortDescription()
{
   static string description = "General setting properties of a measurement object";
   return description;
}

const string& PropertiesMeasurementObject::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesMeasurementObject::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesMeasurementObject::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesMeasurementObject::getDescriptorId()
{
   static string id = "{ED7B4ECF-3AFB-44ad-AE42-3CBEC1673B12}";
   return id;
}

bool PropertiesMeasurementObject::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesMeasurementObject::resetModifiers()
{
   mDistancePrecisionModifier.setModified(false);
   mBearingPrecisionModifier.setModified(false);
   mEndPointsPrecisionModifier.setModified(false);
   mLineStateModifier.setModified(false);
   mLineStyleModifier.setModified(false);
   mLineWidthModifier.setModified(false);
   mLineColorModifier.setModified(false);
   mLineScaledModifier.setModified(false);
   mTextModifier.setModified(false);
   mAlignmentModifier.setModified(false);
   mFontModifier.setModified(false);
   mTextColorModifier.setModified(false);
}

int PropertiesMeasurementObject::getDistancePrecision(const std::list<GraphicObject*>& objects) const
{
   int precision(-1);
   list<GraphicObject*>::const_iterator it;
   for (it = objects.begin(); it != objects.end(); ++it)
   {
      MeasurementObjectImp* pCurrent = dynamic_cast<MeasurementObjectImp*>(*it);
      if (pCurrent != NULL)
      {
         int currentVal = pCurrent->getDistancePrecision();
         if (precision == -1)
         {
            precision = currentVal;
         }
         else
         {
            if (precision != currentVal)
            {
               return -1;
            }
         }
      }
   }
   return precision;
}

int PropertiesMeasurementObject::getBearingPrecision(const std::list<GraphicObject*>& objects) const
{
   int precision(-1);
   list<GraphicObject*>::const_iterator it;
   for (it = objects.begin(); it != objects.end(); ++it)
   {
      MeasurementObjectImp* pCurrent = dynamic_cast<MeasurementObjectImp*>(*it);
      if (pCurrent != NULL)
      {
         int currentVal = pCurrent->getBearingPrecision();
         if (precision == -1)
         {
            precision = currentVal;
         }
         else
         {
            if (precision != currentVal)
            {
               return -1;
            }
         }
      }
   }
   return precision;
}

int PropertiesMeasurementObject::getEndPointsPrecision(const std::list<GraphicObject*>& objects) const
{
   int precision(-1);
   list<GraphicObject*>::const_iterator it;
   for (it = objects.begin(); it != objects.end(); ++it)
   {
      MeasurementObjectImp* pCurrent = dynamic_cast<MeasurementObjectImp*>(*it);
      if (pCurrent != NULL)
      {
         int currentVal = pCurrent->getEndPointsPrecision();
         if (precision == -1)
         {
            precision = currentVal;
         }
         else
         {
            if (precision != currentVal)
            {
               return -1;
            }
         }
      }
   }
   return precision;
}
