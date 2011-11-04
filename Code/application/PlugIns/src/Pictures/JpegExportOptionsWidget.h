/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEGEXPORTOPTIONSWIDGET_H
#define JPEGEXPORTOPTIONSWIDGET_H

#include "LabeledSectionGroup.h"

class ImageResolutionWidget;
class QSlider;

class JpegExportOptionsWidget : public LabeledSectionGroup
{
public:
   JpegExportOptionsWidget();
   ~JpegExportOptionsWidget();
   void setResolution(unsigned int width, unsigned int height);
   void getResolution(unsigned int& width, unsigned int& height);

   unsigned int getCompressionQuality();

private:
   JpegExportOptionsWidget(const JpegExportOptionsWidget& rhs);
   JpegExportOptionsWidget& operator=(const JpegExportOptionsWidget& rhs);
   QSlider* mpQualitySlider;
   ImageResolutionWidget* mpResolutionWidget;
};

#endif
