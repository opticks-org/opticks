/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include "ColorType.h"
#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>

class Axis;
class Classification;
class Font;
class PlotSet;
class PlotView;
class QImage;
class QWidget;

/**
 *  A widget to display plot and associated information.
 *
 *  The plot widget contains a plot view and several additional widgets to
 *  display information about the plot.  The plot widget contains a title, the
 *  plot view, classification markings, axes, and a legend.  The axes are
 *  displayed only for Cartesian plots and not polar plots.
 *
 *  When the user right-clicks in the plot widget, a \ref contextmenus
 *  "context menu" is invoked allowing the user to change the display
 *  characteristics of the widget.
 *
 *  @warning   The PlotView::signalAboutToShowContextMenu() signal is not
 *             emitted when the user right-clicks in the plot widget.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The plot is renamed.
 *  - The plot is deleted.
 *  - The background color changes.
 *  - The classification text label position changes.
 *  - The classification text label color changes.
 *  - The classification text label font changes.
 *  - The classification text label text changes.
 *  - The organization text label position changes.
 *  - The organization text label color changes.
 *  - The organization text label font changes.
 *  - The organization text label text changes.
 *  - The title text changes.
 *  - The title text font changes.
 *  - The legend background color changes.
 *  - The user right-clicks in the widget to invoke a context menu.
 *  - Everything else documented in Subject.
 *
 *  @see     PlotView
 */
class PlotWidget : public SessionItem, public Subject
{
public:
   /**
    *  Emitted with boost::any<ContextMenu*> when the user right-clicks to
    *  invoke a context menu.
    *
    *  This signal provides a means by which an object can be notified when a
    *  context menu is invoked by the user clicking inside a plot widget.  To
    *  receive notification for when a context menu is invoked when the user
    *  clicks on any session item, attach to the
    *  DesktopServices::signalAboutToShowContextMenu() signal instead.
    *
    *  This signal is emitted after getContextMenuActions() is called and
    *  after the DesktopServices::signalAboutToShowContextMenu() signal is
    *  emitted, but before the context menu is shown to give attached objects a
    *  chance to add or modify the context menu that will be displayed to the
    *  user.
    *
    *  The ContextMenu pointer value is guaranteed to be non-\c NULL.  The
    *  session items vector in the context menu contains the plot widget and
    *  the plot view.
    *
    *  @see     \ref callingsequence "Context menu calling sequence"
    */
   SIGNAL_METHOD(PlotWidget, AboutToShowContextMenu)

   /**
    *  @copydoc SessionItem::getDisplayName()
    *
    *  @default The default implementation returns the display name of the plot
    *           widget.  If the display name is empty, the display name of the
    *           PlotView contained in the widget is returned instead.
    */
   virtual const std::string& getDisplayName() const = 0;

   /**
    *  @copydoc SessionItem::getDisplayText()
    *
    *  @default The default implementation returns the display text of the plot
    *           widget.  If the display text is empty, the display text of the
    *           PlotView contained in the widget is returned instead.
    */
   virtual const std::string& getDisplayText() const = 0;

   /**
    *  @copydoc SessionItem::getContextMenuActions()
    *
    *  @default The default implementation returns the context menu actions
    *           listed \ref plotwidget "here".  The default actions can be
    *           removed or additional actions can be added by attaching to the
    *           signalAboutToShowContextMenu() signal.
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Returns the plot widget as a Qt widget.
    *
    *  This method returns the plot widget as a Qt widget, which can then be used
    *  to add the plot widget to a custom dialog or widget layout.
    *
    *  @return  A pointer to the plot widget.
    */
   virtual const QWidget* getWidget() const = 0;

   /**
    *  Returns the plot widget as a Qt widget.
    *
    *  This method returns the plot widget as a Qt widget, which can then be used
    *  to add the plot widget to a custom dialog or widget layout.
    *
    *  @return  A pointer to the plot widget.
    */
   virtual QWidget* getWidget() = 0;

   /**
    *  Returns the plot set that contains this plot widget.
    *
    *  @return  A pointer to the plot set.  \b NULL is returned if this plot
    *           widget is not contained in a plot set.
    *
    *  @see     PlotSet
    */
   virtual PlotSet* getPlotSet() const = 0;

   /**
    *  Returns the plot view inside of the widget.
    *
    *  @return  A pointer to the plot view.
    *
    *  @see     PlotView
    */
   virtual PlotView* getPlot() const = 0;

