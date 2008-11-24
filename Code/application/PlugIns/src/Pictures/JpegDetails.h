/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEGDETAILS_H
#define JPEGDETAILS_H

#include "PicturesExporter.h"

class OptionsJpegExporter;
class QWidget;

class JpegDetails : public PicturesDetails
{
public:
   JpegDetails();

   std::string name();
   std::string shortDescription();
   std::string description();
   std::string extensions();
   bool savePict(QString strFilename, QImage img, const SessionItem *pItem);
   QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList);
   bool isProduction() const;

private:
   std::auto_ptr<OptionsJpegExporter> mpOptionsWidget;
};

#endif
