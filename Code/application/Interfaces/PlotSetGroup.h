/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTSETGROUP_H
#define PLOTSETGROUP_H

#include "Serializable.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DynamicObject;
class PlotSet;
class PlotWidget;
class QIcon;
class QWidget;
class Signature;

/**
 *  A widget to manage multiple plot sets.
 *
 *  The plot set group widget contains one or more plot sets, which are tabbed
 *  widgets that manage multiple plot widgets.  The list of available plot sets
 *  is available in an information bar at the top of the widget.  Each plot set
 *  can contain one or more plots, which appear as a tab with the plot name
 *  along the bottom of the widget.
 *
 *  @see       PlotSet, PlotWidget
 */
class PlotSetGroup : public Subject, public Serializable
{
public:
   /**
    *  Emitted with boost::any<\link PlotSet\endlink*> when a new plot set is
    *  added to the widget.
    */
   SIGNAL_METHOD(PlotSetGroup, PlotSetAdded)

   /**
    *  Emitted with boost::any<\link PlotSet\endlink*> when an existing plot set
    *  is activated in the widget.
    */
   SIGNAL_METHOD(PlotSetGroup, PlotSetActivated)

   /**
    *  Emitted with boost::any<\link PlotSet\endlink*> just before a plot set in
    *  the widget is deleted.
    */
   SIGNAL_METHOD(PlotSetGroup, PlotSetDeleted)

   /**
    *  Returns the Qt widget pointer for this plot set group widget.
    *
    *  @return  A non-const pointer to the Qt widget containing the plot set
    *  group.
    *
    *  @see     getWidget() const
    */
   virtual QWidget* getWidget() = 0;

   /**
    *  Returns the Qt widget pointer for this plot set group widget.
    *
    *  @return  A const pointer to the Qt widget containing the plot set group.
    *
    *  @see     getWidget()
    */
   virtual const QWidget* getWidget() const = 0;

   /**
    *  Adds a new plot set to the widget.
    *
    *  This method adds a new plot set container widget for plots but does not
    *  add actual plots.  The new plot set widget is made active, so the widget
    *  may appear empty after calling this method.
    *
    *  @param   plotSetName
    *           The name of the new plot set, which appears in the information
    *           bar along the top of the widget.  If this method is called
    *           during loading of a session, this parameter will be interpreted
    *           as a session ID.  If the session item does not exist, a new plot
    *           set will be created.
    *
    *  @return  A pointer to the created plot set.  \c NULL is returned if a
    *           plot set with the given name already exists.
    */
   virtual PlotSet* createPlotSet(const std::string& plotSetName) = 0;

   /**
    *  Retrieves the plot set with a given name.
    *
    *  @param   plotSetName
    *           The name of the plot set to retrieve.  This method does nothing
    *           if an empty string is passed in.
    *
    *  @return  A pointer to the plot set with the given name.  \c NULL is
    *           returned if a plot set with the given name does not exist.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual PlotSet* getPlotSet(const std::string& plotSetName) const = 0;

   /**
    *  Retrieves the plot set that contains a given plot.
    *
    *  @param   pPlot
    *           The plot for which to retrieve its parent plot set.
    *
    *  @return  A pointer to the plot set containing the given plot.  \c NULL is
    *           returned if no plot set contains the given plot, or if the given
    *           plot is \c NULL.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual PlotSet* getPlotSet(PlotWidget* pPlot) const = 0;

   /**
    *  Retrieves all plot sets in the widget.
    *
    *  @return  A vector containing pointers to the plot sets currently
    *           contained in the widget.  An empty vector indicates that the
    *           widget does not contain any plot sets.
    *
    *  @see     getCurrentPlotSet()
    */
   virtual std::vector<PlotSet*> getPlotSets() const = 0;

   /**
    *  Returns the total number of plot sets.
    *
    *  @return  The number of plot sets in the widget.
    */
   virtual unsigned int getNumPlotSets() const = 0;

   /**
    *  Queries whether a plot set exists in the widget.
    *
    *  @param   pPlotSet
    *           The plot set to query for its existence.
    *
    *  @return  Returns \c true if the given plot set is contained in the
    *           widget; otherwise returns \c false.
    */
   virtual bool containsPlotSet(PlotSet* pPlotSet) const = 0;

   /**
    *  Sets the active plot set.
    *
    *  @param   pPlotSet
    *           The plot set to activate.  This method does nothing if \c NULL
    *           is passed in.
    *
    *  @return  Returns \c true if the plot set was successfully activated;
    *           otherwise returns \c false.
    */
   virtual bool setCurrentPlotSet(PlotSet* pPlotSet) = 0;

   /**
    *  Retrieves the current plot set.
    *
    *  @return  A pointer to the current plot set.  \c NULL is returned if the
    *           widget does not contain any plots.
    *
    *  @see     getPlotSets()
    */
   virtual PlotSet* getCurrentPlotSet() const = 0;

   /**
    *  Renames a plot set with a given name.
    *
    *  @param   pPlotSet
    *           The plot set to rename.  This method does nothing if \c NULL is
    *           passed in.
    *  @param   newName
    *           The new name for the plot set, which appears in the information
    *           bar along the top of the widget.  The name must be unique among
    *           the plot sets in this widget.  This method does nothing if an
    *           empty string is passed in.
    *
    *  @return  Returns \c true if the plot set was successfully renamed.
    *           Returns \c false if the plot set does not exist or if the new
    *           name is the same as that of another plot set.
    */
   virtual bool renamePlotSet(PlotSet* pPlotSet, const std::string& newName) = 0;

