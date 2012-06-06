/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MJPEGEXPORTER_H
#define MJPEGEXPORTER_H

#include "MovieExporter.h"

#include <boost/rational.hpp>

/**
 *  Avi/MJPEG Exporter
 *
 *  This plug-in exports core movie sequences to avi/MJPEG movie files.
 */
class MjpegExporter : public MovieExporter
{
public:
   MjpegExporter();
   virtual ~MjpegExporter();

protected:
   virtual AVOutputFormat* getOutputFormat() const;
   virtual boost::rational<int> convertToValidFrameRate(const boost::rational<int>& frameRate) const;
   virtual bool setAvCodecOptions(AVCodecContext* pContext);
   virtual bool convertToValidResolution(int& resolutionX, int& resolutionY) const;
};

#endif
