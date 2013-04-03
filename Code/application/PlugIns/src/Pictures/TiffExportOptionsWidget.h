/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIFFEXPORTOPTIONSWIDGET_H
#define TIFFEXPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"

class ImageResolutionWidget;
class QCheckBox;
class QSpinBox;

class TiffExportOptionsWidget : public LabeledSectionGroup
{
public:
   TiffExportOptionsWidget();
   virtual ~TiffExportOptionsWidget();

   void setResolution(unsigned int width, unsigned int height);
   void getResolution(unsigned int& width, unsigned int& height) const;
   unsigned int getRowsPerStrip() const;
   bool getPackBitsCompression() const;

private:
   TiffExportOptionsWidget(const TiffExportOptionsWidget& rhs);
   TiffExportOptionsWidget& operator=(const TiffExportOptionsWidget& rhs);

   QCheckBox* mpPackBits;
   QSpinBox* mpRowsPerStrip;
   ImageResolutionWidget* mpResolutionWidget;
};

#endif
