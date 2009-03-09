/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDCANVASITEM_H
#define WIZARDCANVASITEM_H

#include <Qt3Support/Q3Canvas>
#include <Qt3Support/Q3PointArray>
#include <QtCore/QMap>
#include <QtGui/QPainter>
#include <QtGui/QToolButton>

class WizardItem;
class WizardItemImp;
class WizardNode;

/**
 *  A canvas item containing connectable input and output nodes.
 *
 *  A WizardCanvasItem provides the means to chain together plug-ins and specific values
 *  using input and output nodes.  Each node has the following attributes in the item:
 *  name, current type, original type, valid types, color, index, location, and connection
 *  point.  The name, current type, and color define the node.  The index, location, and
 *  connection point are available to connect with another item.  The valid types and
 *  original type indicate if a node can be connected with multipl types of nodes.
 *
 *  The item is rectangular in shape, with the item name appearing below the rectangular
 *  area.  The input nodes are located on the left side of the rectangle, and the output
 *  nodes are located on the right side.  An execution order of the item appears in the
 *  center of the rectangluar area.
 *
 *  The color of the item can be set using setColor(), and the item has a black border
 *  that becomes red when the WizardItem is set to run in batch mode.
 *
 *  The height of the item varies based on the number of input and output nodes, and the
 *  width varies based on the item name.  The total size of the item is obtained with
 *  areaPoints() which return a list of points defining the polygonal extents of the item.
 *
 *  @see     WizardItem
 */
class WizardCanvasItem : public Q3CanvasPolygonalItem
{
public:
   /**
    *  Constructs a wizard canvas item.
    *
    *  This constructor creates a wizard item with no initial nodes.
    *
    *  @param   pCanvas
    *           The associated canvas object.
    */
   WizardCanvasItem(WizardItem* pItem, Q3Canvas* pCanvas);

   /**
    *  Destructs the wizard canvas item.
    */
   ~WizardCanvasItem();

   /**
    *  Retrieves the wizard item in the data model corresponding to this canvas item.
    *
    *  @return  A pointer to the wizard item in the data model.
    */
   WizardItem* getWizardItem() const;

   /**
    *  Sets the background color of the item.
    *
    *  @param   clrItem
    *           The new color for the item.
    *
    *  @see     WizardCanvasItem::getColor
    */
   void setColor(const QColor& clrItem);

   /**
    *  Retrieves the background color of the item.
    *
    *  @return  The background color of the item.
    *
    *  @see     WizardCanvasItem::setColor
    */
   QColor getColor() const;

   /**
    *  Sets the execution order of the item.
    *
    *  This method sets the value indicating its exeuction place in the wizard.  The number
    *  is drawn in the top center of the item.  Items with a lower execution order value
    *  are run before items with a higher value.
    *
    *  @param   iOrder
    *           The new one-based execution order for the item.
    *
    *  @see     WizardCanvasItem::getExecutionOrder
    */
   void setExecutionOrder(int iOrder);

   /**
    *  Retrieves the execution order.
    *
    *  @return  The one-based value indicating the place in the wizard execute sequence.
    *
    *  @see     WizardCanvasItem::setExecutionOrder
    */
   int getExecutionOrder() const;

   /**
    *  Moves the item and sets all canvas chunks covered by the item as changed.
    *
    *  This method overrides the QCanvasPolygonalItem method and calls setChanged() to
    *  ensure that all canvas chunks are appropriately updated.
    *
    *  @param   dX
    *           The x-distance in canvas coordinates for which to move the item.
    *  @param   dY
    *           The y-distance in canvas coordinates for which to move the item.
    */
   void moveBy(double dX, double dY);

   /**
    *  Selects the items and sets all canvas chunks covered by the item as changed.
    *
    *  This method overrides the QCanvasPolygonalItem method and calls setChanged() to
    *  ensure that all canvas chunks are appropriately updated.
    *
    *  @param   bSelected
    *           True if the item should be selected or false if the item should be
    *           deselected.
    */
   void setSelected(bool bSelected);

   /**
    *  Recalculates the canvas item dimensions.
    *
    *  This method calculates the height of the rectangle based on the number of input and
    *  output nodes, and the width and height of the item text based on the name's font
    *  characteristics.  The item is erased and redrawn to ensure that the bounding rectangle
    *  is accurate.
    */
   void updateDimensions();

