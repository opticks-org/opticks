/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSSPATIALDATAVIEW_H
#define OPTIONSSPATIALDATAVIEW_H

#include <QtGui/QWidget>
#include "AppVersion.h"

class PanLimitTypeComboBox;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QRadioButton;

class OptionsSpatialDataView : public QWidget
{
   Q_OBJECT

public:
   OptionsSpatialDataView();
   ~OptionsSpatialDataView();

   void applyChanges();

   static const std::string& getName()
   {
      static std::string var = "Spatial Data View Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Windows/Workspace/Cube";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display spatial data view related options for the application";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display spatial data view related options for the application";
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
      static std::string var = "{3F70A20E-4231-429a-AE6F-0A1F5903396F}";
      return var;
   }

private:
   OptionsSpatialDataView(const OptionsSpatialDataView& rhs);
   OptionsSpatialDataView& operator=(const OptionsSpatialDataView& rhs);
   QSpinBox* mpInsetSizeSpin;
   QSpinBox* mpInsetZoomSpin;
   QComboBox* mpInsetZoom;
   QCheckBox* mpShowCoordinates;
   QCheckBox* mpDisplayCrosshair;
   QSpinBox* mpFastPanSpeedSpin;
   QSpinBox* mpSlowPanSpeedSpin;
   QSpinBox* mpMousePanSensitivitySpin;
   PanLimitTypeComboBox* mpPanLimit;
   QDoubleSpinBox* mpMinZoom;
   QSpinBox* mpMaxZoom;
   QCheckBox* mpGeoCoordTooltip;
   QCheckBox* mpConfirmLayerDelete;
   QCheckBox* mpActiveLayer;
};

#endif
