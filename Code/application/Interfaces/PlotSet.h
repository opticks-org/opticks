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
class QWidget;
class View;

/**
 *  A collection of plots in a single widget.
 *
 *  A plot set provides a grouping of plots within a single widget.  A plot set
 *  displays individual plots on tabs along the bottom of the widget.  Each tab
 *  contains a single plot widget.
 *
 *  To display multiple plot set widgets within a single widget, see the
 *  PlotSetGroup class.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setName(), setAssociatedView(),
 *    createPlot(), setCurrentPlot(), deletePlot(), clear()
 *  - Everything else documented in Subject.
 *
 *  @see     PlotSetGroup, PlotWidget
 */
class PlotSet : public SessionItem, public Subject
{
public:
   /**
    *  Emitted when the plot set is renamed with boost::any<std::string>
    *  containing the new plot set name.
    *
    *  @see     setName()
    */
   SIGNAL_METHOD(PlotSet, Renamed)

   /**
    *  Emitted when a view is associated with the plot set with
    *  boost::any<\link View\endlink*> containing a pointer to the newly
    *  associated view.
    *
    *  @see     setAssociatedView()
    */
   SIGNAL_METHOD(PlotSet, ViewAssociated)

   /**
    *  Emitted when a plot widget is added to the plot set with
    *  boost::any<\link PlotWidget\endlink*> containing a pointer to the plot
    *  widget that is being added.
    *
    *  @see     createPlot()
    */
   SIGNAL_METHOD(PlotSet, PlotAdded)

   /**
    *  Emitted when a plot widget is activated with
    *  boost::any<\link PlotWidget\endlink*> containing a pointer to the newly
    *  activated plot widget.
    *
    *  @see     setCurrentPlot()
    */
   SIGNAL_METHOD(PlotSet, Activated)

   /**
    *  Emitted when a plot widget is deleted from the plot set with
    *  boost::any<\link PlotWidget\endlink*> containing a pointer to the plot
    *  widget that is being deleted.
    *
    *  @see     deletePlot(), clear()
    */
   SIGNAL_METHOD(PlotSet, PlotDeleted)

   /**
    *  Sets the plot set name.
    *
    *  @param   name
    *           The new plot set name.  This method does nothing if an empty
    *           string is passed in.
    *
    *  @notify  This method will notify signalRenamed() with
    *           boost::any<std::string> containing the new plot set name.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Returns the Qt widget pointer for this plot set widget.
    *
    *  @return  A non-const pointer to the Qt widget containing the plot set.
    *
    *  @see     getWidget() const
    */
   virtual QWidget* getWidget() = 0;

   /**
    *  Returns the Qt widget pointer for this plot set widget.
    *
    *  @return  A const pointer to the Qt widget containing the plot set.
    *
    *  @see     getWidget()
    */
   virtual const QWidget* getWidget() const = 0;

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
    *  @return  A pointer to the created plot.  \c NULL is returned if an error
    *           occurred and the plot could not be created.
    *
    *  @notify  This method will notify signalPlotAdded() with
    *           boost::any<\link PlotWidget\endlink*> containing a pointer to
    *           the plot widget that is created.
    */
   virtual PlotWidget* createPlot(const std::string& plotName, const PlotType& plotType) = 0;

   /**
    *  Retrieves the plot with a given name.
    *
    *  @param   plotName
    *           The plot name.
    *
    *  @return  A pointer to the plot.  \c NULL is returned if no plot exists
    *           with the given name in the plot set.
    *
    *  @see     getPlots(), getCurrentPlot()
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
    *           The plot to query for its existence.
    *
    *  @return  Returns \c true if the plot is contained in the plot set;
    *           otherwise returns \c false.
    */
   virtual bool containsPlot(PlotWidget* pPlot) const = 0;

   /**
    *  Sets the active plot in the plot set.
    *
    *  @param   pPlot
    *           The plot to make the active plot.  This method does nothing if
    *           \c NULL is passed in.
    *
    *  @return  Returns \c true if the plot was successfully activated;
    *           otherwise returns \c false.
    *
    *  @notify  This method will notify signalActivated() with
    *           boost::any<\link PlotWidget\endlink*> containing a pointer to
    *           the newly activated plot widget.
    */
   virtual bool setCurrentPlot(PlotWidget* pPlot) = 0;