   /**
    *  Sets the background color of the plot widget.
    *
    *  This method sets the background color of the plot widget, but does not
    *  set the background color of the plot area or the legend.
    *
    *  @param   backgroundColor
    *           The new plot widget background color, which must be valid.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @see     setLegendBackgroundColor(), PlotView::setBackgroundColor()
    */
   virtual void setBackgroundColor(const ColorType& backgroundColor) = 0;

   /**
    *  Returns the background color of the plot widget.
    *
    *  The background color of the plot widget is separate from the background
    *  color of the plot area and legend.
    *
    *  @return  The current background color of the plot widget.
    *
    *  @see     getLegendBackgroundColor(), PlotView::getBackgroundColor()
    */
   virtual ColorType getBackgroundColor() const = 0;

   /**
    *  Sets the position of the classification label.
    *
    *  @param   ePosition
    *           The position of the label.
    *
    *  @notify  This method will notify Subject::signalModified() if the
    *           position is different from the previous position.
    *
    *  @see     PositionType
    */
   virtual void setClassificationPosition(PositionType ePosition) = 0;

   /**
    *  Gets the position of the classification label.
    *
    *  @return  The position of the label.
    */
   virtual PositionType getClassificationPosition() const = 0;

   /**
    *  Sets the classification text for this plot widget.
    *
    *  This method replaces the current classification text of the widget to
    *  the string populated by Classification::getClassificationText().
    *
    *  @param   pClassification
    *           A pointer to the classification object from which to set the
    *           widget's text markings.  The classification text is cleared if
    *           the given classification object pointer is \b NULL or if the
    *           object is invalid.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @see     Classification, Classification::getClassificationText(),
    *           Classification::isValid(),
    *           setClassificationText(const std::string&)
    */
   virtual void setClassificationText(const Classification* pClassification) = 0;

   /**
    *  Sets the classification text for this plot widget.
    *
    *  This method replaces the current classification text of the widget to
    *  the text in the given string.
    *
    *  @param   classificationText
    *           The new classification text for this plot widget.  The
    *           classification text is cleared if the given string is empty.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @see     setClassificationText(const Classification*)
    */
   virtual void setClassificationText(const std::string& classificationText) = 0;

   /**
    *  Returns a text string containing the classification text of the plot
    *  widget.
    *
    *  @return  A string containing the classification text.
    */
   virtual std::string getClassificationText() const = 0;

   /**
    *  Sets the font for the classification text.
    *
    *  @param   font
    *           The new classification font.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setClassificationFont(const Font& font) = 0;

   /**
    *  Returns read-only access to the classification text font.
    *
    *  @return  The current classification text font.  To modify the font
    *           values, call setClassificationFont() instead.
    */
   virtual const Font& getClassificationFont() const = 0;

   /**
    *  Sets the classification text label color.
    *
    *  @param   classificationColor
    *           The new classification text label color.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setClassificationColor(const ColorType& classificationColor) = 0;

   /**
    *  Returns the color of the classification text label.
    *
    *  @return  The classification text label color.
    */
   virtual ColorType getClassificationColor() const = 0;

   /**
    *  Sets the position of the organization label.
    *
    *  @param   ePosition
    *           The position of the label.
    *
    *  @notify  This method will notify Subject::signalModified() if the
    *           position is different from the previous position.
    *
    *  @see     PositionType
    */
   virtual void setOrganizationPosition(PositionType ePosition) = 0;

   /**
    *  Gets the position of the organization label.
    *
    *  @return  The position of the label.
    *
    *  @see     PositionType
    */
   virtual PositionType getOrganizationPosition() const = 0;

   /**
    *  Sets the organization text label.
    *
    *  @param   strOrganization
    *           The text label.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setOrganizationText(const std::string& strOrganization) = 0;

   /**
    *  Gets the organization text label.
    *
    *  @return  The text label.
    */
   virtual std::string getOrganizationText() const = 0;

   /**
    *  Sets the font for the organization text.
    *
    *  @param   font
    *           The new organization font.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setOrganizationFont(const Font& font) = 0;

   /**
    *  Returns read-only access to the organization text font.
    *
    *  @return  The current organization text font.  To modify the font
    *           values, call setOrganizationFont() instead.
    */
   virtual const Font& getOrganizationFont() const = 0;

   /**
    *  Sets the organization label color.
    *
    *  @param   clrOrganization
    *           The new label color.
    *
    *  @notify  This method will notify Subject::signalModified().
    */
   virtual void setOrganizationColor(const ColorType& clrOrganization) = 0;

   /**
    *  Gets the organization label color.
    *
    *  @return  The label color.
    */
   virtual ColorType getOrganizationColor() const = 0;

