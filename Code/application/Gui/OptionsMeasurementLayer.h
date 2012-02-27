/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSMEASUREMENTLAYER_H
#define OPTIONSMEASUREMENTLAYER_H

#include <QtGui/QWidget>

#include "AppVersion.h"

class CustomColorButton;
class DistanceUnitsButton;
class FontSizeComboBox;
class LineStyleComboBox;
class LineWidthComboBox;
class QCheckBox;
class QFontComboBox;
class QSpinBox;

class OptionsMeasurementLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsMeasurementLayer();
   virtual ~OptionsMeasurementLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Measurement Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Measurement";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = getShortDescription();
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display measurement layer related options for the application";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT_MSG;
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = APP_VERSION_NUMBER;
      return var;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{773531CC-2AC4-49a7-9C6E-62818651861E}";
      return var;
   }

private:
   OptionsMeasurementLayer(const OptionsMeasurementLayer& rhs);
   OptionsMeasurementLayer& operator=(const OptionsMeasurementLayer& rhs);

   LineWidthComboBox* mpLineWidth;
   LineStyleComboBox* mpLineStyle;
   CustomColorButton* mpLineColor;
   CustomColorButton* mpTextColor;
   FontSizeComboBox* mpTextFontSize;
   QFontComboBox* mpTextFont;
   QCheckBox* mpBoldCheck;
   QCheckBox* mpItalicsCheck;
   QCheckBox* mpUnderlineCheck;
   QCheckBox* mpDisplayBearing;
   QCheckBox* mpDisplayDistance;
   QCheckBox* mpDisplayEndPoints;
   QSpinBox* mpBearingPrecision;
   QSpinBox* mpDistancePrecision;
   QSpinBox* mpEndPointsPrecision;
   DistanceUnitsButton* mpDistanceUnits;
};

#endif
