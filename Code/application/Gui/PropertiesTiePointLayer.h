/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESTIEPOINTLAYER_H
#define PROPERTIESTIEPOINTLAYER_H

#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>

#include "LabeledSectionGroup.h"

#include <string>

class TiePointLayer;
class CustomColorButton;
class SessionItem;

class PropertiesTiePointLayer : public LabeledSectionGroup
{
public:
   PropertiesTiePointLayer();
   ~PropertiesTiePointLayer();

   bool initialize(SessionItem* pSessionItem);
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

private:
   PropertiesTiePointLayer(const PropertiesTiePointLayer& rhs);
   PropertiesTiePointLayer& operator=(const PropertiesTiePointLayer& rhs);
   TiePointLayer* mpTiePointLayer;

   // Pixel marker
   QSpinBox* mpSizeSpin;
   CustomColorButton* mpColorButton;

   // Labels
   QCheckBox* mpLabelsCheck;
};

#endif
