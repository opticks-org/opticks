/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "PicturesPlotWidgetExporter.h"
#include "PlotWidget.h"
#include "PlugInArgList.h"
#include "Progress.h"

#include <QtGui/QImage>

PicturesPlotWidgetExporter::PicturesPlotWidgetExporter(PicturesDetails *pDetails) : PicturesExporter(pDetails)
{
   setName(pDetails->name() + " Plot Widget Exporter");
   setShortDescription(pDetails->shortDescription() + " Plot Widget Exporter");
   setDescription(pDetails->description() + " Plot Widget Exporter");
   setSubtype(TypeConverter::toString<PlotWidget>());
   setDescriptorId("{2B7C526C-F752-466c-B72C-11A0BD263F23}-" + pDetails->name());
   setProductionStatus(pDetails->isProduction());
}

PicturesPlotWidgetExporter::~PicturesPlotWidgetExporter()
{
}

bool PicturesPlotWidgetExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(PicturesExporter::getInputSpecification(pArgList));
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<PlotWidget>(ExportItemArg()));

   return true;
}

bool PicturesPlotWidgetExporter::extractInputArgs(const PlugInArgList* pInArgList)
{
   VERIFY(PicturesExporter::extractInputArgs(pInArgList));
   VERIFY(pInArgList != NULL);

   mpItem = pInArgList->getPlugInArgValue<PlotWidget>(ExportItemArg());
   return (mpItem != NULL);
}

bool PicturesPlotWidgetExporter::generateImage(QImage &image)
{
   PlotWidget* pPlotWidget = dynamic_cast<PlotWidget*>(mpItem);
   if (pPlotWidget == NULL)
   {
      return false;
   }

   bool presizedImage = !image.isNull();
   QSize outputSize = image.size();
   pPlotWidget->getCurrentImage(image);
   // we just scale the plot...there's not "hidden" data that's not rendered at
   // a smaller scale like in a spatial data view. if the output size is very large
   // this could result in some aliasing and jaggies...if this becomes a problem
   // for users, we can deal with that situation later.
   if (presizedImage)
   {
      image = image.scaled(outputSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
   }

   return true;
}
