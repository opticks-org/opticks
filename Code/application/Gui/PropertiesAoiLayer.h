/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESAOILAYER_H
#define PROPERTIESAOILAYER_H

#include <QtGui/QCheckBox>

#include "LabeledSectionGroup.h"

#include <string>

class AoiLayer;
class CustomColorButton;
class QLabel;
class SessionItem;
class SymbolTypeButton;

class PropertiesAoiLayer : public LabeledSectionGroup
{
public:
   PropertiesAoiLayer();
   ~PropertiesAoiLayer();

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
   PropertiesAoiLayer(const PropertiesAoiLayer& rhs);
   PropertiesAoiLayer& operator=(const PropertiesAoiLayer& rhs);
   AoiLayer* mpAoiLayer;

   // Pixel marker
   SymbolTypeButton* mpSymbolButton;
   CustomColorButton* mpColorButton;

   // Labels
   QCheckBox* mpShapeCheck;

   // Statistics
   QLabel* mpPixelCount;
   QLabel* mpObjectCount;
};

#endif
