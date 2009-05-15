/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BMPDETAILS_H
#define BMPDETAILS_H

#include "PicturesExporter.h"

#include <memory>

class ImageResolutionWidget;

class BmpDetails : public PicturesDetails
{
public:
   std::string name();
   std::string shortDescription();
   std::string description();
   std::string extensions();
   bool savePict(QString strFilename, QImage img, const SessionItem *pItem);
   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);
   bool isProduction() const;

private:
   void computeExportResolution(unsigned int& imageWidth, unsigned int& imageHeight);
   std::auto_ptr<ImageResolutionWidget> mpOptionsWidget;
};

#endif
