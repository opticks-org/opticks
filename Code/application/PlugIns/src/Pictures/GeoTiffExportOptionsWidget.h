/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOTIFFEXPORTOPTIONSWIDGET_H
#define GEOTIFFEXPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"
#include "OptionsTiffExporter.h"

class QCheckBox;
class QRadioButton;
class QSpinBox;

class GeoTiffExportOptionsWidget : public LabeledSectionGroup
{
   Q_OBJECT

public:
   GeoTiffExportOptionsWidget();
   virtual ~GeoTiffExportOptionsWidget();

   OptionsTiffExporter::TransformationMethod getTransformationMethod() const;
   int getRowsPerStrip() const;
   bool getPackBitsCompression() const;

private:
   QRadioButton* mpTiePointRadio;
   QRadioButton* mpMatrixRadio;
   QSpinBox* mpRowsPerStrip;
   QCheckBox* mpPackBits;
};

#endif
