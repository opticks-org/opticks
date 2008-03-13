/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDITEMIMP_H
#define WIZARDITEMIMP_H

#include "EnumWrapper.h"
#include "WizardItem.h"
#include "SubjectAdapter.h"
#include "WizardNode.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

#include <string>
#include <vector>

/**
 *  Contains information about two nodes that comprise a connection in a wizard.
 *
 *  This struct represents a connection between two wizard nodes.  It contains a
 *  pointer to the output node and the connected input node as well as index values
 *  for the item containing the node and its index in the node list.  It may be
 *  possible for the node values to be valid and the index values invalid and vice
 *  versa.
 */
struct WizardConnection
{
   WizardConnection()
   {
      mpInputNode = NULL;
      mpOutputNode = NULL;
      miInputItemIndex = -1;
      miInputNodeIndex = -1;
      miOutputItemIndex = -1;
      miOutputNodeIndex = -1;
   }

   WizardNode* mpInputNode;
   WizardNode* mpOutputNode;
   int miInputItemIndex;
   int miInputNodeIndex;
   int miOutputItemIndex;
   int miOutputNodeIndex;
};

/**
 *  The implementation class for a wizard item.
 *
 *  A wizard item defines a single task that is executed from within a wizard.  The
 *  item has a name and a type to uniquely identify the item.  The available types are
 *  documented in the WizardItem class documentation.
 *
 *  An item also has a batch mode and a coordinate position for display in the wizard
 *  builder.  The batch mode applies only to the item and not the entire wizard.  Batch
 *  and interactive mode can be set with the setBatchMode() method.
 *
 *  The item also contains input and output nodes contining the data used to execute
 *  the item.  Nodes can be added, removed, and cleared from the item, and the
 *  isItemConnected() and getConnectedItems() methods can be used to query which items
 *  may be connected to the nodes.
 *
 *  @see    WizardItem
 */
class WizardItemImp : public WizardItem, public SubjectAdapter
{
public:
   /**
    *  Constructs the wizard item.
    *
    *  @param   name
    *           The item name.
    *  @param   type
    *           The item type.
    */
   WizardItemImp(std::string name, std::string type);

   /**
    *  Destroys the wizard item.
    *
    *  The destructor destroys the wizard item and all input and output nodes.
    */
   ~WizardItemImp();

   /**
    *  Sets the values of this item to that of another item.
    *
    *  This opertor sets the values of this item to the values of the given item
    *  by performing a deep copy.  This means that any existing nodes are destroyed
    *  and new nodes are created with values set to those of the given item's nodes.
    *
    *  @param   item
    *           The item whose values should be set in this item.
    */
   WizardItemImp& operator =(const WizardItemImp& item);

   /**
    *  Possible types of member data modifications.
    *
    *  This enumerated type is used when calling notify() to indicate the type of change event
    *  prompting the notify.  The address of the change type is set as the value parameter of
    *  notify() call.
    *
    *  @see     WizardNodeImp::WizardNodeChangeType
    */
   enum WizardItemChangeTypeEnum { ItemName, ItemPosition, ItemBatchMode, ItemNodeAdded, ItemNodeRemoved };

   /**
    * @EnumWrapper WizardItemImp::WizardItemChangeTypeEnum.
    */
   typedef EnumWrapper<WizardItemChangeTypeEnum> WizardItemChangeType;

   /**
    *  Sets the item name.
    *
    *  @param   name
    *           The item name.
    *
    *  @see     WizardItemImp::getName
    */
   void setName(const std::string& name);

   /**
    *  Returns the item name.
    *
    *  @return  The item name.
    *
    *  @see     WizardItemImp::setName
    */
   const std::string& getName() const;

   /**
    *  Returns the item type.
    *
    *  @return  The item type.
    */
   const std::string& getType() const;

   /**
    *  Sets the batch mode operation state of the item.
    *
    *  @param   bBatch
    *           TRUE if the item should run in batch mode.  FALSE if the item should
    *           run in interactive mode.
    *  @param   bModeSupported
    *           TRUE if the item can support the given mode.  FALSE if the item does
    *           not support the given mode.
    *
    *  @see     WizardItemImp::getBatchMode
    */
   void setBatchMode(bool bBatch, bool bModeSupported);

   /**
    *  Returns the batch mode operation state of the item.
    *
    *  @return  TRUE if the item is set to run in batch mode.  FALSE if the item is
    *           set to run in interactive mode.
    *
    *  @see     WizardItemImp::setBatchMode
    */
   bool getBatchMode() const;

   /**
    *  Returns whether the item supports the current batch/interactive mode.
    *
    *  @return  TRUE if the item supports the current mode.  FALSE if the item does
    *           not support the current mode.
    *
    *  @see     WizardItemImp::getBatchMode
    */
   bool isCurrentModeSupported() const;

   /**
    *  Sets the item coordinate position for display in the wizard builder.
    *
    *  @param   dX
    *           The x-coordinate.
    *  @param   dY
    *           The y-coordinate.
    *
    *  @see     WizardItemImp::getXPosition
    *  @see     WizardItemImp::getYPosition
    */
   void setPosition(double dX, double dY);

   /**
    *  Returns the x-coordinate position of the item
    *
    *  @return  The x-coordinate.
    *
    *  @see     WizardItemImp::getYPosition
    *  @see     WizardItemImp::setPosition
    */
   double getXPosition() const;

   /**
    *  Returns the y-coordinate position of the item
    *
    *  @return  The y-coordinate.
    *
    *  @see     WizardItemImp::getXPosition
    *  @see     WizardItemImp::setPosition
    */
   double getYPosition() const;

