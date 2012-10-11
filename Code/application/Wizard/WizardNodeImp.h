/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDNODEIMP_H
#define WIZARDNODEIMP_H

#include "DataVariant.h"
#include "EnumWrapper.h"
#include "WizardNode.h"
#include "SubjectAdapter.h"

#include <string>
#include <vector>

class WizardItem;

/**
 *  The implementation class for a wizard node.
 *
 *  A wizard node contains the input or output data for an item executed in a wizard.  A
 *  node has a name, type, value and a description.  The node type is a string containing the actual
 *  data type.  This is used in conjunction with the value, which is stored as a void
 *  pointer.  The value can then be any data type, which is identified by the string.  The
 *  name is also used to uniquely identify nodes of the same type.
 *
 *  A node can also be connected with one or more other nodes.  For example, an output node
 *  would be connected with an input node on another wizard item so that the value of the
 *  output node is used as the value for the input node.  All connected nodes are of the
 *  same data type as the original node.  The connected nodes can be retrieved with the
 *  getConnectedNodes() method.
 */
class WizardNodeImp : public WizardNode, public SubjectAdapter
{
public:
   /**
    *  Constructs the wizard node.
    *
    *  @param   pItem
    *           The parent wizard item.
    *  @param   name
    *           The node name.
    *  @param   type
    *           The node type.
    *  @param   description
    *           The node description.
    */
   WizardNodeImp(WizardItem* pItem, std::string name, std::string type, std::string description);

   /**
    *  Destroys the wizard item.
    */
   ~WizardNodeImp();

   /**
    *  Sets the values of this node to that of another node.
    *
    *  This method sets the values of this node to the values of the given node.
    *  The value of the parent wizard item is not set and remains to be the
    *  original item.
    *
    *  @param   pNode
    *           The node whose values should be set in this node.
    *
    *  @return  Returns \c true if this node's values are successfully set to
    *           those of the given node, otherwise returns \c false.
    */
   bool copyNode(const WizardNodeImp* pNode);

   SIGNAL_METHOD(WizardNodeImp, Renamed)
   SIGNAL_METHOD(WizardNodeImp, TypeChanged)
   SIGNAL_METHOD(WizardNodeImp, DescriptionChanged)
   SIGNAL_METHOD(WizardNodeImp, ValueChanged)
   SIGNAL_METHOD(WizardNodeImp, NodeConnected)
   SIGNAL_METHOD(WizardNodeImp, NodeDisconnected)

   /**
    *  Returns the parent wizard item.
    *
    *  @return  The wizard item.
    */
   WizardItem* getItem() const;

   /**
    *  Sets the node name.
    *
    *  @param   nodeName
    *           The node name.
    *
    *  @see     WizardNodeImp::getName
    */
   void setName(const std::string& nodeName);

   /**
    *  Returns the node name.
    *
    *  @return  The node name.
    *
    *  @see     WizardNodeImp::setName
    */
   const std::string& getName() const;

   /**
    *  Sets the node type.
    *
    *  @param   nodeType
    *           The node type.
    *
    *  @see     WizardNodeImp::getType
    */
   void setType(const std::string& nodeType);

   /**
    *  Returns the node type.
    *
    *  @return  The node type.
    *
    *  @see     WizardNodeImp::setType
    */
   const std::string& getType() const;

   /**
    *  Sets the item description.
    *
    *  @param   nodeDescription
    *           The item description.
    */
   void setDescription(const std::string& nodeDescription);

   /**
    *  Gets the item description.
    *
    *  @return  The item description.
    */
   const std::string& getDescription() const;

   /**
    *  Sets the original node type.
    *
    *  Some node types can also support connections with other compatible node types.
    *  The original type specifies the kind of value that must be present for the
    *  wizard to work successfully.  This value can differ from WizardNode::getType(),
    *  since that value is simply the current connected type.
    *
    *  @param   originalType
    *           The original node type.
    */
   void setOriginalType(const std::string& originalType);

