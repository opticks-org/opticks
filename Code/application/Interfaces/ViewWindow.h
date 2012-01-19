/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H

#include "Window.h"
#include "TypesFile.h"

#include <string>

class QWidget;
class View;

/**
 *  A window that displays a view.
 *
 *  A view window is an abstract base class for windows to display a single
 *  widget that can be either a View object or a generic QWidget.  Since
 *  the window contains a single widget, the current image can be printed
 *  as well.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: createView(), setWidget()
 *  - Everything documented in Window.
 *
 *  @see     Window
 */
class ViewWindow : public Window
{
public:
   /**
    *  Emitted just before the widget is set into the window.
    *
    *  @see     setWidget()
    */
   SIGNAL_METHOD(ViewWindow, AboutToSetWidget)

   /**
    *  Emitted after the widget is set into the window with boost::any<QWidget*>
    *  containing a pointer to the widget that was set.
    *
    *  @see     setWidget()
    */
   SIGNAL_METHOD(ViewWindow, WidgetSet)

   /**
    *  Creates the view based on a given type.
    *
    *  This method creates a view and sets it as the display widget in the
    *  window.
    *
    *  @param   viewName
    *           The name for the view.  Depending on the view type, the name
    *           may appear in the view's title bar.
    *  @param   viewType
    *           The type of the view to be created.
    *
    *  @return  A pointer to the created view.  NULL is returned if the view
    *           is already created or if an error occurred.
    *
    *  @notify  This method will notify signalAboutToSetWidget() just before the
    *           created view is set into the window and signalWidgetSet() just
    *           after the view is set with boost::any<QWidget*> containing a
    *           pointer to the view that is set into the window.
    *
    *  @see     ViewType, setWidget()
    */
   virtual View* createView(const std::string& viewName, const ViewType& viewType) = 0;

   /**
    *  Returns a pointer to the view.
    *
    *  @return  A pointer to the view displayed in the window.  NULL is returned
    *           if a view has not been created, or if the window is displaying a
    *           custom widget.
    */
   virtual View* getView() const = 0;

   /**
    *  Sets the widget to be displayed in the window.
    *
    *  This method sets the view window to display a custom Qt widget instead of
    *  a View.  This method does nothing for SpatialDataWindow and ProductWindow
    *  objects since by default, they contain a view.
    *
    *  @param   pWidget
    *           The Qt widget to set in the view window.  Cannot be NULL.
    *
    *  @notify  This method will notify signalAboutToSetWidget() just before the
    *           widget is set into the window and signalWidgetSet() just after
    *           the widget is set with boost::any<QWidget*> containing a pointer
    *           to the widget that is set into the window.
    */
   virtual void setWidget(QWidget* pWidget) = 0;

   /**
    *  Returns the current widget in the window.
    *
    *  @return  A pointer to the widget in the view window.  If a View is
    *           displayed, the widget displaying the View is returned.  NULL is
    *           returned if no widget exists or the widget cannot be accessed.
    *
    *  @see     setWidget()
    */
   virtual QWidget* getWidget() const = 0;

   /**
    *  Sends the current image displayed in the window to the printer.
    *
    *  @param   bSetupDialog
    *           Set this value to TRUE to display the print options dialog
    *           before printing.
    */
   virtual void print(bool bSetupDialog = false) = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~ViewWindow() {}
};

#endif
