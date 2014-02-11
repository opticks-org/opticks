/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDHISTOGRAM_H__
#define POINTCLOUDHISTOGRAM_H__

#include "AlgorithmShell.h"

class PointCloudHistogram : public AlgorithmShell
{
public:
   PointCloudHistogram();
   virtual ~PointCloudHistogram();

public:
   virtual bool getInputSpecification(PlugInArgList*&);
   virtual bool getOutputSpecification(PlugInArgList*&);
   virtual bool execute(PlugInArgList*, PlugInArgList*);
};

#endif
