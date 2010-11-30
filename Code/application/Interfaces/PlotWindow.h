/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "DockWindow.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DynamicObject;
class PlotSet;
class PlotWidget;
class Signature;

/**
 *  A window to manage multiple plot views.
 *
 *  A plot window contiains one or more tabbed widgets inside of a dock window.  A tabbed
 *  widget supports one plot on each tab, and collectively make up a plot set.  Multiple
 *  plot sets can be managed within the plot window.  The current plot set name appears on
 *  an information bar at the top of the window and users can change the current plot set.
 *  Each plot in a plot set must have a unique name to the plot set which appears on its tab.
 *
 *  When a context menu is invoked for the plot window, the
 *  DockWindow::signalAboutToShowContextMenu() signal is emitted where the
 *  session items vector in the context menu contains both the plot window and
 *  the current plot set.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: createPlotSet(), setCurrentPlotSet(), 
 *    deletePlotSet(), clear().
 *  - Everything else documented in DockWindow.
 *
 *  @see     DockWindow, PlotSet, PlotWidget
 */
class PlotWindow : public DockWindow
{
public:
   /**
    *  Emitted with any<PlotSet*> when a PlotSet is made current.
    */
   SIGNAL_METHOD(PlotWindow, PlotSetActivated)
   /**
    *  Emitted with any<PlotSet*> when a PlotSet is added to the window.
    */
   SIGNAL_METHOD(PlotWindow, PlotSetAdded)
   /**
    *  Emitted with any<PlotSet*> when a PlotSet is deleted from the window.
    */
   SIGNAL_METHOD(PlotWindow, PlotSetDeleted)

   /**
    *  Adds a new plot set to the window.
    *
    *  This method adds a new container for plots but does not add actual plots.  The
    *  new plot container is made active, so the window may appear empty after calling
    *  this method.
    *
    *  @param   plotSet
    *           The name of the new plot set, which appears in the information bar
    *           along the top of the window. If this method is called during session restore,
    *           this parameter will be interpreted as a session id. If the session item does not
    *           exist, a new plot set will be created.
    *
    *  @return  A pointer to the created plot set.  NULL is returned if a plot set with
    *           the given name already exists.
    *
    *  @notify  This method will notify signalPlotSetAdded with any<PlotSet*>.
    */
   virtual PlotSet* createPlotSet(const std::string& plotSet) = 0;

   /**
    *  Retrieves the plot set with a given name.
    *
    *  @param   plotSet
    *           The name of the plot set to retrieve.  Cannot be empty.
    *
    *  @return  A pointer to the plot set with the given name.  NULL is returned if a
    *           plot set with the given name does not exist.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual PlotSet* getPlotSet(const std::string& plotSet) const = 0;

   /**
    *  Retrieves the plot set that contains a given plot.
    *
    *  @param   pPlot
    *           The plot for which to retrieve its parent plot set.  Cannot be NULL.
    *
    *  @return  A pointer to the plot set containing the given plot.  NULL is returned
    *           if no plot set contains the given plot.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual PlotSet* getPlotSet(PlotWidget* pPlot) const = 0;

   /**
    *  Retrieves all plot sets in the window.
    *
    *  @param   plotSets
    *           A vector that is populated with pointers to the plot sets currently
    *           contained in the window.  The vector is emptied if the window does not
    *           contain any plot sets.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual void getPlotSets(std::vector<PlotSet*>& plotSets) const = 0;

   /**
    *  Returns the total number of plot sets in the window.
    *
    *  @return  The number of plot sets in the window.
    */
   virtual unsigned int getNumPlotSets() const = 0;

   /**
    *  Queries whether a plot set exists in the window.
    *
    *  @param   pPlotSet
    *           The plot set to query for its existence.  Cannot be NULL.
    *
    *  @return  TRUE if the plot set is contained in the window, otherwise FALSE.
    */
   virtual bool containsPlotSet(PlotSet* pPlotSet) const = 0;

   /**
    *  Sets the active plot set.
    *
    *  @param   pPlotSet
    *           The plot set to activate.  Cannot be NULL.
    *
    *  @return  TRUE is the plot set was successfully activated, otherwise FALSE.
    *
    *  @notify  This method will notify signalPlotSetActivated with any<PlotSet*>.
    */
   virtual bool setCurrentPlotSet(PlotSet* pPlotSet) = 0;

   /**
    *  Retrieves the current plot set.
    *
    *  @return  A pointer to the current plot set.  NULL is returned if the window does
    *           not contain any plots.
    *
    *  @see     getPlotSets()
    */
   virtual PlotSet* getCurrentPlotSet() const = 0;

   /**
    *  Renames a plot set with a given name.
    *
    *  @param   pPlotSet
    *           The plot set to rename.  Cannot be NULL.
    *  @param   newPlotSetName
    *           The new plot set name, which appears in the information bar along the
    *           top of the window..  The name must be unique among plot sets of this
    *           window.
    *
    *  @return  TRUE if the plot set was successfully renamed.  FALSE if the plot set
    *           does not exist or the new name is the same as that of another plot set.
    */
   virtual bool renamePlotSet(PlotSet* pPlotSet, const std::string& newPlotSetName) = 0;

