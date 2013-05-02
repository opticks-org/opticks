/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODISL1BIMPORTOPTIONSWIDGET_H
#define MODISL1BIMPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"
#include "ModisUtilities.h"
#include "RasterDataDescriptor.h"
#include "SafePtr.h"
#include "TypesFile.h"

class RasterConversionTypeComboBox;

class ModisL1bImportOptionsWidget : public LabeledSectionGroup
{
   Q_OBJECT

public:
   ModisL1bImportOptionsWidget(RasterDataDescriptor* pDescriptor);
   virtual ~ModisL1bImportOptionsWidget();

   void setRasterConversion(ModisUtilities::RasterConversionType rasterConversion);
   ModisUtilities::RasterConversionType getRasterConversion() const;

protected:
   EncodingType getOriginalDataType() const;

protected slots:
   void updateDataDescriptor(ModisUtilities::RasterConversionType rasterConversion);

private:
   RasterConversionTypeComboBox* mpRasterConversionCombo;
   SafePtr<RasterDataDescriptor> mpDescriptor;
};

#endif
