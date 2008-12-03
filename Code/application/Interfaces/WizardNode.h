/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef _WIZARDNODE
#define _WIZARDNODE

#include "Serializable.h"

#include <string>
#include <vector>

class WizardItem;

/**
 *  A data element used an input or output for a wizard item.
 *
 *  %Wizard nodes define the input or output data for an item executed in a wizard.  A
 *  node has a name, type, and a value.  The node type is a string containing the actual
 *  data type.  This is used in conjunction with the value, which is stored as a void
 *  pointer.  The value can then be any data type, which is identified by the string.  The
 *  name is also used to uniquely identify nodes of the same type.
 *
 *  A node can also be connected with one or more other nodes.  For example, an output node
 *  would be connected with an input node on another wizard item so that the value of the
 *  output node is used as the value for the input node.  All connected nodes are of the
 *  same data type as the original node.  The connected nodes can be retrieved with the
 *  getConnectedNodes() method.
 *
 *  @see     WizardItem, WizardNodeExt1
 */
class WizardNode : public Serializable
{
public:
   /**
    *  Returns the node name.
    *
    *  @return  The node name.
    *
    *  @see     getType()
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Returns the node type.
    *
    *  @return  The node type.
    *
    *  @see     getOriginalType(), getValidTypes()
    */
   virtual const std::string& getType() const = 0;

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
    *  @see     getType(), getValidTypes()
    */
   virtual const std::string& getOriginalType() const = 0;

   /**
    *  Returns all valid types for this node.
    *
    *  This method returns the types that this node will accept.  The node may accept
    *  multiple types, but the value must always be a kind of the original type.
    *
    *  @return  All valid types for this node.
    *
    *  @see     getType(), getOriginalType()
    */
   virtual const std::vector<std::string>& getValidTypes() const = 0;

   /**
    *  Returns the node value.
    *
    *  @return  The node value as a void pointer.  The type must be queried to extract
    *           the actual data value referenced by the pointer.
    *
    *  @see     getType(), setValue()
    */
   virtual void* getValue() const = 0;

   /**
    *  Sets the node value.
    *
    *  @param   pValue
    *           The node value as a void pointer.
    *
    *  @see     getValue()
    */
   virtual void setValue(void* pValue) = 0;

   /**
    *  Returns the wizard item to which this node belongs.
    *
    *  @return  A pointer to the wizard item that contains this node.
    */
   virtual WizardItem* getItem() const = 0;

   /**
    *  Retrieves all connected nodes.
    *
    *  @return  A reference to a vector containing pointers to all of the nodes connected
    *           to this node.
    */
   virtual const std::vector<WizardNode*>& getConnectedNodes() const = 0;

protected:
   /**
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from WizardItem.  The WizardItem will manage any instances
    * of this object.
    */
   virtual ~WizardNode() {}
};

/**
 *  Extends capability of the WizardNode interface.
 *
 *  This class provides additional capability for the WizardNode interface
 *  class.  A pointer to this class can be obtained by performing a dynamic
 *  cast on a pointer to WizardNode or any of its subclasses.
 *
 *  @warning A pointer to this class can only be used to call methods contained
 *           in this extension class and cannot be used to call any methods in
 *           WizardNode or its subclasses.
 */
class WizardNodeExt1
{
public:
   /**
   *  Gets the item description.
   *
   *  @return  The item description.
   */
   virtual const std::string& getDescription() const = 0;
protected:
   /**
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from WizardItem.  The WizardItem will manage any instances
    * of this object.
    */
   virtual ~WizardNodeExt1() {}
};

#endif