   /**
    *  Returns the original node type.
    *
    *  Some node types can also support connections with other compatible node types.
    *  The original type specifies the kind of value that must be present for the
    *  wizard to work successfully.  This value can differ from WizardNode::getType(),
    *  since that value is simply the current connected type.
    *
    *  @return  The original node type.
    *
    *  @see     WizardNodeImp::getType
    *  @see     WizardNodeImp::getValidTypes
    */
   const std::string& getOriginalType() const;

   /**
    *  Sets all valid types for this node.
    *
    *  This method sets the types that this node will accept for connections.  Even though
    *  the node may accept multiple types, the value must always be a kind of the original
    *  type.
    *
    *  @param   validTypes
    *           All valid types for this node.
    */
   void setValidTypes(const std::vector<std::string>& validTypes);

   /**
    *  Returns all valid types for this node.
    *
    *  This method returns the types that this node will accept.  The node may accept
    *  multiple types, but the value must always be a kind of the original type.
    *
    *  @return  All valid types for this node.
    *
    *  @see     WizardNodeImp::getOriginalType
    *  @see     WizardNodeImp::getType
    */
   const std::vector<std::string>& getValidTypes() const;

   /**
    *  Sets the node value.
    *
    *  @param   pValue
    *           The node value as a void pointer.
    *
    *  @see     WizardNodeImp::getValue
    */
   void setValue(void* pValue);

   /**
    *  Returns the node value.
    *
    *  @return  The node value as a void pointer.  The type must be queried to extract
    *           the actual data value referenced by the pointer.
    *
    *  @see     WizardNodeImp::getType
    *  @see     WizardNodeImp::setValue
    */
   void* getValue() const;

   /**
    *  Deletes the node value.
    *
    *  @see     WizardNodeImp::getValue
    *  @see     WizardNodeImp::setValue
    */
   void deleteValue();

   /**
    *  Adds an existing node to the list of connected nodes.
    *
    *  @param   pNode
    *           The connected node.
    *
    *  @return  TRUE if the node was successfully added to the connection list, otherwise
    *           FALSE.
    *
    *  @see     WizardNodeImp::removeConnectedNode
    */
   bool addConnectedNode(WizardNode* pNode);

   /**
    *  Returns the current number of connected nodes.
    *
    *  This method is equivalent to getConnectedNodes().size().
    *
    *  @return  The number of connected nodes.
    *
    *  @see     WizardNodeImp::getConnectedNodes
    */
   int getNumConnectedNodes() const;

   /**
    *  Retrieves all connected nodes.
    *
    *  @return  A reference to a vector containing pointers to all of the nodes connected
    *           to this node.
    */
   const std::vector<WizardNode*>& getConnectedNodes() const;

   /**
    *  Removes node from the connection list.
    *
    *  This method does not delete the node that is removed from the connection list.
    *
    *  @param   pNode
    *           The node to disconnect.
    *
    *  @return  TRUE if the node was successfully disconnected.  FALSE if the node could
    *           not be disconnected or the given node does not exist.
    *
    *  @see     WizardNodeImp::clearConnectedNodes
    */
   bool removeConnectedNode(WizardNode* pNode);

   /**
    *  Removes all connected nodes.
    *
    *  This method does not delete the removed nodes.
    *
    *  @see     WizardNodeImp::removeConnectedNode
    */
   void clearConnectedNodes();

   /**
    *  Queries whether a node is connected to this node.
    *
    *  @param   pNode
    *           The node for which to query for a connection.
    *
    *  @return  TRUE if the node is connected.  FALSE if the node is not connected or the
    *           node does not exist.
    *
    *  @see     WizardNodeImp::getConnectedNodes
    */
   bool isNodeConnected(WizardNode* pNode) const;

   // These methods are documented in the Serializable and TypeAwareObject class documentation
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   WizardNodeImp(const WizardNodeImp& node);
   WizardNodeImp& operator =(const WizardNodeImp& node);

   WizardItem* mpItem;

   std::string mName;
   std::string mType;
   std::string mDescription;
   std::string mOriginalType;
   std::vector<std::string> mValidTypes;
   DataVariant mDeepCopyValue;
   void* mpShallowCopyValue;
   std::vector<WizardNode*> mConnectedNodes;
};

#endif