   /**
    *  Returns the active plot for the plot set.
    *
    *  @return  A pointer to the active plot.  \c NULL is returned if the plot
    *           set does not contain any plots.
    *
    *  @see     getPlot()
    */
   virtual PlotWidget* getCurrentPlot() const = 0;

   /**
    *  Renames a plot in the plot set with a given name.
    *
    *  @param   pPlot
    *           The plot to rename.  This method does nothing if \c NULL is
    *           passed in.
    *  @param   newPlotName
    *           The new plot name, which appears on the tab.  The name cannot
    *           have the same name as another plot in the plot set.  This method
    *           does nothing if an empty string is passed in.
    *
    *  @return  Returns \c true if the plot was successfully renamed.  Returns
    *           \c false if the plot does not exist or the new name is the same
    *           as that of another plot.
    *
    *  @notify  This method will notify View::signalRenamed() with
    *           boost::any<std::string> containing the new plot name.
    *
    *  @see     renamePlot(PlotWidget*)
    */
   virtual bool renamePlot(PlotWidget* pPlot, const std::string& newPlotName) = 0;

   /**
    *  Deletes a plot in the plot set.
    *
    *  @param   pPlot
    *           The plot to delete.  This method does nothing if \c NULL is
    *           passed in.
    *
    *  @return  Returns \c true if the plot was successfully deleted from this
    *           plot set; otherwise returns \c false.
    *
    *  @notify  This method will notify signalPlotDeleted() with
    *           boost::any<\link PlotWidget\endlink*> containing a pointer to
    *           the plot widget that is being deleted.
    */
   virtual bool deletePlot(PlotWidget* pPlot) = 0;

   /**
    *  Removes all plots from the plot set.
    *
    *  @notify  This method will notify signalPlotDeleted() with
    *           boost::any<\link PlotWidget\endlink*> containing a pointer to
    *           the plot widget that is being deleted for each plot removed from
    *           the set.
    */
   virtual void clear() = 0;

   /**
    *  Associates a view with the plot set.
    *
    *  This method associates a view with the plot set.  When a view is
    *  associated with the plot set, plots are added and removed as layers are
    *  created and destroyed in the view.
    *
    *  @warning If the associated view is deleted, the plot set is also deleted
    *           automatically.
    *
    *  @param   pView
    *           The view to associate with plot set.  If \e pView is \c NULL,
    *           the plot set does not have an associated view.
    *
    *  @notify  This method will notify signalViewAssociated() with
    *           boost::any<\link View\endlink*> containing a pointer to the
    *           newly associated view.
    */
   virtual void setAssociatedView(View* pView) = 0;

   /**
    *  Returns the view associated with the plot set.
    *
    *  @return  A non-const pointer to the view currently associated with the
    *           plot set.  \c NULL is returned if no view is associated.
    *
    *  @see     setAssociatedView(), getAssociatedView() const
    */
   virtual View* getAssociatedView() = 0;

   /**
    *  Returns the view associated with the plot set.
    *
    *  @return  A const pointer to the view currently associated with the plot
    *           set.  \c NULL is returned if no view is associated.
    *
    *  @see     setAssociatedView(), getAssociatedView()
    */
   virtual const View* getAssociatedView() const = 0;

   /**
    *  Renames a plot in the plot set with a user-defined name.
    *
    *  This method prompts the user to select a new name for the given plot and
    *  renames the plot.
    *
    *  @param   pPlot
    *           The plot to rename.  This method does nothing if \c NULL is
    *           passed in.
    *
    *  @return  The new plot name, which is guaranteed to be unique for all
    *           plots in the plot set.  An empty string is returned if the user
    *           cancels the process.
    *
    *  @notify  This method will notify View::signalRenamed() with
    *           boost::any<std::string> containing the new plot name chosen by
    *           the user.
    *
    *  @see     renamePlot(PlotWidget*, const std::string&)
    */
   virtual std::string renamePlot(PlotWidget* pPlot) = 0;

protected:
   /**
    *  The plot set should be destroyed only if it is not owned by a parent
    *  widget by calling DesktopServices::deletePlotSet().
    */
   virtual ~PlotSet()
   {}
};

#endif
