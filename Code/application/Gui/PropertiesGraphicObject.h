/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESGRAPHICOBJECT_H
#define PROPERTIESGRAPHICOBJECT_H

#include "LabeledSectionGroup.h"
#include "Modifier.h"

#include <list>
#include <string>

class GraphicArcWidget;
class GraphicFillWidget;
class GraphicImageWidget;
class GraphicLayerImp;
class GraphicLineWidget;
class GraphicObject;
class GraphicObjectWidget;
class GraphicScaleWidget;
class GraphicSymbolWidget;
class GraphicTextWidget;
class GraphicTriangleWidget;
class GraphicUnitsWidget;
class GraphicViewWidget;
class LabeledSection;
class SessionItem;

class PropertiesGraphicObject : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesGraphicObject();
   ~PropertiesGraphicObject();

   bool initialize(SessionItem* pSessionItem);
   bool initialize(const std::list<GraphicObject*>& graphicObjects);
   bool applyChanges();

   static const std::string& getName();
   static const std::string& getPropertiesName();
   static const std::string& getDescription();
   static const std::string& getShortDescription();
   static const std::string& getCreator();
   static const std::string& getCopyright();
   static const std::string& getVersion();
   static const std::string& getDescriptorId();
   static bool isProduction();

signals:
   void modified();

protected:
   void resetModifiers();

private:
   PropertiesGraphicObject(const PropertiesGraphicObject& rhs);
   PropertiesGraphicObject& operator=(const PropertiesGraphicObject& rhs);
   std::list<GraphicObject*> mObjects;
   GraphicLayerImp* mpGraphicLayer;

   // General
   GraphicObjectWidget* mpGeneralWidget;
   LabeledSection* mpGeneralSection;
   Modifier mLowerLeftModifier;
   Modifier mUpperRightModifier;
   Modifier mRotationModifier;

   // Fill
   GraphicFillWidget* mpFillWidget;
   LabeledSection* mpFillSection;
   Modifier mFillColorModifier;
   Modifier mFillStyleModifier;
   Modifier mHatchStyleModifier;

   // Line
   GraphicLineWidget* mpLineWidget;
   LabeledSection* mpLineSection;
   Modifier mLineStateModifier;
   Modifier mLineStyleModifier;
   Modifier mLineWidthModifier;
   Modifier mLineColorModifier;
   Modifier mLineScaledModifier;

   // Arc
   GraphicArcWidget* mpArcWidget;
   LabeledSection* mpArcSection;
   Modifier mStartAngleModifier;
   Modifier mStopAngleModifier;
   Modifier mArcRegionModifier;

   // Image
   GraphicImageWidget* mpImageWidget;
   LabeledSection* mpImageSection;
   Modifier mImageFileModifier;
   Modifier mOpacityModifier;

   // Scale
   GraphicScaleWidget* mpScaleWidget;
   LabeledSection* mpScaleSection;
   Modifier mScaleModifier;

   // Symbol
   GraphicSymbolWidget* mpSymbolWidget;
   LabeledSection* mpSymbolSection;
   Modifier mSymbolNameModifier;
   Modifier mSymbolSizeModifier;

   // Text
   GraphicTextWidget* mpTextWidget;
   LabeledSection* mpTextSection;
   Modifier mTextModifier;
   Modifier mAlignmentModifier;
   Modifier mFontModifier;
   Modifier mTextColorModifier;

   // Triangle
   GraphicTriangleWidget* mpTriangleWidget;
   LabeledSection* mpTriangleSection;
   Modifier mApexModifier;

   // View
   GraphicViewWidget* mpViewWidget;
   LabeledSection* mpViewSection;
   Modifier mViewModifier;

   // Units
   GraphicUnitsWidget* mpUnitsWidget;
   LabeledSection* mpUnitsSection;
   Modifier mUnitsModifier;
};

#endif
