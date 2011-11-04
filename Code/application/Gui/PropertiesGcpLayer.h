/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESGCPLAYER_H
#define PROPERTIESGCPLAYER_H

#include <QtGui/QSpinBox>

#include "LabeledSectionGroup.h"

#include <string>

class CustomColorButton;
class GcpLayer;
class GcpSymbolButton;
class SessionItem;

class PropertiesGcpLayer : public LabeledSectionGroup
{
public:
   PropertiesGcpLayer();
   ~PropertiesGcpLayer();

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
   PropertiesGcpLayer(const PropertiesGcpLayer& rhs);
   PropertiesGcpLayer& operator=(const PropertiesGcpLayer& rhs);
   GcpLayer* mpGcpLayer;

   // Pixel marker
   GcpSymbolButton* mpSymbolButton;
   QSpinBox* mpSizeSpin;
   CustomColorButton* mpColorButton;
};

#endif
