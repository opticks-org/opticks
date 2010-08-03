/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MPEG1EXPORTER_H
#define MPEG1EXPORTER_H

#include "MovieExporter.h"

/**
 *  Mpeg v1 Exporter
 *
 *  This plug-in exports core movie sequences to mpeg 1 movie files.
 */
class Mpeg1Exporter : public MovieExporter
{
public:
   Mpeg1Exporter();
   virtual ~Mpeg1Exporter();

protected:
   virtual AVOutputFormat *getOutputFormat() const;
};

#endif
