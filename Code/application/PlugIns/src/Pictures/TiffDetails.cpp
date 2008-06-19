/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <geovalues.h>
#include <geo_normalize.h>
#include <geotiffio.h>     // public interface
#include <geo_tiffp.h>     // external TIFF interface
#include <geo_keyp.h>      // private interface

#include "AnnotationLayer.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "OptionsTiffExporter.h"
#include "ProductView.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "TiffDetails.h"

#include <string>

TiffDetails::TiffDetails() : mpOptionsWidget(NULL)
{
}

std::string TiffDetails::name()
{
   return "GeoTIFF";
}

std::string TiffDetails::shortDescription()
{
   return "TIFF";
}

std::string TiffDetails::description()
{
   return "Tagged Image File Format";
}

std::string TiffDetails::extensions()
{
   return "TIFF Files (*.tif)";
}

QWidget* TiffDetails::getExportOptionsWidget(const PlugInArgList *)
{
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new (std::nothrow) OptionsTiffExporter());
      if (mpOptionsWidget.get() != NULL)
      {
         mpOptionsWidget->setPromptUserToSaveSettings(true);
      }
   }

   return mpOptionsWidget.get();
}

bool TiffDetails::savePict(QString strFilename, QImage img, const SessionItem *pItem)
{
   if(strFilename.isEmpty() || img.isNull())
   {
      return false;
   }

   std::string filename = strFilename.toStdString();

#if defined(WIN_API)
   // replace "/" with "\\" to accommodate the sgi specific code
   // in the library
   unsigned int i;
   for (i= 0; i < filename.length(); i++)
   {
      if (filename[i] == '/')
         filename[i] = '\\';
   }
#endif

   //open the file to write
   TIFF* pOut = XTIFFOpen(filename.c_str(), "wb");
   if(pOut == NULL)
   {
      return false;
   }

   bool packBits = OptionsTiffExporter::getSettingPackBitsCompression();
   if (mpOptionsWidget.get() != NULL)
   {
      mpOptionsWidget->applyChanges();
      packBits = mpOptionsWidget->getPackBitsCompression();
   }

   // Initialize private variables for tiff
   TIFFSetField(pOut, TIFFTAG_IMAGEWIDTH, static_cast<uint32>(img.width()));
   TIFFSetField(pOut, TIFFTAG_IMAGELENGTH, static_cast<uint32>(img.height()));
   TIFFSetField(pOut, TIFFTAG_BITSPERSAMPLE, 8);
   TIFFSetField(pOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
   TIFFSetField(pOut, TIFFTAG_SAMPLESPERPIXEL, 3);
   TIFFSetField(pOut, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
   TIFFSetField(pOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(pOut, 1L));
   TIFFSetField(pOut, TIFFTAG_COMPRESSION, (packBits ? COMPRESSION_PACKBITS : COMPRESSION_NONE));

   //allocate the memory needed
   tdata_t pBuf = _TIFFmalloc(TIFFScanlineSize(pOut));

   //loop through the image and convert to tiff and write to the file
   int x, y;
   for(y = 0; y < img.height(); y++) 
   {
      uint8* pp = static_cast<uint8*>(pBuf);

      QRgb *pData = reinterpret_cast<QRgb*>(img.scanLine(y));
      for(x = 0; x < img.width(); x++) 
      {
         pp[0] = qRed(pData[x]);
         pp[1] = qGreen(pData[x]);
         pp[2] = qBlue(pData[x]);
         pp += 3;
      }

      //write out 1 row and check for error
      if(TIFFWriteScanline(pOut, pBuf, y, 0) < 0)
      {
         //error message
         return false;
      }
   }

   // Add in the geo info if it is available
   addGeoKeys(pOut, img.width(), img.height(), pItem);

   XTIFFClose(pOut);
   return true;
}

bool TiffDetails::addGeoKeys(TIFF* pOut, int width, int height, const SessionItem *pItem)
{
   if((pOut == NULL) || (width == 0) || (height == 0))
   {
      return false;
   }

   const View* pInputView = dynamic_cast<const View*>(pItem);
   if (pInputView == NULL)
   {
      return false;
   }

   GTIF *pGtif = GTIFNew(pOut);
   if(pGtif == NULL)
   {
      return false;
   }

   RasterElement* pGeoreferencedRaster = NULL; // First raster element we find with georeferencing information
   const ProductView* pView = dynamic_cast<const ProductView*>(pInputView);
   if(pView != NULL)
   {
      AnnotationLayer* pAnno = pView->getLayoutLayer();
      if(pAnno == NULL)
      {
         GTIFFree(pGtif);
         return false;
      }

      /* NOTE: If we find more than one SpatialDataView with a georeferenced RasterElement, we will only provide
      geo-data for the FIRST one - because two views could theoretically screw up the
      geo-data if one is in Australia and the other in Canada.
      */

      // get all the view objects
      std::list<GraphicObject*> objs;
      pAnno->getObjects(VIEW_OBJECT, objs);

      // for every object, find the data set with a geocoord matrix
      for(std::list<GraphicObject*>::iterator it = objs.begin();
         it != objs.end(); ++it)
      {
         GraphicObject* pObj = *it;
         if(pObj != NULL)
         {
            SpatialDataView *pSpView = dynamic_cast<SpatialDataView*>(pObj->getObjectView());
            if(pSpView != NULL)
            {
               LayerList *pLayerList = pSpView->getLayerList();
               if (pLayerList != NULL)
               {
                  RasterElement *pRaster = pLayerList->getPrimaryRasterElement();
                  if (pRaster != NULL && pRaster->isGeoreferenced())
                  {
                     pGeoreferencedRaster = pRaster;
                     break;
                  }
               }
            }
         }
      }
   }

   const SpatialDataView *pSpView = dynamic_cast<const SpatialDataView*>(pInputView);
   if (pSpView != NULL)
   {
      LayerList *pLayerList = pSpView->getLayerList();
      if (pLayerList != NULL)
      {
         RasterElement *pRaster = pLayerList->getPrimaryRasterElement();
         if (pRaster != NULL && pRaster->isGeoreferenced())
         {
            pGeoreferencedRaster = pRaster;
         }
      }
   }

   if(pGeoreferencedRaster != NULL)
   {
      LocationType lowerLeft;
      LocationType upperLeft;
      LocationType upperRight;
      LocationType lowerRight;
      pInputView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

      LocationType latLong;
      //get the lat/long's (0,0)
      latLong = pGeoreferencedRaster->convertPixelToGeocoord(upperLeft);
      double ll1y = latLong.mY;               //delta long
      double ll1x = latLong.mX;               //delta lat

      latLong = pGeoreferencedRaster->convertPixelToGeocoord(upperRight);
      double ll2y = latLong.mY;               //long  
      double ll2x = latLong.mX;               //lat

      latLong = pGeoreferencedRaster->convertPixelToGeocoord(lowerLeft);
      double ll3y = latLong.mY;               //long
      double ll3x = latLong.mX;               //lat

      //compute transformation Matrix values
      //added slight modification, must divide by magnitude
      double a = (ll2y - ll1y) / width;
      double b = (ll3y - ll1y) / height;
      double d = (ll1y);
      double e = (ll2x - ll1x) / width;
      double f = (ll3x - ll1x) / height;
      double h = (ll1x);
      double k = 1.0, p = 1.0;

      double tMatrix[16] = {a, b, 0.0, d,
                            e, f, 0.0, h,
                            0.0, 0.0, k, 0.0,
                            0.0, 0.0, 0.0, p};

      TIFFSetField(pOut, GTIFF_TRANSMATRIX, 16, tMatrix);
    
      GTIFKeySet(pGtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelGeographic);
      GTIFKeySet(pGtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsArea);
      GTIFKeySet(pGtif, GeogAngularUnitsGeoKey, TYPE_SHORT,  1, Angular_Degree);
      GTIFKeySet(pGtif, GeogLinearUnitsGeoKey, TYPE_SHORT,  1, Linear_Meter);
      GTIFKeySet(pGtif, ProjCenterLongGeoKey, TYPE_DOUBLE,  1, latLong.mY);
      GTIFKeySet(pGtif, ProjCenterLatGeoKey, TYPE_DOUBLE,  1, latLong.mX);
   
      // Here we violate the GTIF abstraction to retarget on another file.
      // We should just have a function for copying tags from one GTIF object
      // to another.
      pGtif->gt_tif = pOut;
      pGtif->gt_flags |= FLAG_FILE_MODIFIED;
   }

   // Install keys and tags 
   GTIFWriteKeys(pGtif);
   GTIFFree(pGtif);
   return true;
}

bool TiffDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}
