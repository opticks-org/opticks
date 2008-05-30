/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AVIEXPORTER_H
#define AVIEXPORTER_H

#include "MovieExporter.h"

/**
 *  Avi/wmv Exporter
 *
 *  This plug-in exports core movie sequences to avi/wmv movie files.
 */
class AviExporter : public MovieExporter
{
public:
   AviExporter();
   virtual ~AviExporter();

protected:
   virtual AVOutputFormat *getOutputFormat() const;
};

#endif
