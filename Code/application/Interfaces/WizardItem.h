/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef _WIZARDITEM
#define _WIZARDITEM

#include "Serializable.h"
#include "WizardNode.h"

#include <string>
#include <vector>

/**
 *  A single wizard task.
 *
 *  A WizardItem object defines a single task that is executed from within a wizard.  The
 *  item has a name and a type for the unique identifiers.  The available types are as
 *  follows:
 *
 * <pre>
 *    Type              %Description
 *    ===============   =======================================================================
 *    Algorithm         A plug-in typically used for processing data.
 *    Desktop           This item allows for a run-time selection and execution of a method in
 *                      the DesktopServices interface.  This type of item is only useful
 *                      in batch mode.
 *    Exporter          A plug-in to save data to a file.
 *    Georeference      A georeferencing algorithm.
 *    Importer          A plug-in to load data from a file.
 *    Interpreter       A plug-in for scripting language support.
 *    RasterPager       A plug-in to map data for a RasterElement.
 *    Value             A specific data value.  The available data types for a value item are
 *                      the types documented in the DynamicObject::set() method.
 *    Viewer            A plug-in to uniquely display data.
 *    Wizard            A plug-in used to execute other plug-ins and services.
 * </pre>
 *
 *  An item is designated to run either in batch mode or interactive mode.  The
 *  getBatchMode() method queries this state.
 *
 *  The item contains input and output nodes that define the required input data and the
 *  generated output data.  The nodes can be retrieved using getInputNodes() and
 *  getOutputNodes() or with getInputNode() and getOutputNode() if the name and type of
 *  the node are known.
 *
 *  @see        WizardNode
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
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from WizardObject.  The WizardObject will manage any instances
    * of this object.
    */
   virtual ~WizardItem() {}
};

#endif