   /**
    *  Deletes an existing plot set from the widget.
    *
    *  This method deletes a plot set from the widget and deletes all plots
    *  contained in the plot set.
    *
    *  @param   pPlotSet
    *           The plot set to delete.  This method does nothing if \c NULL is
    *           passed in.
    *
    *  @return  Returns \c true if the plot set and its plots were successfully
    *           deleted; otherwise returns \c false.
    */
   virtual bool deletePlotSet(PlotSet* pPlotSet) = 0;

   /**
    *  Retrieves plots of a given type contained on all plot sets.
    *
    *  @param   plotType
    *           The plot type.
    *
    *  @return  A vector containing pointers to the plots of the given type in
    *           all plot sets.  An empty vector indicates that no existing plot
    *           set contains any plots of the given type.
    *
    *  @see     getPlots() const, PlotSet::getPlots()
    */
   virtual std::vector<PlotWidget*> getPlots(PlotType plotType) const = 0;

   /**
    *  Retrieves all plots contained on all plot sets.
    *
    *  @return  A vector containing pointers to all plots in all plot sets.
    *           An empty vector indicated that either there are no plot sets, or
    *           that all plot sets are empty.
    *
    *  @see     getPlots(PlotType) const, PlotSet::getPlots()
    */
   virtual std::vector<PlotWidget*> getPlots() const = 0;

   /**
    *  Returns the total number of plots in all plot sets.
    *
    *  @return  The number of plots in all plot sets in the widget.
    */
   virtual unsigned int getNumPlots() const = 0;

   /**
    *  Queries whether a plot exists on any plot set in the widget.
    *
    *  @param   pPlot
    *           The plot to query for its existence.
    *
    *  @return  Returns \c true if the plot is contained on a plot set in the
    *           widget; otherwise returns \c false.
    */
   virtual bool containsPlot(PlotWidget* pPlot) const = 0;

   /**
    *  Sets the active plot in the widget.
    *
    *  @param   pPlot
    *           The plot to activate.  If the plot is not on the current plot
    *           set, the plot set containing the given plot is also activated.
    *           This method does nothing if \c NULL is passed in.
    *
    *  @return  Returns \c true if the plot was successfully activated;
    *           otherwise returns \c false.
    */
   virtual bool setCurrentPlot(PlotWidget* pPlot) = 0;

   /**
    *  Returns the active plot on the current plot set.
    *
    *  @return  A pointer to the active plot.  \c NULL is returned if no plot
    *           sets exist or if the current plot set does not contain any
    *           plots.
    */
   virtual PlotWidget* getCurrentPlot() const = 0;

   /**
    *  Adds Signature data to a plot, creating the plot if necessary.
    *
    *  This method attempts to create a new dataset on an existing or new plot.
    *  If the specified Signature has attributes matching the specified names,
    *  the attributes are of type vector<basic-numeric-type>, and the vectors
    *  are of the same size, it will create a new dataset.  If the plot name
    *  parameter does not refer to an existing plot, it will create a new plot
    *  and put the dataset in it.  If the plot name parameter refers to an
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
    *           The name of the plot in which to add the data.  If the specified
    *           plot does not exist, it will be created.  This method does
    *           nothing if an empty string is passed in.
    *
    *  @return  A pointer to the plot in which the signature data was added.
    *           Returns \c NULL if the function failed for any reason.
    *
    *  @see     plotData(const DynamicObject&, const std::string&, const std::string&, const std::string&)
    */
   virtual PlotWidget* plotData(const Signature& sig, const std::string& xAttribute, const std::string& yAttribute,
      const std::string& plotName) = 0;

   /**
    *  Adds DynamicObject data to a plot, creating the plot if necessary.
    *
    *  This method attempts to create a new dataset on an existing or new plot.
    *  If the specified DynamicObject has attributes matching the specified
    *  names, the attributes are of type vector<basic-numeric-type>, and the
    *  vectors are of the same size, it will create a new dataset.  If the plot
    *  name parameter does not refer to an existing plot, it will create a new
    *  plot and put the dataset in it.  If the plot name parameter refers to an
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
    *           The name of the plot in which to add the data.  If the specified
    *           plot does not exist, it will be created.  This method does
    *           nothing if an empty string is passed in.
    *
    *  @return  A pointer to the plot in which the dynamic object data was
    *           added.  Returns \c NULL if the function failed for any reason.
    *
    *  @see     plotData(const Signature&, const std::string&, const std::string&, const std::string&)
    */
   virtual PlotWidget* plotData(const DynamicObject& obj, const std::string& xAttribute, const std::string& yAttribute,
      const std::string& plotName) = 0;

   /**
    *  Deletes all plot sets in the widget.
    *
    *  This method first deletes all plots in each plot set and then deletes
    *  all plot sets in the widget.
    */
   virtual void clear() = 0;

   /**
    *  Sets the icon in the information bar at the top of the widget.
    *
    *  @param   icon
    *           The icon to set in the information bar.
    */
   virtual void setInfoBarIcon(const QIcon& icon) = 0;

protected:
   /**
    *  The plot set group should be destroyed only if it is not owned by a
    *  parent widget by calling DesktopServices::deletePlotSetGroup().
    */
   virtual ~PlotSetGroup()
   {}
};

#endif