   /**
    *  Deletes an existing plot set from the window.
    *
    *  This method deletes a plot set from the window and deletes all plots contained
    *  in the plot set.
    *
    *  @param   pPlotSet
    *           The plot set to delete.  The name of the plot set appears in the
    *           information bar along the top of the window.
    *
    *  @return  TRUE if the plot set and its plots were successfully deleted, otherwise
    *           FALSE.
    *
    *  @notify  This method will notify signalPlotSetDeleted with any<PlotSet*>.
    */
   virtual bool deletePlotSet(PlotSet* pPlotSet) = 0;

   /**
    *  Retrieves plots of a given type contained on all plot sets.
    *
    *  @param   plotType
    *           The plot type.
    *  @param   plots
    *           A vector to contain pointers to the plots in all plot sets.  The vector
    *           is emptied if all existing plot sets do not contain any plots of the given
    *           type.
    *
    *  @see     PlotSet::getPlots()
    */
   virtual void getPlots(const PlotType& plotType, std::vector<PlotWidget*>& plots) const = 0;

   /**
    *  Retrieves all plots contained on all plot sets.
    *
    *  @param   plots
    *           A vector to contain pointers to the plots in all plot sets.  The vector
    *           is emptied if all existing plot sets do not contain any plots.
    *
    *  @see     PlotSet::getPlots()
    */
   virtual void getPlots(std::vector<PlotWidget*>& plots) const = 0;

   /**
    *  Returns the total number of plots on all plot sets in the window.
    *
    *  @return  The total number of plots on all plot sets.
    */
   virtual unsigned int getNumPlots() const = 0;

   /**
    *  Queries whether a plot exists on any plot set in the window.
    *
    *  @param   pPlot
    *           The plot to query for its existence.  Cannot be NULL.
    *
    *  @return  TRUE if the plot is contained on a plot set in the window, otherwise
    *           FALSE.
    */
   virtual bool containsPlot(PlotWidget* pPlot) const = 0;

   /**
    *  Sets the active plot in the window.
    *
    *  @param   pPlot
    *           The plot to make the active plot.  If the plot is not on the current
    *           plot set, the plot's set is also activated.  Cannot be NULL.
    *
    *  @return  TRUE if the plot was successfully activated, otherwise FALSE.
    */
   virtual bool setCurrentPlot(PlotWidget* pPlot) = 0;

   /**
    *  Returns the active plot on the current plot set.
    *
    *  @return  A pointer to the plot.  NULL is returned if no plot sets exist or if
    *           the current plot set does not contain any plots.
    */
   virtual PlotWidget* getCurrentPlot() const = 0;

   /**
    *  Removes all plot sets from the window.
    *
    *  This method first removes all plots from each plot set and then removes all plot
    *  sets from the window.
    *
    *  @notify  This method will notify signalPlotSetDeleted with any<PlotSet*>
    *           for each PlotSet removed.
    */
   virtual void clear() = 0;

   /**
    *  Adds data to a plot, creating the plot if necessary.
    *
    *  This method attempts to create a new dataset on an existing or new plot.
    *  If the specified Signature has attributes matching the specified names,
    *  the attributes are of type vector<basic-numeric-type> and the vectors
    *  are of the same size, it will create a new dataset. If the plot name
    *  parameter does not refer to an existing plot, it will create a new plot 
    *  and put the dataset in it. If the plot name parameter refers to an 
    *  existing plot, it will attempt to add the dataset to the specified plot.
    *  If the attribute names don't match the names of the X and Y axes in the 
    *  plot, it will query the user as to whether it should add the dataset 
    *  anyway.
    *
    *  @param   sig
    *           The Signature to extract the data from.
    *  @param   xAttribute
    *           The name of the Signature attribute to use for the X-axis 
    *           values.
    *  @param   yAttribute
    *           The name of the Signature attribute to use for the Y-axis 
    *           values.
    *  @param   plotName
    *           The name of the plot to display the data in. If the specified
    *           plot does not exist, it will be created. If the name is empty
    *           the function will fail.
    *
    *  @return  A pointer to the plot the data was added to. NULL is returned 
    *           if the function failed for any reason.
    */
   virtual PlotWidget* plotData(const Signature &sig, const std::string &xAttribute,
      const std::string &yAttribute, const std::string &plotName) = 0;

   /**
    *  Adds data to a plot, creating the plot if necessary.
    *
    *  This method attempts to create a new dataset on an existing or new plot.
    *  If the specified DynamicObject has attributes matching the specified names,
    *  the attributes are of type vector<basic-numeric-type> and the vectors
    *  are of the same size, it will create a new dataset. If the plot name
    *  parameter does not refer to an existing plot, it will create a new plot 
    *  and put the dataset in it. If the plot name parameter refers to an 
    *  existing plot, it will attempt to add the dataset to the specified plot.
    *  If the attribute names don't match the names of the X and Y axes in the 
    *  plot, it will query the user as to whether it should add the dataset 
    *  anyway.
    *
    *  @param   obj
    *           The DynamicObject to extract the data from.
    *  @param   xAttribute
    *           The name of the DynamicObject attribute to use for the X-axis 
    *           values.
    *  @param   yAttribute
    *           The name of the DynamicObject attribute to use for the Y-axis 
    *           values.
    *  @param   plotName
    *           The name of the plot to display the data in. If the specified
    *           plot does not exist, it will be created. If the name is empty
    *           the function will fail.
    *
    *  @return  A pointer to the plot the data was added to. NULL is returned 
    *           if the function failed for any reason.
    */
   virtual PlotWidget* plotData(const DynamicObject &obj, const std::string &xAttribute,
      const std::string &yAttribute, const std::string &plotName) = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~PlotWindow() {}
};

#endif
