/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDOBJECT_H
#define WIZARDOBJECT_H

#include "Subject.h"
#include "Serializable.h"

#include <string>
#include <vector>

class WizardItem;

/**
 *  The basic wizard type.
 *
 *  The WizardObject class is the basic class containing items representing plug-ins,
 *  desktop services, and specifically defined values that are sequenced together to
 *  define a unique process.  To use the WizardObject in a plug-in, the wizard items
 *  must already be defined.  The getItems() method returns a vector of WizardItem
 *  pointers containing the specific information about each item.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The wizard is populated from an XML file.
 *  - Everything documented in Subject.
 *
 *  @see       WizardObjectExt1, WizardItem
 */
class WizardObject : public Subject, public Serializable
{
public:
   /**
    *  Sets the wizard name.
    *
    *  @param   name
    *           The wizard name.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Returns the wizard name.
    *
    *  @return  The wizard name.
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Returns all items in the wizard.
    *
    *  This method returns pointers to all of the items stored in the wizard.  The order
    *  within the returned vector defines the execution order for each item.
    *
    *  @return  The vector of WizardItem pointers.
    *
    *  @see     WizardItem
    */
   virtual const std::vector<WizardItem*>& getItems() const = 0;

   /**
    *  Queries whether this wizard operates entirely in batch mode.
    *
    *  @return  TRUE if the wizard operates in batch mode, otherwise FALSE.
    *
    *  @see     WizardObject
    */
   virtual bool isBatch() const = 0;

   /**
    *  Returns the menu location and command name from which the wizard is
    *  executed.
    *
    *  The menu location of the wizard is returned as a string, which is
    *  formatted with bracket characters ([,]) to specify a toolbar, and 
    *  a slash (/) to indicate submenu levels.  The toolbar name must come
    *  first in the string.  A slash immediately following the toolbar name
    *  specifys that the submenus and command should be added to the default
    *  toolbar menu, which has the same name as the toolbar.  If a slash does
    *  not follow the toolbar name, the menus and command are added directly
    *  to the toolbar.  If the string does not include a toolbar name, the
    *  menus and command are added to the main menu bar.  The string cannot
    *  end with a slash, and the name after the last slash indicates the
    *  command name.  Examples of the menu string include
    *  "[Wizard]Import/MyLoadWizard", "&Tools/PlotManager", and
    *  "[Geo]/Georeference".
    *
    *  @return  The plug-in menu location and command name.  An empty string
    *           is returned if the wizard does not have a menu location,
    *           indicating that it should not be executed from the menus.
    */
   virtual const std::string& getMenuLocation() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~WizardObject() {}
};

/**
 * Extends capability of the WizardObject interface.
 *
 * This class provides additional capability for the WizardObject interface
 * class.  A pointer to this class can be obtained by performing a dynamic cast
 * on a pointer to WizardObject.
 *
 * @warning A pointer to this class can only be used to call methods contained
 *          in this extension class and cannot be used to call any methods in
 *          WizardObject.
 */
class WizardObjectExt1
{
public:
   /**
    * Returns the execution order of a wizard item.
    *
    * @param   pItem
    *          The wizard item to query for its execution order.
    *
    * @return  The one-based execution order for the wizard item.  If \c NULL
    *          is passed in or the given item does not exist in the wizard, a
    *          value of -1 is returned.
    */
   virtual int getExecutionOrder(WizardItem* pItem) const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~WizardObjectExt1()
   {}
};

#endif
