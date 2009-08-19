/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDITEM_H
#define WIZARDITEM_H

#include "Serializable.h"
#include "WizardNode.h"

#include <string>
#include <vector>

class WizardObject;

/**
 *  A single wizard task.
 *
 *  A WizardItem object defines a single task that is executed from within a wizard.  The
 *  item has a name and a type for the unique identifiers.  The available types are as
 *  follows:
 *
 *  <table>
 *  <tr><td><b>Type</b></td><td><b>%Description</b></td></tr>
 *  <tr><td>Algorithm</td><td>A plug-in typically used for processing data.</td></tr>
 *  <tr><td>%Exporter</td><td>A plug-in to save data to a file.</td></tr>
 *  <tr><td>%Georeference</td><td>A georeferencing algorithm.</td></tr>
 *  <tr><td>%Importer</td><td>A plug-in to load data from a file.</td></tr>
 *  <tr><td>Value</td><td>A specific data value that is stored as a DataVariant.</td></tr>
 *  <tr><td>Viewer</td><td>A plug-in to uniquely display data.</td></tr>
 *  <tr><td>Wizard</td><td>A plug-in used to execute other plug-ins and services.  Many of these
 *    items allow for a run-time selection and execution of a method in the DesktopServices
 *    interface.</td></tr>
 *  </table>
 *
 *  An item is designated to run either in batch mode or interactive mode.  The
 *  getBatchMode() method queries this state.
 *
 *  The item contains input and output nodes that define the required input data and the
 *  generated output data.  The nodes can be retrieved using getInputNodes() and
 *  getOutputNodes() or with getInputNode() and getOutputNode() if the name and type of
 *  the node are known.
 *
 *  @see       WizardItemExt1, WizardNode
 */
class WizardItem : public Serializable
{
public:
   /**
    *  Returns the item name.
    *
    *  @return  The item name.
    *
    *  @see     getType()
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Returns the item type.
    *
    *  @return  The item type.
    *
    *  @see     getName()
    */
   virtual const std::string& getType() const = 0;

   /**
    *  Returns the batch mode operation state of the item.
    *
    *  @return  TRUE if the item is set to run in batch mode.  FALSE if the item is
    *           set to run in interactive mode.
    */
   virtual bool getBatchMode() const = 0;

   /**
    *  Returns whether the item supports the current batch/interactive mode.
    *
    *  @return  TRUE if the item supports the current mode.  FALSE if the item does
    *           not support the current mode.
    *
    *  @see     getBatchMode()
    */
   virtual bool isCurrentModeSupported() const = 0;

   /**
    *  Retrieves a single input node.
    *
    *  @param   name
    *           The input node name.
    *  @param   type
    *           The input node type.
    *
    *  @return  A pointer to the input node.  NULL is returned if no input node exists
    *           with the given name and type.
    *
    *  @see     getInputNodes(), getOutputNode()
    */
   virtual WizardNode* getInputNode(const std::string& name, const std::string& type) const = 0;

   /**
    *  Retrieves all input nodes.
    *
    *  @return  A reference to a vector containing the input node pointers.
    *
    *  @see     getInputNode(), getOutputNodes()
    */
   virtual const std::vector<WizardNode*>& getInputNodes() const = 0;

   /**
    *  Retrieves a single output node.
    *
    *  @param   name
    *           The output node name.
    *  @param   type
    *           The output node type.
    *
    *  @return  A pointer to the output node.  NULL is returned if no output node exists
    *           with the given name and type.
    *
    *  @see     getOutputNodes(), getInputNode()
    */
   virtual WizardNode* getOutputNode(const std::string& name, const std::string& type) const = 0;

   /**
    *  Retrieves all output nodes.
    *
    *  @return  A reference to a vector containing the output node pointers.
    *
    *  @see     getOutputNode(), getInputNodes()
    */
   virtual const std::vector<WizardNode*>& getOutputNodes() const = 0;

   /**
    *  Queries whether an item is directly or indirectly connected to this item.
    *
    *  This method provides a recursive search for the connected item by searching the
    *  entire chain of items connected to the reference item.  The input flag specifies
    *  whether to search the chain of input node or output node connections.
    *
    *  @param   pItem
    *           The item to query for a connection
    *  @param   bInputNode
    *           TRUE to search items connected to the input nodes and FALSE to search
    *           items connected to the output nodes.
    *
    *  @return  TRUE if the item is connected.  FALSE if the item is not connected or the
    *           item does not exist.
    *
    *  @see     getConnectedItems()
    */
   virtual bool isItemConnected(WizardItem* pItem, bool bInputNode) const = 0;

   /**
    *  Retrieves wizard items connected to the input or output nodes of this item.
    *
    *  @param   bInputNode
    *           TRUE to return items connected to the input nodes and FALSE to return
    *           items connected to the output nodes.
    *  @param   connectedItems
    *           A vector that is populated with pointers to the wizard items connected
    *           with this one.
    *
    *  @see     isItemConnected()
    */
   virtual void getConnectedItems(bool bInputNode, std::vector<WizardItem*>& connectedItems) const = 0;

protected:
   /**
    * A plug-in cannot create this object; it can only retrieve an already existing
    * object from WizardObject.  The WizardObject will manage any instances
    * of this object.
    */
   virtual ~WizardItem() {}
};

/**
 * Extends capability of the WizardItem interface.
 *
 * This class provides additional capability for the WizardItem interface
 * class.  A pointer to this class can be obtained by performing a dynamic cast
 * on a pointer to WizardItem.
 *
 * @warning A pointer to this class can only be used to call methods contained
 *          in this extension class and cannot be used to call any methods in
 *          WizardItem.
 */
class WizardItemExt1
{
public:
   /**
    * Returns the parent wizard containing this item.
    *
    * @return  The parent wizard.
    */
   virtual WizardObject* getWizard() = 0;

   /**
    * Returns the parent wizard containing this item.
    *
    * @return  The parent wizard.
    */
   virtual const WizardObject* getWizard() const = 0;

protected:
   /**
    * A plug-in cannot create this object; it can only retrieve an already
    * existing item from WizardObject.  The WizardObject will manage any
    * instances of this object.
    */
   virtual ~WizardItemExt1()
   {}
};

#endif
