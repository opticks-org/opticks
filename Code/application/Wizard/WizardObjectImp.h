/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDOBJECTIMP_H
#define WIZARDOBJECTIMP_H

#include "SubjectImp.h"
#include "SerializableImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <string>
#include <vector>

class WizardItem;

/**
 *  The wizard implementation class.
 *
 *  The WizardObjectImp class is the implementation class for a wizard to contain items
 *  representing plug-ins, desktop services, and specifically defined values.  The items
 *  are sequenced together to define a unique process.  Items can be added and removed
 *  from the wizard using addItem(), removeItem(), and clear().  Each item has an
 *  execution order that defines where in the overall sequence the item is run.  The
 *  order for an item can be increased or decreased with increaseItemOrder() and
 *  decreaseItemOrder().
 *
 *  The wizard also contains a menu location for adding a menu command to the main menus
 *  and a batch mode state that indicates whether the entire wizard should execute in
 *  batch mode or interactive mode.
 *
 *  @see   WizardItem
 */
class WizardObjectImp : public SubjectImp
{
public:
   /**
    *  Constructs the wizard.
    */
   WizardObjectImp();

   /**
    *  Destroys the wizard element.
    *
    *  The destructor destroys the wizard element.  All wizard items are deleted.
    */
   ~WizardObjectImp();

   /**
    *  Sets the wizard name.
    *
    *  @param   name
    *           The wizard name.
    */
   void setName(const std::string& name);

   /**
    *  Returns the wizard name.
    *
    *  @return  The wizard name.
    */
   const std::string& getName() const;

   /**
    *  Adds a new item to the wizard.
    *
    *  @param   itemName
    *           The name for the item.
    *  @param   itemType
    *           The type of the item.  See the WizardItem documentation for the
    *           available item types.
    *
    *  @see     WizardItem
    *  @see     WizardObjectImp::removeItem
    */
   WizardItem* addItem(std::string itemName, std::string itemType);

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
   const std::vector<WizardItem*>& getItems() const;

   /**
    *  Removes and deletes a single item from the wizard.
    *
    *  @param   pItem
    *           The item to remove.
    *
    *  @return  TRUE if the item was successfully removed, otherwise FALSE.
    *
    *  @see     WizardObjectImp::addItem
    *  @see     WizardObjectImp::clear
    */
   bool removeItem(WizardItem* pItem);

   /**
    *  Removes and deletes all items from the wizard.
    *
    *  @see     WizardObjectImp::removeItem
    */
   void clear();

   /**
    *  Increases the execution order of an item by one.
    *
    *  This method increases the execution order of an item such that it is run later
    *  in the sequence of items.  If other items are connected to the output nodes of
    *  the given item, their execution order changes as well to preserve the flow of
    *  the wizard.  If the item is connected directly or indirectly to the last item
    *  in the wizard, the execution order does not change.
    *
    *  @param   pItem
    *           The item to increase the execution order.
    *
    *  @return  True if the order was successfully increased on the item, otherwise false.
    *
    *  @see     WizardObjectImp::decreaseItemOrder
    */
   bool increaseItemOrder(WizardItem* pItem);

   /**
    *  Decreases the execution order of an item by one.
    *
    *  This method decreases the execution order of an item such that it is run earlier
    *  in the sequence of items.  If other items are connected to the input nodes of
    *  the given item, their execution order changes as well to preserve the flow of
    *  the wizard.  If the item is connected directly or indirectly to the first item
    *  in the wizard, the execution order does not change.
    *
    *  @param   pItem
    *           The item to decrease the execution order.
    *
    *  @return  True if the order was successfully decreased on the item, otherwise false.
    *
    *  @see     WizardObjectImp::increaseItemOrder
    */
   bool decreaseItemOrder(WizardItem* pItem);

   /**
    *  Sets the batch mode state of the entire wizard.
    *
    *  @param   bBatch
    *           TRUE if the wizard should run in batch mode.  FALSE if the wizard should
    *           run in interactive mode.
    */
   void setBatch(bool bBatch);

   /**
    *  Returns the batch mode state of the entire wizard.
    *
    *  @return  TRUE if the wizard is set to run in batch mode.  FALSE if the wizard is
    *           set to run in interactive mode.
    */
   bool isBatch() const;

   /**
    *  Sets the menu location and command name from which the wizard is
    *  executed.
    *
    *  @param   location
    *           The menu location.  The string format should be the same
    *           as described in getMenuLocation().
    *
    *  @see     WizardObjectImp::getMenuLocation
    */
   void setMenuLocation(const std::string& location);

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
    *
    *  @see     WizardObjectImp::setMenuLocation
    */
   const std::string& getMenuLocation() const;

   // These methods are documented in the Serializable and TypeAwareObject class documentation
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   std::string mName;
   std::vector<WizardItem*> mItems;
   bool mbBatch;
   std::string mMenuLocation;
};

#define WIZARDOBJECTADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   void WizardObjectAdapter::setName(const std::string& name) \
   { \
      impClass::setName(name); \
   } \
   const std::string& WizardObjectAdapter::getName() const \
   { \
      return impClass::getName(); \
   } \
   const std::vector<WizardItem*>& WizardObjectAdapter::getItems() const \
   { \
      return impClass::getItems(); \
   } \
   bool WizardObjectAdapter::isBatch() const \
   { \
      return impClass::isBatch(); \
   } \
   const std::string& WizardObjectAdapter::getMenuLocation() const \
   { \
      return impClass::getMenuLocation(); \
   } \
   
#endif
