/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONITEM_H
#define SESSIONITEM_H

#include <list>
#include <string>
#include <vector>

#include "ContextMenuAction.h"

class QIcon;
class SessionItemSerializer;
class SessionItemDeserializer;

/**
 *  The base class for all objects in a session.
 *
 *  The SessionItem class is a base class for all objects that are included in
 *  a session.
 */
class SessionItem
{
public:
   /**
    *  Returns a unique ID for the session item.
    *
    *  The session item ID is automatically assigned when the item is created.
    *  It is unique for all session items in the current session.
    *
    *  @return  The unique ID for the session item as a text string.  The
    *           string has the format {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    *           where 'x' is a hexidecimal digit.
    */
   virtual const std::string& getId() const = 0;

   /**
    *  Returns the icon associated with the session item.
    *
    *  @return  The item icon.
    */
   virtual const QIcon& getIcon() const = 0;

   /**
    *  Returns the session item name.
    *
    *  The session item name is the full name of the session item.  This can be
    *  a longer name that is used to help uniquely identify the item.  To get
    *  the name that is displayed to the user, call getDisplayName() instead.
    *
    *  @return  The session item name.
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Returns the session item name that should be displayed to the user.
    *
    *  The display name can be used to present a shorter name to the user such
    *  as a filename that does not include the full path.  To get the full name
    *  of the session item, call getName() instead.
    *
    *  @param   fullName
    *           If set to \c true and no display name has been set, then the
    *           full session item name is returned instead.
    *
    *  @return  The session item display name.  If no display name has been set
    *           and the \em fullName parameter is set to \c true, then the full
    *           item name is returned, which is equivalent to calling getName().
    */
   virtual const std::string& getDisplayName(bool fullName = false) const = 0;

   /**
    *  Returns the additional text about the session item that can be displayed
    *  to the user.
    *
    *  @return  The text string containing additional information about this
    *           session item that can be displayed to the user.
    */
   virtual const std::string& getDisplayText() const = 0;

   /**
    *  Returns the context menu actions available for this session item.
    *
    *  @return  The list of context menu actions that should be displayed when
    *           the user right-clicks on a widget displaying or containing the
    *           session item.
    *
    *  @see     ContextMenuAction
    */
   virtual std::list<ContextMenuAction> getContextMenuActions() const = 0;

   /**
    *  Returns whether the display name and display text are automatically
    *  updated when a filename is set as the item name.
    *
    *  If this method returns \c true and the item name is updated to a
    *  filename, the display name is automatically set to the base filename and
    *  extension without the full path.  The display text is automatically set
    *  to the full path and filename, which is the same value as the item name
    *  returned by getName().
    *
    *  If this method returns \c true and the item name is updated to a name
    *  that is not a filename, the display name and display text are set to
    *  empty text.
    *
    *  If this method returns \c false, the display name and display text are
    *  not affected when the item name is updated.
    *
    *  @return  Returns \c true if the display name and display text are
    *           automatically updated when a filename is set as the item name;
    *           otherwise returns \c false.  By default, this method returns
    *           \c true when a SessionItem is first created.
    *
    *  @see     getDisplayName(), getDisplayText()
    */
   virtual bool hasFilenameDisplay() const = 0;

   /**
    *  Returns the Properties plug-in names whose widgets should comprise the
    *  available tabs of the properties dialog for this session item.
    *
    *  @return  Returns the Properties plug-in names for this session item.
    */
   virtual std::vector<std::string> getPropertiesPages() const = 0;

   /**
    *  Retrieves session save validity.
    *
    *  Method returns whether or not the item will be included when saving a
    *  session.
    *
    *  @return  Returns \c true if the item will be saved in a session, or
    *           \c false otherwise.
    */
   virtual bool isValidSessionSaveItem() const = 0;

   /**
    *  Saves the SessionItem as part of a full session save.
    *
    *  This method will normally only be called by the SessionManager during a 
    *  session save. Every concrete type that inherits from SessionItem should 
    *  provide its own implementation for this method. If a SessionItem needs
    *  to be recreated on session load, but does not have any state information
    *  that it needs to save, it should call serialize(NULL,0) on the serializer.
    *  If a SessionItem does not need to be created on session load, it should
    *  simply return true from this method. This method will only be called on
    *  session items that exist at the time of session serialization.
    *
    *  @param serializer
    *           The object to use to save the item as part of the current 
    *           session.
    *
    *  @return  True if the item was successfully saved and false otherwise.
    */
   virtual bool serialize(SessionItemSerializer& serializer) const = 0;

   /**
    *  Restores the SessionItem from a saved session.
    *
    *  This method will normally only be called by the SessionManager during a 
    *  session restore operation. Every concrete type that inherits from SessionItem 
    *  should provide its own implementation for this method.
    *
    *  @param deserializer
    *           The object to use to restore the item from a saved session
    *
    *  @return  True if the item was successfully restored and false otherwise.
    */
   virtual bool deserialize(SessionItemDeserializer& deserializer) = 0;

protected:
   /**
    *  The session item should be destroyed by calling one of the destruction
    *  methods to destroy the specific subclass instance.
    */
   virtual ~SessionItem() {}
};

#endif
