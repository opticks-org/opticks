/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESLATLONLAYER_H
#define PROPERTIESLATLONLAYER_H

#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>

#include "LabeledSectionGroup.h"

#include <string>

class CustomColorButton;
class DmsFormatTypeComboBox;
class LatLonLayer;
class LatLonLineEdit;
class LatLonStyleComboBox;
class LineWidthComboBox;
class SessionItem;

class PropertiesLatLonLayer : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesLatLonLayer();
   ~PropertiesLatLonLayer();

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

protected slots:
   void autoTickSpacingEnabled(bool bEnable);

private:
   PropertiesLatLonLayer(const PropertiesLatLonLayer& rhs);
   PropertiesLatLonLayer& operator=(const PropertiesLatLonLayer& rhs);
   LatLonLayer* mpLatLonLayer;

   // Coordinates
   QRadioButton* mpLatLonRadio;
   DmsFormatTypeComboBox* mpLatLonFormatCombo;
   QRadioButton* mpUtmRadio;
   QRadioButton* mpMgrsRadio;
   QFontComboBox* mpFontCombo;
   QComboBox* mpFontSizeCombo;

   // Gridlines
   LatLonStyleComboBox* mpStyleCombo;
   LineWidthComboBox* mpWidthCombo;
   CustomColorButton* mpColorButton;
   QRadioButton* mpAutomaticRadio;
   QRadioButton* mpCustomRadio;
   QLabel* mpLatitudeLabel;
   LatLonLineEdit* mpLatitudeEdit;
   QLabel* mpLongitudeLabel;
   LatLonLineEdit* mpLongitudeEdit;
};

#endif
