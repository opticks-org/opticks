/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>

class SessionItem;
class QWidget;

/**
 *  Interface for properties plug-ins.
 *
 *  %Properties plug-ins will be displayed to the user when opening the
 *  application properties dialog.  This interface can be implemented by a
 *  plug-in to get its custom properties widgets to appear in the application
 *  properties dialog.  The application properties dialog is modal and will
 *  instantiate all properties plug-ins when shown and will destroy all
 *  properties plug-ins when the dialog is closed.
 *
 *  @see PropertiesShell, PropertiesQWidgetWrapper
 */
class Properties
{
public:
   /**
    *  Returns the widget that will be displayed to the user.
    *
    *  The widget will be displayed when the user selects the applicable tab in
    *  the properties dialog.  It is only instantiated once per instance of
    *  this plug-in.  The widget will be owned by the plug-in and should be
    *  destroyed when the plug-in is destroyed.  Please note, that the
    *  properties dialog in which the widget will be displayed in is resizable,
    *  so ensure that the Qt layout for the widget resizes properly.
    *
    *  @warning The QWidget* returned by this method should never be
    *           instantiated in the constructor of this plug-in.  When the
    *           application is run in batch mode, it will attempt to create
    *           this plug-in and if this plug-in attempts to create a QWidget*
    *           in its constructor, the application will crash.
    *
    *  @return  Returns the widget that will be displayed to the user.
    */
   virtual QWidget* getWidget() = 0;

   /**
    *  Returns the name of the properties.
    *
    *  This method is called by the properties dialog to get the name to
    *  display on the tab containing the widget returned by getWidget().
    *
    *  @return  Returns the properties name that is displayed to the user.
    */
   virtual const std::string& getPropertiesName() const = 0;

   /**
    *  Initializes the widget to the properties of a given session item.
    *
    *  This method is called by the properties dialog for the widget to
    *  initialize its child widgets based on the current state of the given
    *  session item.
    *
    *  This method is guaranteed to only be called once per instance of the
    *  class.
    *
    *  @param   pSessionItem
    *           The session item for which to initialize the widget's values.
    *
    *  @return  Returns \c true if the widget was successfully initialized from
    *           the given session item; otherwise returns \c false.  If this
    *           method returns \c false, the widget is not added to the
    *           properties dialog.
    */
   virtual bool initialize(SessionItem* pSessionItem) = 0;

   /**
    *  Applies the changes that the user made in the widget.
    *
    *  This method is called when the user clicks the OK or Apply buttons in
    *  the properties dialog.  It applies the changes in the widget by calling
    *  applyChanges() on the templated class.  The applyChanges() method of the
    *  templated class should return \c true if all changes were successfully
    *  applied to the object.  It should also return \c true if no updates need
    *  to be made.
    *
    *  The values modified by the user while the QWidget is displayed should be
    *  applied to the session item for which the widget was initialized in the
    *  initialize() method.
    *
    *  @return  This method returns \c true if the changes were successfully
    *           applied to the session item or if no changes need to be made.
    *           If \c false is returned, the widget will be activated in the
    *           properties dialog if necessary and the dialog will not be
    *           closed if the OK button was clicked.
    */
   virtual bool applyChanges() = 0;

protected:
   /**
    *  Since the Properties interface is usually used in conjunction with the
    *  PlugIn interface, this should be destroyed by casting to the PlugIn
    *  interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Properties() {}
};

#endif
