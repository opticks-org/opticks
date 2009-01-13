/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "FrameLabelObjectImp.h"
#include "GraphicArcWidget.h"
#include "GraphicFillWidget.h"
#include "GraphicImageWidget.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLineWidget.h"
#include "GraphicObject.h"
#include "GraphicObjectImp.h"
#include "GraphicObjectWidget.h"
#include "GraphicScaleWidget.h"
#include "GraphicSymbolWidget.h"
#include "GraphicTextWidget.h"
#include "GraphicTriangleWidget.h"
#include "GraphicUtilities.h"
#include "GraphicViewWidget.h"
#include "LabeledSection.h"
#include "PropertiesGraphicObject.h"
#include "TextObjectImp.h"
#include "ViewObjectImp.h"
#include "Undo.h"

using namespace std;

PropertiesGraphicObject::PropertiesGraphicObject() :
   LabeledSectionGroup(NULL),
   mpGraphicLayer(NULL)
{
   // General
   mpGeneralWidget = new GraphicObjectWidget();
   mpGeneralSection = new LabeledSection(mpGeneralWidget, "General");

   // Fill
   mpFillWidget = new GraphicFillWidget();
   mpFillSection = new LabeledSection(mpFillWidget, "Fill");

   // Line
   mpLineWidget = new GraphicLineWidget();
   mpLineSection = new LabeledSection(mpLineWidget, "Line");

   // Arc
   mpArcWidget = new GraphicArcWidget();
   mpArcSection = new LabeledSection(mpArcWidget, "Arc");

   // Image
   mpImageWidget = new GraphicImageWidget();
   mpImageSection = new LabeledSection(mpImageWidget, "Image");

   // Scale
   mpScaleWidget = new GraphicScaleWidget();
   mpScaleSection = new LabeledSection(mpScaleWidget, "Scale");

   // Symbol
   mpSymbolWidget = new GraphicSymbolWidget();
   mpSymbolSection = new LabeledSection(mpSymbolWidget, "Symbol");

   // Text
   mpTextWidget = new GraphicTextWidget();
   mpTextSection = new LabeledSection(mpTextWidget, "Text");

   // Triangle
   mpTriangleWidget = new GraphicTriangleWidget();
   mpTriangleSection = new LabeledSection(mpTriangleWidget, "Triangle");

   // View
   mpViewWidget = new GraphicViewWidget();
   mpViewSection = new LabeledSection(mpViewWidget, "View");

   // Initialization
   setSizeHint(450, 450);

   // Connections
   VERIFYNR(mLowerLeftModifier.attachSignal(mpGeneralWidget, SIGNAL(lowerLeftChanged(const LocationType&))));
   VERIFYNR(mUpperRightModifier.attachSignal(mpGeneralWidget, SIGNAL(upperRightChanged(const LocationType&))));
   VERIFYNR(mRotationModifier.attachSignal(mpGeneralWidget, SIGNAL(rotationChanged(double))));
   VERIFYNR(mFillColorModifier.attachSignal(mpFillWidget, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(mFillStyleModifier.attachSignal(mpFillWidget, SIGNAL(styleChanged(FillStyle))));
   VERIFYNR(mHatchStyleModifier.attachSignal(mpFillWidget, SIGNAL(hatchChanged(SymbolType))));
   VERIFYNR(mLineStateModifier.attachSignal(mpLineWidget, SIGNAL(stateChanged(bool))));
   VERIFYNR(mLineStyleModifier.attachSignal(mpLineWidget, SIGNAL(styleChanged(LineStyle))));
   VERIFYNR(mLineWidthModifier.attachSignal(mpLineWidget, SIGNAL(widthChanged(unsigned int))));
   VERIFYNR(mLineColorModifier.attachSignal(mpLineWidget, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(mLineScaledModifier.attachSignal(mpLineWidget, SIGNAL(scaledChanged(bool))));
   VERIFYNR(mStartAngleModifier.attachSignal(mpArcWidget, SIGNAL(startAngleChanged(double))));
   VERIFYNR(mStopAngleModifier.attachSignal(mpArcWidget, SIGNAL(stopAngleChanged(double))));
   VERIFYNR(mArcRegionModifier.attachSignal(mpArcWidget, SIGNAL(regionChanged(ArcRegion))));
   VERIFYNR(mImageFileModifier.attachSignal(mpImageWidget, SIGNAL(imageFileChanged(const QString&))));
   VERIFYNR(mOpacityModifier.attachSignal(mpImageWidget, SIGNAL(opacityChanged(int))));
   VERIFYNR(mScaleModifier.attachSignal(mpScaleWidget, SIGNAL(scaleChanged(double))));
   VERIFYNR(mSymbolNameModifier.attachSignal(mpSymbolWidget, SIGNAL(nameChanged(const QString&))));
   VERIFYNR(mSymbolSizeModifier.attachSignal(mpSymbolWidget, SIGNAL(sizeChanged(unsigned int))));
   VERIFYNR(mTextModifier.attachSignal(mpTextWidget, SIGNAL(textChanged(const QString&))));
   VERIFYNR(mAlignmentModifier.attachSignal(mpTextWidget, SIGNAL(alignmentChanged(int))));
   VERIFYNR(mFontModifier.attachSignal(mpTextWidget, SIGNAL(fontChanged(const QFont&))));
   VERIFYNR(mTextColorModifier.attachSignal(mpTextWidget, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(mApexModifier.attachSignal(mpTriangleWidget, SIGNAL(apexChanged(int))));
   VERIFYNR(mViewModifier.attachSignal(mpViewWidget, SIGNAL(viewChanged(View*))));

   VERIFYNR(connect(&mLowerLeftModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mUpperRightModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mRotationModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mFillColorModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mFillStyleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mHatchStyleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineStateModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineStyleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineWidthModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineColorModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mLineScaledModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mStartAngleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mStopAngleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mArcRegionModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mImageFileModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mOpacityModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mScaleModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mSymbolNameModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mSymbolSizeModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mTextModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mAlignmentModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mFontModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mTextColorModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mApexModifier, SIGNAL(modified()), this, SIGNAL(modified())));
   VERIFYNR(connect(&mViewModifier, SIGNAL(modified()), this, SIGNAL(modified())));
}

PropertiesGraphicObject::~PropertiesGraphicObject()
{
   // Remove all section widgets, which sets their parent to NULL
   clear();

   // Delete the section widgets
   delete mpGeneralSection;
   delete mpFillSection;
   delete mpLineSection;
   delete mpArcSection;
   delete mpImageSection;
   delete mpScaleSection;
   delete mpSymbolSection;
   delete mpTextSection;
   delete mpTriangleSection;
   delete mpViewSection;
}

bool PropertiesGraphicObject::initialize(SessionItem* pSessionItem)
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

bool PropertiesGraphicObject::initialize(const list<GraphicObject*>& graphicObjects)
{
   mObjects = graphicObjects;
   mpGraphicLayer = NULL;

   // Add the required sections
   bool bFill = false;
   bool bLineOn = false;
   bool bLine = false;
   bool bArc = false;
   bool bImage = false;
   bool bFileImage = false;
   bool bScale = false;
   bool bSymbol = false;
   bool bText = false;
   bool bEditText = false;
   bool bTriangle = false;
   bool bView = false;

   clear();
   for (list<GraphicObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter)
   {
      GraphicObjectImp* pGraphicObject = dynamic_cast<GraphicObjectImp*>(*iter);
      if (pGraphicObject != NULL)
      {
         if ((pGraphicObject->hasProperty("FillColor") == true) ||
            (pGraphicObject->hasProperty("FillStyle") == true) ||
            (pGraphicObject->hasProperty("HatchStyle") == true))
         {
            bFill = true;
         }

         if (pGraphicObject->hasProperty("LineOn") == true)
         {
            bLineOn = true;
         }

         if ((bLineOn == true) || (pGraphicObject->hasProperty("LineColor") == true) ||
            (pGraphicObject->hasProperty("LineScaled") == true) ||
            (pGraphicObject->hasProperty("LineStyle") == true) ||
            (pGraphicObject->hasProperty("LineWidth") == true))
         {
            bLine = true;
         }

         if ((pGraphicObject->hasProperty("ArcRegion") == true) ||
            (pGraphicObject->hasProperty("Wedge") == true))
         {
            bArc = true;
         }

         if (pGraphicObject->hasProperty("Filename") == true)
         {
            bFileImage = true;
            bImage = true;
         }

         if (pGraphicObject->hasProperty("Alpha") == true)
         {
            bImage = true;
         }

         if (pGraphicObject->hasProperty("Scale") == true)
         {
            bScale = true;
         }

         if ((pGraphicObject->hasProperty("GraphicSymbol") == true) ||
            (pGraphicObject->hasProperty("GraphicSymbolSize") == true))
         {
            bSymbol = true;
         }

         if ((pGraphicObject->hasProperty("Font") == true) ||
            (pGraphicObject->hasProperty("TextAlignment") == true) ||
            (pGraphicObject->hasProperty("TextColor") == true) ||
            (pGraphicObject->hasProperty("TextString") == true))
         {
            bText = true;
         }

         if (pGraphicObject->hasProperty("Apex") == true)
         {
            bTriangle = true;
         }

         if (dynamic_cast<ViewObjectImp*>(pGraphicObject) != NULL)
         {
            bView = true;
         }

         // Only allow text to be edited for TextObjects, make text read-only for
         // ScaleBarObject, FrameLabelObject, etc.
         if ((dynamic_cast<TextObjectImp*>(pGraphicObject) != NULL) &&
            (dynamic_cast<FrameLabelObjectImp*>(pGraphicObject) == NULL))
         {
            bEditText = true;
         }
      }
   }

   addSection(mpGeneralSection);
   if (bFill == true)
   {
      addSection(mpFillSection);
   }

   if (bLine == true)
   {
      addSection(mpLineSection);
   }

   if (bArc == true)
   {
      addSection(mpArcSection);
   }

   if (bImage == true)
   {
      addSection(mpImageSection);
   }

   if (bScale == true)
   {
      addSection(mpScaleSection);
   }

   if (bSymbol == true)
   {
      addSection(mpSymbolSection, 1000);
   }

   if (bText == true)
   {
      addSection(mpTextSection, 1000);
   }

   if (bTriangle == true)
   {
      addSection(mpTriangleSection);
   }

   if (bView == true)
   {
      addSection(mpViewSection);
   }

   addStretch(1);

   // General
   mpGeneralWidget->setLowerLeft(GraphicUtilities::getLlCorner(mObjects));
   mpGeneralWidget->setUpperRight(GraphicUtilities::getUrCorner(mObjects));
   mpGeneralWidget->setRotation(GraphicUtilities::getRotation(mObjects));

   // Fill
   mpFillWidget->setFillColor(COLORTYPE_TO_QCOLOR(GraphicUtilities::getFillColor(mObjects)));
   mpFillWidget->setFillStyle(GraphicUtilities::getFillStyle(mObjects));
   mpFillWidget->setHatchStyle(GraphicUtilities::getHatchStyle(mObjects));

   // Line
   mpLineWidget->setHideLineState(!bLineOn);
   mpLineWidget->setLineState(GraphicUtilities::getLineState(mObjects));
   mpLineWidget->setLineStyle(GraphicUtilities::getLineStyle(mObjects));
   mpLineWidget->setLineWidth(GraphicUtilities::getLineWidth(mObjects));
   mpLineWidget->setLineColor(GraphicUtilities::getLineColor(mObjects));
   mpLineWidget->setLineScaled(GraphicUtilities::getLineScaled(mObjects));

   // Arc
   mpArcWidget->setStartAngle(GraphicUtilities::getStartAngle(mObjects));
   mpArcWidget->setStopAngle(GraphicUtilities::getStopAngle(mObjects));
   mpArcWidget->setRegion(GraphicUtilities::getArcRegion(mObjects));

   // Image
   mpImageWidget->setImageSource(GraphicImageWidget::RAW_DATA);
   if (bFileImage == true)
   {
      mpImageWidget->setImageSource(GraphicImageWidget::FILE);
      mpImageWidget->setImageFile(QString::fromStdString(GraphicUtilities::getImageFile(mObjects)));
   }

   mpImageWidget->setOpacity(GraphicUtilities::getAlpha(mObjects) / 255.0 * 100.0);

   // Scale
   mpScaleWidget->setScale(GraphicUtilities::getScale(mObjects));

   // Symbol
   mpSymbolWidget->setSymbolName(QString::fromStdString(GraphicUtilities::getGraphicSymbol(mObjects)));
   mpSymbolWidget->setSymbolSize(GraphicUtilities::getGraphicSymbolSize(mObjects));

   // Text
   mpTextWidget->setTextReadOnly(!bEditText);
   mpTextWidget->setText(QString::fromStdString(GraphicUtilities::getText(mObjects)));
   mpTextWidget->setAlignment(GraphicUtilities::getTextAlignment(mObjects));
   mpTextWidget->setTextFont(GraphicUtilities::getFont(mObjects));
   mpTextWidget->setColor(COLORTYPE_TO_QCOLOR(GraphicUtilities::getTextColor(mObjects)));

   // Triangle
   mpTriangleWidget->setApex(GraphicUtilities::getApex(mObjects) * 100.0);

   // View
   mpViewWidget->setView(GraphicUtilities::getObjectView(mObjects));

   // Reset the modification flags
   resetModifiers();

   return true;
}

bool PropertiesGraphicObject::applyChanges()
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
         // General
         LocationType lowerLeft = pGraphicObject->getLlCorner();
         LocationType upperRight = pGraphicObject->getUrCorner();

         if (mLowerLeftModifier.isModified() == true)
         {
            lowerLeft = mpGeneralWidget->getLowerLeft();
         }

         if (mUpperRightModifier.isModified() == true)
         {
            upperRight = mpGeneralWidget->getUpperRight();
         }

         pGraphicObject->setBoundingBox(lowerLeft, upperRight);

         if (mRotationModifier.isModified() == true)
         {
            pGraphicObject->setRotation(mpGeneralWidget->getRotation());
         }

         // Fill
         if (mFillColorModifier.isModified() == true)
         {
            pGraphicObject->setFillColor(QCOLOR_TO_COLORTYPE(mpFillWidget->getFillColor()));
         }

         if (mFillStyleModifier.isModified() == true)
         {
            pGraphicObject->setFillStyle(mpFillWidget->getFillStyle());
         }

         if (mHatchStyleModifier.isModified() == true)
         {
            pGraphicObject->setHatchStyle(mpFillWidget->getHatchStyle());
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

         // Arc
         if (mStartAngleModifier.isModified() == true)
         {
            pGraphicObject->setStartAngle(mpArcWidget->getStartAngle());
         }

         if (mStopAngleModifier.isModified() == true)
         {
            pGraphicObject->setStopAngle(mpArcWidget->getStopAngle());
         }

         if (mArcRegionModifier.isModified() == true)
         {
            pGraphicObject->setArcRegion(mpArcWidget->getRegion());
         }

         // Image
         if (mImageFileModifier.isModified() == true)
         {
            string imageFile = mpImageWidget->getImageFile().toStdString();
            if (imageFile.empty() == false)
            {
               pGraphicObject->setImageFile(imageFile.c_str());
            }
            else
            {
               pGraphicObject->setImageFile(NULL);
            }
         }

         if (mOpacityModifier.isModified() == true)
         {
            pGraphicObject->setAlpha(mpImageWidget->getOpacity() / 100.0 * 255.0);
         }

         // Scale
         if (mScaleModifier.isModified() == true)
         {
            pGraphicObject->setScale(mpScaleWidget->getScale());
         }

         // Symbol
         if (mSymbolNameModifier.isModified() == true)
         {
            pGraphicObject->setSymbolName(mpSymbolWidget->getSymbolName().toStdString());
         }

         if (mSymbolSizeModifier.isModified() == true)
         {
            pGraphicObject->setSymbolSize(mpSymbolWidget->getSymbolSize());
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

         // Triangle
         if (mApexModifier.isModified() == true)
         {
            pGraphicObject->setApex(mpTriangleWidget->getApex() / 100.0);
         }

         // View
         if (mViewModifier.isModified() == true)
         {
            pGraphicObject->setObjectView(mpViewWidget->getView());
         }
      }
   }

   resetModifiers();
   return true;
}

const string& PropertiesGraphicObject::getName()
{
   static string name = "Graphic Object Properties";
   return name;
}

const string& PropertiesGraphicObject::getPropertiesName()
{
   static string propertiesName = "Graphic Object";
   return propertiesName;
}

const string& PropertiesGraphicObject::getDescription()
{
   static string description = "General setting properties of a graphic object";
   return description;
}

const string& PropertiesGraphicObject::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesGraphicObject::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesGraphicObject::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesGraphicObject::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesGraphicObject::getDescriptorId()
{
   static string id = "{CBD59052-257F-48B0-9A70-A01A638198D6}";
   return id;
}

bool PropertiesGraphicObject::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesGraphicObject::resetModifiers()
{
   mLowerLeftModifier.setModified(false);
   mUpperRightModifier.setModified(false);
   mRotationModifier.setModified(false);
   mFillColorModifier.setModified(false);
   mFillStyleModifier.setModified(false);
   mHatchStyleModifier.setModified(false);
   mLineStateModifier.setModified(false);
   mLineStyleModifier.setModified(false);
   mLineWidthModifier.setModified(false);
   mLineColorModifier.setModified(false);
   mLineScaledModifier.setModified(false);
   mStartAngleModifier.setModified(false);
   mStopAngleModifier.setModified(false);
   mArcRegionModifier.setModified(false);
   mImageFileModifier.setModified(false);
   mOpacityModifier.setModified(false);
   mScaleModifier.setModified(false);
   mSymbolNameModifier.setModified(false);
   mSymbolSizeModifier.setModified(false);
   mTextModifier.setModified(false);
   mAlignmentModifier.setModified(false);
   mFontModifier.setModified(false);
   mTextColorModifier.setModified(false);
   mApexModifier.setModified(false);
   mViewModifier.setModified(false);
}
