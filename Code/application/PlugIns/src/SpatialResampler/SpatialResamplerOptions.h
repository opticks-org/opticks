/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALRESAMPLEROPTIONS_H
#define SPATIALRESAMPLEROPTIONS_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

class InterpolationComboBox;
class QDoubleSpinBox;

class SpatialResamplerOptions : public LabeledSectionGroup
{
   Q_OBJECT

public:
   SETTING(XScaleFactor, SpatialResamplerConfig, double, 2.0);
   SETTING(YScaleFactor, SpatialResamplerConfig, double, 2.0);
   SETTING(InterpolationMethod, SpatialResamplerConfig, InterpolationType, INTERP_BICUBIC);

   SpatialResamplerOptions();
   virtual ~SpatialResamplerOptions();

   static const std::string& getName()
   {
      static std::string var = "Spatial Resampler Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Spatial Resampler";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Configuration options for the spatial resampler";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Configuration options for the spatial resampler";
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
      static std::string var = "{90C68CD1-203C-44E2-928E-CFD38BD3816D}";
      return var;
   }

   void applyChanges();

private:
   QDoubleSpinBox* mpXScaleFactorSpinBox;
   QDoubleSpinBox* mpYScaleFactorSpinBox;
   InterpolationComboBox* mpInterpMethodComboBox;
};

#endif