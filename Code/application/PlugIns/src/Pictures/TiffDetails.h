/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIFFDETAILS_H
#define TIFFDETAILS_H

#include <xtiffio.h>
#include <memory>

#include "PicturesExporter.h"

class TiffExportOptionsWidget;

class TiffDetails : public PicturesDetails
{
public:
   TiffDetails();

   std::string name();
   std::string shortDescription();
   std::string description();
   std::string extensions();
   bool savePict(QString strFilename, QImage img, const SessionItem *pItem);
   QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList);
   bool isProduction() const;

   /**
    *  Adds geo-information to the TIFF file.
    *
    *  @param   out
    *           The TIFF file in which to write the geo-information.
    *  @param   width
    *           The width of the image.
    *  @param   height
    *           The height of the image.
    *  @param   pItem
    *           The item being exported
    *
    *  @return  TRUE if the geo-information was successfully saved in the file, otherwise FALSE.
    */
   bool addGeoKeys(TIFF* pOut, int width, int height, const SessionItem *pItem);

private:
   void computeExportResolution(unsigned int& imageWidth, unsigned int& imageHeight);
   std::auto_ptr<TiffExportOptionsWidget> mpOptionsWidget;
};

#endif
