/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESPOINTCLOUDVIEW_H
#define PROPERTIESPOINTCLOUDVIEW_H

#include "DimensionDescriptor.h"
#include "LabeledSectionGroup.h"
#include "Modifier.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class ComplexComponentComboBox;
class CustomColorButton;
class PointCloudView;
class QAction;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QMenu;
class QListWidget;
class QPushButton;
class QSpinBox;
class RasterElement;
class RasterLayer;
class RegionUnitsComboBox;
class SessionItem;
class StretchTypeComboBox;

class PropertiesPointCloudView : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesPointCloudView();
   ~PropertiesPointCloudView();

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
   bool mInitializing;
   PointCloudView* mpPointCloud;

   QComboBox* mpColorizeBy;
   Modifier mColorizeByModifier;

   QSpinBox* mpDecimationSpin;
   Modifier mDecimationModifier;

   QDoubleSpinBox* mpPointSizeSpin;
   Modifier mPointSizeModifier;

   QDoubleSpinBox* mpZExaggerationSpin;
   Modifier mZExaggerationModifier;

   QDoubleSpinBox* mpLowerStretchSpin;
   QDoubleSpinBox* mpUpperStretchSpin;
   Modifier mStretchModifier;

   CustomColorButton* mpLowerColorButton;
   CustomColorButton* mpUpperColorButton;
   Modifier mColorModifier;

   QCheckBox* mpUseColorMap;
   Modifier mUseColorMapModifier;

   QListWidget* mpColorMapList;
   std::map<std::string, std::string> mPreloadedColorMaps;
};

#endif
