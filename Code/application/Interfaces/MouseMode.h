/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MOUSEMODE_H
#define MOUSEMODE_H

#include <string>

class QAction;

/**
 *  Provides custom handling of mouse events in a view.
 *
 *  A mouse mode is a special mode used in a view to process mouse events.  The
 *  mouse mode consists of a name, a cursor, a cursor mask, and a cursor hot spot.
 *  All values are specified when the mouse mode is created and this class
 *  provides access to the mouse mode object itself and its name.
 *
 *  A mouse mode also contains an optional Qt action that is added to the main
 *  mouse mode action group when the mode is added to a view.  This allows the
 *  action to be automatically toggled off when another mouse mode action in the
 *  group is activated and all other actions will be toggled off when this mouse
 *  mode is activated.
 *
 *  @see     DesktopServices::createMouseMode(), View::setMouseMode()
 */
class MouseMode
{
public:
   /**
    *  Retrieves mouse mode name.
    *
    *  @param   modeName
    *           A string populated with the mouse mode name.
    */
   virtual void getName(std::string& modeName) const = 0;

   /**
    *  Retrieves the action associated with the mouse mode.
    *
    *  If the mouse mode has an associated action, it is added to the main mouse
    *  mode action group when the mode is added to a view.  This allows the action
    *  to be automatically toggled off when another mouse mode action in the group
    *  is activated and all other actions will be toggled off when this mouse mode
    *  is activated.
    *
    *  See DesktopServices::createMouseMode() for details on the default mouse
    *  modes contained in the main mouse mode action group.
    *
    *  @return  The associated Qt action that is included in the main mouse mode
    *           action group.  NULL is returned if the mouse mode is not included
    *           in the main mouse mode action group.
    */
   virtual QAction* getAction() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteMouseMode.
    */
   virtual ~MouseMode() {}
};

#endif
