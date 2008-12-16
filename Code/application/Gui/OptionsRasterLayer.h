/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSRASTERLAYER_H
#define OPTIONSRASTERLAYER_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class ComplexComponentComboBox;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class RegionUnitsComboBox;
class StretchTypeComboBox;

class OptionsRasterLayer : public QWidget
{
   Q_OBJECT

public:
   OptionsRasterLayer();
   ~OptionsRasterLayer();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Raster Layer Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Layers/Raster";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display raster layer related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display raster layer related related options for the application";
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
      static std::string var = "{D0006204-7412-4bd9-8361-7ACA5B11803A}";
      return var;
   }

private:
   QCheckBox* mpUseGpuImage;
   QDoubleSpinBox* mpRedUpperValue;
   RegionUnitsComboBox* mpRedStretch;
   QDoubleSpinBox* mpRedLowerValue;
   QDoubleSpinBox* mpGreenUpperValue;
   RegionUnitsComboBox* mpGreenStretch;
   QDoubleSpinBox* mpGreenLowerValue;
   QDoubleSpinBox* mpBlueUpperValue;
   RegionUnitsComboBox* mpBlueStretch;
   QDoubleSpinBox* mpBlueLowerValue;
   QDoubleSpinBox* mpGrayUpperValue;
   RegionUnitsComboBox* mpGrayStretch;
   QDoubleSpinBox* mpGrayLowerValue;
   QCheckBox* mpFastContrast;
   ComplexComponentComboBox* mpComplexComponent;
   QCheckBox* mpBackgroundTileGen;
   StretchTypeComboBox* mpRgbStretch;
   StretchTypeComboBox* mpGrayscaleStretch;
};

#endif
