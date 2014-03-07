/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PNGEXPORTOPTIONSWIDGET_H
#define PNGEXPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"

class ImageResolutionWidget;
class QCheckBox;
class QSpinBox;

class PngExportOptionsWidget : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PngExportOptionsWidget();
   virtual ~PngExportOptionsWidget();

   void setResolution(unsigned int width, unsigned int height);
   void getResolution(unsigned int& width, unsigned int& height) const;
   unsigned int getRowsPerStrip() const;
   bool getBackgroundColorTransparent() const;
   void showBackgroundColorTransparentCheckbox(bool bShow) ;

private:
   PngExportOptionsWidget(const PngExportOptionsWidget& rhs);
   PngExportOptionsWidget& operator=(const PngExportOptionsWidget& rhs);

   QSpinBox* mpRowsPerStrip;
   ImageResolutionWidget* mpResolutionWidget;
   QCheckBox* mpBackgroundColorTransparent;
   LabeledSection* mpBackgroundSection;
};

#endif
