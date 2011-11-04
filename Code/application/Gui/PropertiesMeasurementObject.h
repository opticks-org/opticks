/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESMEASUREMENTOBJECT_H
#define PROPERTIESMEASUREMENTOBJECT_H

#include "LabeledSectionGroup.h"
#include "Modifier.h"

#include <list>
#include <string>

class GraphicLayerImp;
class GraphicLineWidget;
class GraphicMeasurementWidget;
class GraphicObject;
class GraphicTextWidget;
class LabeledSection;
class SessionItem;

class PropertiesMeasurementObject : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesMeasurementObject();
   ~PropertiesMeasurementObject();

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
   int getDistancePrecision(const std::list<GraphicObject*>& objects) const;
   int getBearingPrecision(const std::list<GraphicObject*>& objects) const;
   int getEndPointsPrecision(const std::list<GraphicObject*>& objects) const;

private:
   PropertiesMeasurementObject(const PropertiesMeasurementObject& rhs);
   PropertiesMeasurementObject& operator=(const PropertiesMeasurementObject& rhs);
   std::list<GraphicObject*> mObjects;
   GraphicLayerImp* mpGraphicLayer;

   // Display
   GraphicMeasurementWidget* mpDisplayWidget;
   LabeledSection* mpDisplaySection;
   Modifier mDistancePrecisionModifier;
   Modifier mBearingPrecisionModifier;
   Modifier mEndPointsPrecisionModifier;

   // Line
   GraphicLineWidget* mpLineWidget;
   LabeledSection* mpLineSection;
   Modifier mLineStateModifier;
   Modifier mLineStyleModifier;
   Modifier mLineWidthModifier;
   Modifier mLineColorModifier;
   Modifier mLineScaledModifier;

   // Text
   GraphicTextWidget* mpTextWidget;
   LabeledSection* mpTextSection;
   Modifier mTextModifier;
   Modifier mAlignmentModifier;
   Modifier mFontModifier;
   Modifier mTextColorModifier;
};

#endif