   /**
    *  Sets the plot title text.
    *
    *  The plot title appears above the plot view and below the classification
    *  and organization text.
    *
    *  @param   title
    *           The new title text.
    *
    *  @notify  This method will notify Subject::signalModified() if the given
    *           title is different than the current title.
    */
   virtual void setTitle(const std::string& title) = 0;

   /**
    *  Returns the plot title text.
    *
    *  @return  The plot title.
    */
   virtual std::string getTitle() const = 0;

   /**
    *  Sets the font for the title text.
    *
    *  @param   font
    *           The new title font.
    *
    *  @notify  This method will notify Subject::signalModified() if the given
    *           font is different than the current font.
    */
   virtual void setTitleFont(const Font& font) = 0;

   /**
    *  Returns read-only access to the title text font.
    *
    *  @return  The current title text font.  To modify the font values, call
    *           setTitleFont() instead.
    */
   virtual const Font& getTitleFont() const = 0;

   /**
    *  Toggles the display state of an axis widget.
    *
    *  This method shows or hides an axis widget if a CartesianPlot is
    *  displayed.  This method does nothing if a PolarPlot is displayed.
    *
    *  @param   axis
    *           The position for which to show or hide the axis.
    *  @param   bShow
    *           Set this parameter to \b true to show the axis or to
    *           \b false to hide the axis.
    */
   virtual void showAxis(AxisPosition axis, bool bShow) = 0;

   /**
    *  Queries the display state of an axis widget.
    *
    *  This method queries the display state of an axis widget if a
    *  CartesianPlot is displayed.
    *
    *  @param   axis
    *           The position for which to query the axis display state.
    *
    *  @return  Returns \b true if the axis in the given position is shown, or
    *           returns \b false if the axis is hidden or if a PolarPlot is
    *           displayed.
    */
   virtual bool isAxisShown(AxisPosition axis) const = 0;

   /**
    *  Retrieves an axis widget.
    *
    *  @param   axis
    *           The position for which to get the axis.
    *
    *  @return  Returns a pointer to the axis widget in the given position.
    *           This method always returns \c NULL if a PolarPlot is displayed.
    */
   virtual Axis* getAxis(AxisPosition axis) const = 0;

   /**
    *  Sets the display state of the plot legend.
    *
    *  @param   bShow
    *           Set this value to \b true to show the legend or to \b false to
    *           hide the legend.
    */
   virtual void showLegend(bool bShow) = 0;

   /**
    *  Queries the display state of the plot legend.
    *
    *  @return  Returns \b true if the legend is displayed, or \b false if the
    *           legend is hidden.
    */
   virtual bool isLegendShown() const = 0;

   /**
    *  Sets the background color of the legend.
    *
    *  This method sets the background color of the legend, which is separate
    *  from the background color of the plot widget and plot area.
    *
    *  @param   backgroundColor
    *           The new legend background color, which must be valid.
    *
    *  @notify  This method will notify Subject::signalModified().
    *
    *  @see     setBackgroundColor(), PlotView::setBackgroundColor()
    */
   virtual void setLegendBackgroundColor(const ColorType& backgroundColor) = 0;

   /**
    *  Returns the background color of the legend.
    *
    *  The background color of the legend is separate from the background
    *  color of the plot widget and plot area.
    *
    *  @return  The current background color of the legend.
    *
    *  @see     getBackgroundColor(), PlotView::getBackgroundColor()
    */
   virtual ColorType getLegendBackgroundColor() const = 0;

   /**
    *  Retrieves an image of the plot widget.
    *
    *  This method retrieves the image data for the plot widget.  This is
    *  similar to taking a screen snapshot of the widget.
    *
    *  @param   image
    *           A Qt image reference that is populated with the current plot
    *           widget image.  If a null image is passed in, the size of the
    *           image will be equal to the size of the plot widget in screen
    *           pixels. Otherwise, the widget will be scaled to the image size
    *           when generating the image data.
    *
    *  @return  Returns \b true if a valid image was successfully retrieved,
    *           otherwise returns \b false.
    */
   virtual bool getCurrentImage(QImage& image) = 0;

   /**
    *  Sends the current widget image to the printer.
    *
    *  @param   bPrintDialog
    *           Set this value to \b true to display the print options dialog
    *           before printing.
    */
   virtual void print(bool bPrintDialog = true) = 0;

protected:
   /**
    *  This object should be destroyed by calling PlotSet::deletePlot() or
    *  DesktopServices::deletePlotWidget().
    */
   virtual ~PlotWidget() {}
};

#endif
