/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREPLOT_H
#define SIGNATUREPLOT_H

#include "CartesianPlot.h"

/**
 *  Plots Signature objects.
 *
 *  This plot will plot signature objects and allows interaction with the
 *  Signature objects currently being plotted.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in CartesianPlot.
 *
 *  @see     PlotView
 */
class SignaturePlot : public CartesianPlot
{
protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~SignaturePlot() {}
};

#endif
