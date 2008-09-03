/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POLARPLOT_H
#define POLARPLOT_H

#include "PlotView.h"

class PolarGridlines;

/**
 *  A polar plot.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - Everything documented in PlotView.
 *
 *  @see     PlotView
 */
class PolarPlot : public PlotView
{
public:
   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref polarplot "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Returns a pointer to the plot's gridlines.
    *
    *  @return  A pointer to the plot's gridlines object.
    */
   virtual PolarGridlines* getGridlines() = 0;

   /**
    *  Returns read-only access to the plot's gridlines.
    *
    *  @return  A const pointer to the plot's gridlines object.  The plot
    *           object represented by the returned pointer should not be
    *           modified.  To modify the values, call the non-const version of
    *           getGridlines().
    */
   virtual const PolarGridlines* getGridlines() const = 0;

protected:
   /**
    * This object should be destroyed by calling DesktopServices::deleteView().
    */
   virtual ~PolarPlot() {}
};

#endif
