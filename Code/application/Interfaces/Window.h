/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>

/**
 *  The basic object in which to display data to the user.
 *
 *  This class is a base class for displaying data to the user.  The main
 *  application can create and manage several types of windows.  Every
 *  window has a name and a type that is used to uniquely identify itself
 *  within the application.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: setName().
 *  - Everything else documented in Subject.
 *
 *  @see     DesktopServices
 */
class Window : public SessionItem, public Subject
{
public:
   /**
    *  This interface is implemented by objects wishing to selectively accept drop events.
    *
    *  @see signalSessionItemDropped()
    */
   class SessionItemDropFilter
   {
   public:
      /**
       *  Accept the drop of the SessionItem.
       *
       *  @param pItem
       *         The SessionItem in the drop event.
       *  @return True to accept the drop, false otherwise.
       */
      virtual bool accept(SessionItem *pItem) const = 0;

   protected:
      virtual ~SessionItemDropFilter() {}
   };

   /**
    *  Emitted with any<SessionItem*> for each SessionItem dropped onto the window.
    *
    *  If a single signal with all SessionItems involved in a dropEvent() is needed, see
    *  signalSessionItemsDropped().
    */
   SIGNAL_METHOD(Window, SessionItemDropped)

   /**
    *  Emitted with any<vector<SessionItem*> > when one or more SessionItems are dropped onto the window.
    *  All SessionItems are included in this signal. See signalSessionItemDropped() if a separate signal for each
    *  SessionItem is needed.
    */
   SIGNAL_METHOD(Window, SessionItemsDropped)

   /**
    *  Sets the window name.
    *
    *  @param   windowName
    *           The new name for the window.  Cannot be empty.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void setName(const std::string& windowName) = 0;

   /**
    *  Returns the window type.
    *
    *  @return  The window type.
    *
    *  @see     WindowType
    */
   virtual WindowType getWindowType() const = 0;

   /**
    *  Enable dropping of SessionItems via drag and drop.
    *
    *  Windows can accept drag and drop events by calling this method.
    *  Once enabled signalSessionItemDropped() is emmitted for each
    *  SessionItem dropped into this Window. If the caller wishes to
    *  selectively accept drops, the SessionItemDropFilter parameter should
    *  be non-null.
    *
    *  @param pFilter
    *         If this is NULL, all SessionItems will be accepted. If non-NULL,
    *         the filter will be added to the filter list. Each filter registered
    *         will be checked for acceptance of the drop until one accepts. The caller
    *         maintains ownership of the filter.
    */
   virtual void enableSessionItemDrops(SessionItemDropFilter *pFilter = NULL) = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteWindow.
    */
   virtual ~Window() {}
};

#endif
