/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETTABLESESSIONITEM_H
#define SETTABLESESSIONITEM_H

#include "SessionItem.h"

class SessionItemId;

/**
 *  Provides access to set attribute values of a session item.
 *
 *  This class provides a generic session item where the various attribute
 *  values can be set.  Calling ObjectFactory::createObject("SettableSessionItem")
 *  creates an instance of this class.
 *
 *  SettableSessionItem is used internally and is typically not used by plug-in
 *  developers.
 *
 *  @see    SessionItem, SettableSessionItemExt1
 */
class SettableSessionItem : public SessionItem
{
public:
   /**
    *  Sets the id of the session item.
    *
    *  This method will set the id of the item. It will only work if getId has
    *  not yet been called.
    *
    *  @param   id
    *           The new id for the SessionItem.
    *
    *  @return   Returns \c true if the id was successfully set, or \c false otherwise.
    */
   virtual bool setId(const SessionItemId& id) = 0;

   /**
    *  Associates an icon with the session item.
    *
    *  The icon can be used by widgets displaying the session item to
    *  differentiate between mutliple session item types, or to uniquely
    *  identify this session item.
    *
    *  @param   icon
    *           The icon to associate with the session item.
    */
   virtual void setIcon(const QIcon& icon) = 0;

   /**
    *  Sets the full session item name.
    *
    *  @param   name
    *           The session item name.
    *
    *  @see     setDisplayName()
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Sets the session item name that should be displayed to the user.
    *
    *  The display name can optionally be used to present a shorter name to the
    *  user.  If the display name is not set, the full session item name may be
    *  displayed to the user instead.
    *
    *  @param   displayName
    *           The name that should be displayed to the user for this session
    *           item.
    *
    *  @see     setName()
    */
   virtual void setDisplayName(const std::string& displayName) = 0;

   /**
    *  Sets additional text about the session item to be displayed to the user.
    *
    *  The display text is an optional means by which additional information
    *  about this session item can be displayed to the user.
    *
    *  @param   displayText
    *           The text string containing additional information about this
    *           session item to display to the user.
    */
   virtual void setDisplayText(const std::string& displayText) = 0;

   /**
    *  Adds a single context menu action to the list of available actions for
    *  this session item.
    *
    *  This method adds an action to the list of available actions that are
    *  avaiable when the user right-clicks on a widget that displays or
    *  contains the session item.  This method is typically called with an
    *  action that should be available regardless of the current state of the
    *  session or the item.
    *
    *  @param   menuAction
    *           A context menu action that should be displayed when the user
    *           right-clicks on a widget that displays or contains the session
    *           item.
    *
    *  @see     setContextMenuActions()
    */
   virtual void addContextMenuAction(const ContextMenuAction& menuAction) = 0;

   /**
    *  Sets available context menu actions for this session item.
    *
    *  This method sets actions that are available to the user when
    *  right-clicking on a widget that displays or contains the session item.
    *  This method is typically called with actions that should be available
    *  regardless of the current state of the session or the item.
    *
    *  @param   actions
    *           A list of context menu actions that should be displayed when
    *           the user right-clicks on a widget that displays or contains the
    *           session item.
    *
    *  @see     addContextMenuAction()
    */
   virtual void setContextMenuActions(const std::list<ContextMenuAction>& actions) = 0;

   /**
    *  Sets whether the display name and display text are automatically updated
    *  when a filename is set as the item name.
    *
    *  @param   bFilenameDisplay
    *           If this parameter is \b true, the display name and display text
    *           will automatically be updated when setName() is called with a
    *           filename.  If this parameter is \b false, the display name and
    *           display text are not affected when setName() is called.
    *
    *  @see     hasFilenameDisplay()
    */
   virtual void setFilenameDisplay(bool bFilenameDisplay) = 0;

   /**
    *  Adds a Properties plug-in name to the vector of available properties
    *  pages for this session item.
    *
    *  This method adds the name of a single Properties plug-in to the vector
    *  of available properties plug-ins whose widgets are displayed to the user
    *  in the properties dialog for this session item.
    *
    *  @param   plugInName
    *           The Properties plug-in name to add to the vector of pages that
    *           should be displayed in the properties dialog for this session
    *           item.
    *
    *  @see     setPropertiesPages()
    */
   virtual void addPropertiesPage(const std::string& plugInName) = 0;

   /**
    *  Removes a Properties plug-in name from the vector of available
    *  properties pages for this session item.
    *
    *  This method removed the name of a single Properties plug-in from the
    *  vector of available properties plug-ins whose widgets are displayed to
    *  the user in the properties dialog for this session item.
    *
    *  @param   plugInName
    *           The Properties plug-in name to remove from the vector of pages
    *           that should be displayed in the properties dialog for this
    *           session item.
    *
    *  @see     addPropertiesPage()
    */
   virtual void removePropertiesPage(const std::string& plugInName) = 0;

   /**
    *  Sets the available properties pages for this session item.
    *
    *  This method sets the names of all Properties plug-ins whose widgets are
    *  displayed to the user in the properties dialog for this session item.
    *
    *  @param   plugInNames
    *           A vector of Properties plug-in names whose widgets should be
    *           displayed in the properties dialog for this item.
    *
    *  @see     addPropertiesPage()
    */
   virtual void setPropertiesPages(const std::vector<std::string>& plugInNames) = 0;

protected:
   /**
    *  This object should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~SettableSessionItem() {}
};

/**
* Extends capability of the SettableSessionItem interface.
*
* This class provides additional capability for the SettableSessionItem interface
* class.  A pointer to this class can be obtained by performing a dynamic cast
* on a pointer to SettableSessionItem or any of its subclasses.
*
* @warning A pointer to this class can only be used to call methods contained
*          in this extension class and cannot be used to call any methods in
*          SettableSessionItem or any of its subclasses.
*/
class SettableSessionItemExt1
{
public:
   /**
    *  Sets Session Save validity.
    *
    *  Method sets whether or not the item will be included in a Session Save.
    *
    *  @param   isValid
    *           Session Save validity of the item.
    */
   virtual void setValidSessionSaveItem(bool isValid) = 0;

protected:
   /**
    *  This object should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~SettableSessionItemExt1() {}
};

#endif