   /**
    *  Retrieves the wizard node at the given point.
    *
    *  @param   pos
    *           The canvas coordinate location.
    *
    *  @return  A pointer to the node at the given point.  NULL is returned if no node
    *           exists at the given point.
    */
   WizardNode* getNode(const QPoint& pos) const;

   /**
    *  Returns the color of a given wizard node.
    *
    *  @param   pNode
    *           The wizard node.  Cannot be NULL.
    *
    *  @return  The color of the wizard node.
    *
    *  @see     WizardCanvasItem::setNodeColor
    */
   QColor getNodeColor(WizardNode* pNode) const;

   /**
    *  Returns the bounding rectangle of a given wizard node.
    *
    *  @param   pNode
    *           The wizard node for which to get its bounding rectangle.  Cannot be NULL.
    *
    *  @return  The bounding rectangle of the node in canvas coordinates.
    */
   QRect getNodeRect(WizardNode* pNode) const;

   /**
    *  Returns the bounding rectangle of the toggle type button associated with a given
    *  wizard node.
    *
    *  @param   pNode
    *           The wizard node for which to get the bounding rectangle of its associated
    *           toggle type button.  Cannot be NULL.
    *
    *  @return  The bounding rectangle of toggle type button associated with a given
    *           wizard node in canvas coordinates.  If the node does not have an
    *           associated toggle button, an invalid QRect is returned.
    */
   QRect getNodeToggleRect(WizardNode* pNode) const;

   /**
    *  Returns the connection point of a given wizard node.
    *
    *  @param   pNode
    *           The wizard node for which to get its connection point.  Cannot be NULL.
    *
    *  @return  The connection point of the node in canvas coordinates.
    *
    *  @see     WizardCanvasItem::getOutputNodeConnectionPoint
    */
   QPoint getNodeConnectionPoint(WizardNode* pNode) const;

   /**
    *  Sets the active wizard node.
    *
    *  The active wizard node is the node for which the user is toggling the valid types.
    *  the toggle button for the active node is drawn as a despressed button to indicate
    *  that it is the active node that is being toggled.
    *
    *  @param   pNode
    *           The new active wizard node.  Can be NULL to specify that no node is active.
    */
   void setActiveNode(WizardNode* pNode);

   /**
    *  Returns the active wizard node.
    *
    *  @return  The active wizard node.  NULL is returned if no node is currently active.
    *
    *  @see     WizardCanvasItem::setActiveNode
    */
   WizardNode* getActiveNode() const;

protected:
   /**
    *  Returns a list of points defining the area of the item.
    *
    *  This method returns a list of points that together define the actual area of the
    *  item.  The area includes several pixels of margin so that the item can be properly
    *  drawn.
    *
    *  @return  The list of points defining the area of the item.
    */
   Q3PointArray areaPoints() const;

   /**
    *  Draws the item on the canvas.
    *
    *  @param   p
    *           The painting context for the canvas.
    */
   void drawShape(QPainter& p);

   /**
    *  Returns the index for a given wizard node.
    *
    *  @param   pNode
    *           The wizard node for which to retrieve its index.
    *
    *  @return  The zero-based node index.  A -1 value is returned if the node does not
    *           exist.  The index value of the node at the top of the item rectangle is
    *           zero.
    */
   int getNodeIndex(WizardNode* pNode) const;

   /**
    *  Sets the canvas chunks covered by the item as changed.
    *
    *  This is different than calling QCanvas::setChanged() in that the chunks covered
    *  by the selection nodes are set as changed and not just the item extents returned
    *  from areaPoints().
    */
   void setChanged();

private:
   WizardItemImp* mpItem;
   int mMargin;

   // Rectangle
   int mRectWidth;
   int mRectHeight;
   int mBorderWidth;
   QColor mColor;
   int mOrder;

   // Nodes
   int mNodeDiameter;
   int mNodeMargin;
   QToolButton* mpDownButton;
   WizardNode* mpActiveNode;
   static QMap<QString, QColor> mNodeColors;

   // Text
   int mTextWidth;
   int mTextHeight;

   // Selection nodes
   int mSelectionNodeSize;
};

#endif
