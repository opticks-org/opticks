/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef JPEG2000EXPORTER_H
#define JPEG2000EXPORTER_H

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "ExporterShell.h"

class Jpeg2000Exporter : public ExporterShell
{
public:
   Jpeg2000Exporter();
   virtual ~Jpeg2000Exporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
#endif
