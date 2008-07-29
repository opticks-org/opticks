/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESPLOTVIEW_H
#define PROPERTIESPLOTVIEW_H

#include "LabeledSectionGroup.h"

#include <string>

class CustomColorButton;
class LineStyleComboBox;
class LineWidthComboBox;
class PlotView;
class SessionItem;

class PropertiesPlotView : public LabeledSectionGroup
{
public:
   PropertiesPlotView();
   ~PropertiesPlotView();

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
   PlotView* mpPlotView;

   // Gridlines
   LineStyleComboBox* mpStyleCombo;
   LineWidthComboBox* mpWidthCombo;
   CustomColorButton* mpColorButton;
};

#endif
