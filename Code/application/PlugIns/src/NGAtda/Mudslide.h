/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MUDSLIDE_H__
#define MUDSLIDE_H__

#include "AlgorithmShell.h"

namespace NGA
{
class Mudslide : public AlgorithmShell
{
public:
   Mudslide();
   virtual ~Mudslide();

public:
   virtual bool getInputSpecification(PlugInArgList*&);
   virtual bool getOutputSpecification(PlugInArgList*&);
   virtual bool execute(PlugInArgList*, PlugInArgList*);
};
}

#endif
