/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESSHELL_H
#define PROPERTIESSHELL_H

#include "PlugInShell.h"
#include "Properties.h"

#include <string>

class QWidget;
class SessionItem;

/**
 *  \ingroup ShellModule
 *  A base class for plug-in shells or plug-in instances.
 *
 *  This class provides a default implementation of the Properties plug-in
 *  interface.  If a properties plug-in needs to implement a QWidget, try
 *  subclassing PropertiesQWidgetWrapper instead of PropertiesShell.  However,
 *  if the properties plug-in should simply wrap an existing QWidget, subclass
 *  PropertiesShell.
 *
 *  @see        Properties, PropertiesQWidgetWrapper
 */
class PropertiesShell : public Properties, public PlugInShell
{
public:
   /**
    *  Creates the properties plug-in.
    *
    *  The constructor sets the plug-in type to
    *  PlugInManagerServices::PropertiesType().
    *
    *  @see     getType()
    */
   PropertiesShell();

   /**
    *  Destroys the properties plug-in.
    */
   virtual ~PropertiesShell();

   /**
    *  @copydoc Properties::getWidget()
    *
    *  @default The default implementation calls createWidget() and stores and
    *           returns the returned widget pointer.  Successive calls to this
    *           method will simply return the stored widget pointer.
    */
   QWidget* getWidget();

   /**
    *  @copydoc Properties::getPropertiesName()
    *
    *  @default The default implementation returns the value provided to
    *           setPropertiesName().  If setPropertiesName() has not been
    *           called, this method returns an empty string.
    */
   const std::string& getPropertiesName() const;

   /**
    *  @copydoc Properties::initialize()
    *
    *  @default The default implementation sets the member pointer that can be
    *           retrieved by calling getSessionItem() and returns \c true.
    */
   bool initialize(SessionItem* pSessionItem);

protected:
   /**
    *  Creates the properties widget.
    *
    *  This method must be implemented in subclasses to create the QWidget.
    *  Upon creation, the widget should be populated with default values.
    *  Initialization of the widget will occur in initialize(), which is called
    *  after this method.
    *
    *  This method is guaranteed to be called only once per instance of this
    *  plug-in.
    *
    *  @return  The created properties widget.
    */
   virtual QWidget* createWidget() = 0;

   /**
    *  Sets the properties name that will be returned from getPropertiesName().
    *
    *  @param   name
    *           The properties name that will appear on a tab in the properties
    *           dialog.
    */
   void setPropertiesName(const std::string& name);

   /**
    *  Returns the session item for which the widget is initialized.
    *
    *  This method provides a means to retrieve the session item to which the
    *  widget is initialized.  The returned pointer can be used when appying
    *  changes back to the session item.
    *
    *  @return  The session item to which changes should be applied.
    *
    *  @see     initialize()
    */
   SessionItem* getSessionItem() const;

private:
   QWidget* mpWidget;
   std::string mName;
   SessionItem* mpSessionItem;
};

#endif
