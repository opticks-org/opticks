/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTSET_H
#define PLOTSET_H

#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class PlotWidget;
class View;

/**
 *  A collection of plots in a plot window.
 *
 *  A plot set provides for a grouping of plots within a plot window.  A plot set displays
 *  plots on a set of tabs along the bottom of the plot window.  Each tab contains a single
 *  plot widget and the plot set name appears at the top of the plot window.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: deletePlot(), setName(), 
 *    setCurrentPlot(), createPlot(), clear()
 *  - Everything else documented in Subject.
 *
 *  @see     PlotWidget, PlotWindow
 */
class PlotSet : public SessionItem, public Subject
{
public:
   /**
    *  Emitted with any<std::string> when the plotset is renamed.
    */
   SIGNAL_METHOD(PlotSet, Renamed)
   /**
    *  Emitted with any<PlotWidget*> when a plot is activated.
    */
   SIGNAL_METHOD(PlotSet, Activated)
   /**
    *  Emitted with any<PlotWidget*> when a plot is deleted from the set.
    */
   SIGNAL_METHOD(PlotSet, PlotDeleted)
   /**
    *  Emitted with any<PlotWidget*> when a plot is added to the set.
    */
   SIGNAL_METHOD(PlotSet, PlotAdded)

   /**
    *  Sets the plot set name.
    *
    *  @param   name
    *           The new plot set name.  Cannot be empty.
    *
    *  @notify  This method will notify signalRenamed with any<std::string>.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Creates a new plot in the plot set.
    *
    *  This method creates an empty plot and adds it to plot set.
    *
    *  @param   plotName
    *           The plot name.
    *  @param   plotType
    *           The plot type.
    *
    *  @return  A pointer to the created plot.  NULL is returned if an error occurred and the
    *           plot could not be created.
    *
    *  @notify  This method will notify signalPlotAdded with any<PlotWidget*>.
    */
   virtual PlotWidget* createPlot(const std::string& plotName, const PlotType& plotType) = 0;

   /**
    *  Retrieves the plot with a given name.
    *
    *  @param   plotName
    *           The plot name.
    *
    *  @return  A pointer to the plot.  NULL is returned if no plot exists with the given
    *           name in the plot set.
    *
    *  @see     getPlots()
    */
   virtual PlotWidget* getPlot(const std::string& plotName) const = 0;

   /**
    *  Retrieves plots of a given type contained in the plot set.
    *
    *  @param   plotType
    *           The plot type.
    *  @param   plots
    *           A vector to contain pointers to the plots in the plot set.  The vector is
    *           emptied if the plot set does not contain any plots of the given type.
    *
    *  @see     getPlot(), getCurrentPlot()
    */
   virtual void getPlots(const PlotType& plotType, std::vector<PlotWidget*>& plots) const = 0;

   /**
    *  Retrieves all plots contained in the plot set.
    *
    *  @param   plots
    *           A vector to contain pointers to the plots in the plot set.  The vector is
    *           emptied if the plot set does not contain any plots.
    *
    *  @see     getPlot(), getCurrentPlot()
    */
   virtual void getPlots(std::vector<PlotWidget*>& plots) const = 0;

   /**
    *  Returns the number of plots in the plot set.
    *
    *  @return  The number of plots in the plot set.
    */
   virtual unsigned int getNumPlots() const = 0;

   /**
    *  Queries whether a plot exists in the plot set.
    *
    *  @param   pPlot
    *           The plot to query for its existence.  Cannot be NULL.
    *
    *  @return  TRUE if the plot is contained in the plot set, otherwise FALSE.
    */
   virtual bool containsPlot(PlotWidget* pPlot) const = 0;

   /**
    *  Sets the active plot in the plot set.
    *
    *  @param   pPlot
    *           The plot to make the active plot.  Cannot be NULL.
    *
    *  @return  TRUE if the plot was successfully activated, otherwise FALSE.
    *
    *  @notify  This method will notify signalActivated with any<PlotWidget*>.
    */
   virtual bool setCurrentPlot(PlotWidget* pPlot) = 0;

   /**
    *  Returns the active plot for the plot set.
    *
    *  @return  A pointer to the plot.  NULL is returned if the plot set does not
    *           contain any plots.
    *
    *  @see     getPlot()
    */
   virtual PlotWidget* getCurrentPlot() const = 0;

   /**
    *  Renames a plot in the plot set with a given name.
    *
    *  @param    pPlot
    *            The plot to rename.  Cannot be NULL.
    *  @param    newPlotName
    *            The new plot name, which appears on the tab.  The name cannot have the
    *            same name as another plot in the plot set.
    *
    *  @return   TRUE if the plot was successfully renamed.  FALSE if the plot does not
    *            exist or the new name is the same as that of another plot.
    *
    *  @notify  This method will notify View::signalRenamed with any<std::string>.
    */
   virtual bool renamePlot(PlotWidget* pPlot, const std::string& newPlotName) = 0;

   /**
    *  Deletes a plot in the plot set.
    *
    *  @param   pPlot
    *           The plot to delete.  Cannot be NULL.
    *
    *  @return  TRUE if the plot was successfully deleted from this plot set,
    *           otherwise FALSE.
    *
    *  @notify  This method will notify signalPlotDeleted with any<PlotWidget*>.
    */
   virtual bool deletePlot(PlotWidget* pPlot) = 0;

   /**
    *  Removes all plots from the plot set.
    *
    *  @notify  This method will notify signalPlotDeleted with any<PlotWidget*> 
    *           for each plot removed from the set.
    */
   virtual void clear() = 0;

   /**
    *  Associates a view with the plot set.
    *
    *  This method associates a view with the plot set.  If the plot set has an associated
    *  view and the view is deleted, the plot set is also deleted.
    *
    *  @param   pView
    *           The view to associate with plot set.  If \e pView is NULL, the plot
    *           set does not have an associated view.
    */
   virtual void setAssociatedView(View* pView) = 0;

   /**
    *  Returns the view associated with the plot set.
    *
    *  @return  The view currently associated with the plot set.  NULL is returned if no
    *           view is associated.
    *
    *  @see     setAssociatedView()
    */
   virtual View* getAssociatedView() const = 0;

   /**
    *  Renames a plot in the plot set with a user-defined name.
    *
    *  This method prompts the user to select a new name for the given plot and renames
    *  the plot.
    *
    *  @param    pPlot
    *            The plot to rename.  Cannot be NULL.
    *
    *  @return   The new plot name, which is unique for all plots in the plot set. An
    *            empty string is returned if the user cancels the process.
    *
    *  @notify  This method will notify View::signalRenamed with any<std::string>.
    */
   virtual std::string renamePlot(PlotWidget* pPlot) = 0;

protected:
   /**
    * This should be destroyed by calling PlotWindow::deletePlotSet.
    */
   virtual ~PlotSet() {}
};

#endif