   /**
    *  Adds a new input node.
    *
    *  @param   name
    *           The input node name.
    *  @param   type
    *           The input node type.
    *
    *  @return  A pointer to the new input node.
    *
    *  @see     WizardItemImp::addOutputNode
    */
   WizardNode* addInputNode(const std::string& name, const std::string& type);

   /**
    *  Returns the current number of input nodes.
    *
    *  This method is equivalent to getInputNodes().size().
    *
    *  @return  The number of input nodes.
    *
    *  @see     WizardItemImp::getInputNodes
    *  @see     WizardItemImp::getNumOutputNodes
    */
   int getNumInputNodes() const;

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
    *  @see     WizardItemImp::getInputNodes
    *  @see     WizardItemImp::getOutputNode
    */
   WizardNode* getInputNode(const std::string& name, const std::string& type) const;

   /**
    *  Retrieves all input nodes.
    *
    *  @return  A reference to a vector containing the input node pointers.
    *
    *  @see     WizardItemImp::getInputNode
    *  @see     WizardItemImp::getOutputNodes
    */
   const std::vector<WizardNode*>& getInputNodes() const;

   /**
    *  Queries whether a node is an input node.
    *
    *  @param   pNode
    *           The node to query to see if it is an input node.
    *
    *  @return  True if the node is an input node or false if the node is an output
    *           node or does not exist in the item.
    *
    *  @see     WizardItemImp::isOutputNode
    */
   bool isInputNode(WizardNode* pNode) const;

   /**
    *  Removes a single input node.
    *
    *  @param   pNode
    *           The input node.
    *  @param   bDeleteValue
    *           TRUE if the memory for the node should be deleted.  FALSE if the memory
    *           should not be deleted.
    *
    *  @return  TRUE if the input node was successfully removed, otherwise FALSE.
    *
    *  @see     WizardItemImp::clearInputNodes
    */
   bool removeInputNode(WizardNode* pNode, bool bDeleteValue);

   /**
    *  Removes and deletes all input nodes.
    *
    *  @see     WizardItemImp::removeInputNode
    */
   void clearInputNodes();

   /**
    *  Adds a new output node.
    *
    *  @param   name
    *           The output node name.
    *  @param   type
    *           The output node type.
    *
    *  @return  A pointer to the new output node.
    *
    *  @see     WizardItemImp::addInputNode
    */
   WizardNode* addOutputNode(const std::string& name, const std::string& type);

   /**
    *  Returns the current number of output nodes.
    *
    *  This method is equivalent to getOutputNodes().size().
    *
    *  @return  The number of output nodes.
    *
    *  @see     WizardItemImp::getOutputNodes
    *  @see     WizardItemImp::getNumInputNodes
    */
   int getNumOutputNodes() const;

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
    *  @see     WizardItemImp::getOutputNodes
    *  @see     WizardItemImp::getInputNode
    */
   WizardNode* getOutputNode(const std::string& name, const std::string& type) const;

   /**
    *  Retrieves all output nodes.
    *
    *  @return  A reference to a vector containing the output node pointers.
    *
    *  @see     WizardItemImp::getOutputNode
    *  @see     WizardItemImp::getInputNodes
    */
   const std::vector<WizardNode*>& getOutputNodes() const;

   /**
    *  Queries whether a node is an output node.
    *
    *  @param   pNode
    *           The node to query to see if it is an output node.
    *
    *  @return  True if the node is an output node or false if the node is an intput
    *           node or does not exist in the item.
    *
    *  @see     WizardItemImp::isInputNode
    */
   bool isOutputNode(WizardNode* pNode) const;

   /**
    *  Removes a single output node.
    *
    *  @param   pNode
    *           The output node.
    *  @param   bDeleteValue
    *           TRUE if the memory for the node should be deleted.  FALSE if the memory
    *           should not be deleted.
    *
    *  @return  TRUE if the output node was successfully removed, otherwise FALSE.
    *
    *  @see     WizardItemImp::clearOutputNodes
    */
   bool removeOutputNode(WizardNode* pNode, bool bDeleteValue);

   /**
    *  Removes and deletes all output nodes.
    *
    *  @see     WizardItemImp::removeOutputNode
    */
   void clearOutputNodes();

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
    *  @see     WizardItemImp::getConnectedItems
    */
   bool isItemConnected(WizardItem* pItem, bool bInputNode) const;

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
    *  @see     WizardItemImp::isItemConnected
    */
   void getConnectedItems(bool bInputNode, std::vector<WizardItem*>& connectedItems) const;

   /**
    *  Returns connection objects that respresent the connections between items in the
    *  given vector.
    *
    *  @param   items
    *           The wizard items for which to get connection objects describing the
    *           connected nodes.
    *
    *  @return  A vector containing wizard connection objects describing the connections
    *           between the given items.
    *
    *  @see     WizardConnection
    *  @see     WizardItemImp::setConnections
    */
   static std::vector<WizardConnection> getConnections(const std::vector<WizardItem*>& items);

   /**
    *  Sets connections in the given items based on the given connection objects.
    *
    *  @param   items
    *           The wizard items in which to set connections.
    *  @param   connections
    *           A vector containing wizard connection objects describing the connections
    *           between the given items.  The size of this vector should be identical to
    *           the size of the items vector.
    *
    *  @see     WizardConnection
    *  @see     WizardItemImp::getConnections
    */
   static void setConnections(std::vector<WizardItem*>& items,
      const std::vector<WizardConnection>& connections);

   // These methods are documented in the Serializable and TypeAwareObject class documentation
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   std::string mName;
   std::string mType;
   bool mbBatch;
   bool mbModeSupported;
   LocationType mCoord;
   std::vector<WizardNode*> mInputNodes;
   std::vector<WizardNode*> mOutputNodes;
};

#endif
