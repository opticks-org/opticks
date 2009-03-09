/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDVIEW_H
#define WIZARDVIEW_H

#include <Qt3Support/Q3Canvas>

#include "Observer.h"
#include "WizardCanvasItem.h"
#include "WizardItemImp.h"
#include "WizardLine.h"
#include "WizardNode.h"

#include <vector>

/**
 *  The view widget for the wizard canvas items.
 *
 *  A WizardView object is the actual widget to display the items on the wizard canvas.
 *  These items include the wizard items themselves, connection lines, and selection nodes.
 *  The WizardView allows manipulation of the item layout and configuration.  Wizard items
 *  and connection lines can be selected with the selectItem() method.  Selection nodes
 *  are then created for the item and displayed.  Their initial z-order is set starting with
 *  10000 and increasing with each node, so items should have a z-order of less than 10000
 *  for the nodes to be displayed on top.  Wizard items must be deleted by calling
 *  deleteSelectedItems() for selection node clean-up.
 *
 *  @see   WizardCanvasItem, WizardLine
 */
class WizardView : public Q3CanvasView, public Observer
{
   Q_OBJECT

public:
   /**
    *  Constructs a WizardView.
    *
    *  @param   pViewing
    *           The "viewed" canvas.  All items on the canvas are displayable in the view.
    *  @param   pParent
    *           The parent widget.
    */
   WizardView(Q3Canvas* pViewing = 0, QWidget* pParent = 0);

   /**
    *  Destroys the view.
    *
    *  The destructor destroys the view widget, but the canvas and its items are not
    *  destroyed.
    */
   ~WizardView();

   void itemAttached(Subject &subject, const std::string &signal, const boost::any &data);
   void itemModified(Subject &subject, const std::string &signal, const boost::any &data);
   void itemDeleted(Subject &subject, const std::string &signal, const boost::any &data);
   void nodeAttached(Subject &subject, const std::string &signal, const boost::any &data);
   void nodeModified(Subject &subject, const std::string &signal, const boost::any &data);
   void nodeDeleted(Subject &subject, const std::string &signal, const boost::any &data);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);

   /**
    *  Selects a wizard item and displays its selection nodes.
    *
    *  This method sets the item as selected and creates selection nodes based on the
    *  item's bounding rectangle.  The selectionChanged() signal is emitted, indicating
    *  a change in the view's overall selected items.
    *
    *  @param   pItem
    *           The wizard item to select.  The pointer cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the view should be redrawn after the item is selected, otherwise
    *           FALSE.
    *
    *  @see     WizardView::selectItems
    *  @see     WizardView::selectAllItems
    *  @see     WizardView::deselectItem
    */
   void selectItem(WizardItem* pItem, bool bRefresh);

   /**
    *  Selects multiple wizard items.
    *
    *  @param   items
    *           The wizard items to select.
    *  @param   bRefresh
    *           TRUE if the view should be redrawn after the items are selected, otherwise
    *           FALSE.
    *
    *  @see     WizardView::selectItem
    *  @see     WizardView::selectAllItems
    */
   void selectItems(const std::vector<WizardItem*>& items, bool bRefresh);

   /**
    *  Selects all wizard items.
    *
    *  This method sets all items in the view as selected and creates corresponding
    *  selection nodes.  The selectionChanged() signal is emitted if new items are selected,
    *  indicating a change in the view's overall selected items.
    *
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the selecting the items,
    *           otherwise FALSE.
    *
    *  @see     WizardView::selectItem
    *  @see     WizardView::selectItems
    */
   void selectAllItems(bool bRefresh);

   /**
    *  Queries whether a wizard item is selected.
    *
    *  @param   pItem
    *           The wizard item to query for its selection state.
    *
    *  return   TRUE if the given wizard item is selected, otherwise FALSE.
    *
    *  @see     WizardView::selectItem
    *  @see     WizardView::deselectItem
    */
   bool isItemSelected(WizardItem* pItem) const;

   /**
    *  Returns a vector of selected wizard items.
    *
    *  return   A vector contining pointers to all selected wizard items.  The vector
    *           does not contain data for selected connections.
    *
    *  @see     WizardView::getSelectedConnections
    *  @see     WizardView::getNumSelectedItems
    */
   std::vector<WizardItem*> getSelectedItems() const;

   /**
    *  Returns the number of currently selected wizard items.
    *
    *  This method is equivalent to calling getSelectedItems().size().
    *
    *  return   The number of currently selected wizard items.
    *
    *  @see     WizardView::getSelectedItems
    */
   int getNumSelectedItems() const;

   /**
    *  Deselects a wizard item and destroys its selection nodes.
    *
    *  This method sets the item as unselected and deletes its corresponding selection
    *  nodes.  The selectionChanged() signal is emitted, indicating a change in the
    *  view's overall selected items.
    *
    *  @param   pItem
    *           The wizard item to deselect.  The pointer cannot be NULL.  If the item is
    *           already not selected, the selectionChanged() signal is not emitted.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the deselecting the items,
    *           otherwise FALSE.
    *
    *  @see     WizardView::deselectAllItems
    *  @see     WizardView::selectItem
    */
   void deselectItem(WizardItem* pItem, bool bRefresh);

   /**
    *  Deselects all view items and destroys their selection nodes.
    *
    *  This method sets all items in the view as unselected and deletes their
    *  corresponding selection nodes.  The selectionChanged() signal is emitted,
    *  indicating a change in the view's overall selected items, provided at least
    *  one item is deselected.
    *
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the deselecting the items,
    *           otherwise FALSE.
    *
    *  @see     WizardView::deselectItem
    */
   void deselectAllItems(bool bRefresh);

   /**
    *  Returns a vector of selected wizard item connections.
    *
    *  return   A vector contining the wizard connection data for all selected connections.
    *           The vector does not contain data for selected wizard items.
    *
    *  @see     WizardView::getNumSelectedConnections
    */
   std::vector<WizardConnection> getSelectedConnections() const;

   /**
    *  Returns the number of currently selected wizard item connections.
    *
    *  This method is equivalent to calling getSelectedConnections().size().
    *
    *  return   The number of currently selected wizard item connections.
    *
    *  @see     WizardView::getSelectedConnections
    */
   int getNumSelectedConnections() const;

   /**
    *  Sets the execution order for each wizard item in the view.
    *
    *  This method sets the execution order for each wizard item based on the position
    *  in the given vector.
    *
    *  @param   items
    *           The wizard items for which to set the execution order.  The first item in
    *           the vector will have an execution order value of 1.
    */
   void updateOrder(std::vector<WizardItem*> items);

   /**
    *  Constructs wizard items and connections.
    *
    *  This method creates wizard canvas items and connections for the items in the
    *  given vector.
    *
    *  @param   items
    *           The wizard items for which to create the view items and connections.
    *  @param   bClear
    *           TRUE if the view should be cleared before creating the new canvas
    *           items, otherwise FALSE.
    */
   void buildView(std::vector<WizardItem*> items, bool bClear);

   /**
    *  Draws the entire view on the given painter.
    *
    *  @param   p
    *           The painter object on which to draw the view.
    */
   void drawView(QPainter* p);

   /**
    *  Destroys all view items and destroys all selection nodes.
    *
    *  This method deletes all canvas items in the view, their corresponding selection
    *  nodes, and the wizard items in the data model.  If any items are selected, the
    *  selectionChanged() signal is emitted.
    *
    *  @see     WizardView::deleteSelectedItems
    */
   void clear();

signals:
   /**
    *  Notification of a change in selected items.
    *
    *  This signal is emitted when one or more items is selected or deselected.  Deleted
    *  items are also deselected, thereby emitting the signal.
    */
   void selectionChanged();

   /**
    *  Notification of a change in item position on the canvas.
    *
    *  This signal is emitted when one or more items moves on the canvas.
    *
    *  @param   pItem
    *           The wizard item that changed position.
    *  @param   dX
    *           The new local x-coordinate of the wizard item.
    *  @param   dY
    *           The new local y-coordinate of the wizard item.
    */
   void itemPositionChanged(WizardItem* pItem, double dX, double dY);

   /**
    *  Notification when a mouse button is pressed over a wizard item.
    *
    *  @param   e
    *           The mouse event.
    *  @param   pItem
    *           The wizard item that was pressed.
    */
   void itemPressed(QMouseEvent* e, WizardItem* pItem);

   /**
    *  Notification when a wizard item is double clicked.
    *
    *  @param   e
    *           The double click mouse event.
    *  @param   pItem
    *           The wizard item that was double clicked.
    */
   void itemDoubleClicked(QMouseEvent* e, WizardItem* pItem);

   /**
    *  Notification when two wizard items are connected
    *
    *  @param   pOutputItem
    *           The item containing the output node that was connected.
    *  @param   pInputItem
    *           The item containing the input node that was connected.
    */
   void itemsConnected(WizardItem* pOutputItem, WizardItem* pInputItem);

   /**
    *  Notification when the current type of a wizard node changes.
    *
    *  This signal is emitted when users change the node type of a node
    *  with multiple valid types by clicking on the down arrow next to
    *  the node.
    *
    *  @param   pNode
    *           The node whose type has changed.
    */
   void nodeTypeChanged(WizardNode* pNode);

protected:
   bool event(QEvent* pEvent);

   /**
    *  Selects or deselects items on the canvas.
    *
    *  This method override is used primarily for selecting items when the left mouse
    *  button is pressed.  If the mouse press location is over an item, the item is
    *  selected and other items are deselected.  If the Shift or Ctrl keys are depressed,
    *  other items are not deselected.  A mouse press that is not over an item deselects
    *  all items unless the Shift or Ctrl keys are depressed, and starts the dragging
    *  rectangle sequence.
    *
    *  @param   e
    *           The incoming mouse message.
    */
   void viewportMousePressEvent(QMouseEvent* e);

   /**
    *  Checks for double clicks on a wizard item.
    *
    *  This method override intercepts a double click mouse event and checks whether the
    *  double click occurred on a wizard item.  If so, the itemDoubleClicked() signal is
    *  emitted.
    *
    *  @param   e
    *           The incoming mouse message.
    *
    *  @see     WizardView::itemDoubleClicked
    */
   void viewportMouseDoubleClickEvent(QMouseEvent* e);

   /**
    *  Moves selected items around the canvas or drags a rectangle in which items will be
    *  selected.
    *
    *  This method override moves all selected items around the canvas.  All selected
    *  items are moved the same distance as the mouse.  If the user is dragging a
    *  rectangle, no itmes are moved and the drag rectangle is updated.
    *
    *  @param   e
    *           The incoming mouse message.
    */
   void viewportMouseMoveEvent(QMouseEvent* e);

   /**
    *  Selects all items within the dragging rectangle.
    *
    *  This method selects all items within the dragging rectangle.  If the user is not
    *  dragging a rectangle, the implementation does nothing.
    *
    *  @param   e
    *           The incoming mouse message.
    */
   void viewportMouseReleaseEvent(QMouseEvent* e);

   /**
    *  Selects a canvas item and displays its selection nodes.
    *
    *  This method sets the canvas item as selected and creates selection nodes based on the
    *  item's bounding rectangle.  Valid canvas items include wizard items and wizard line
    *  items.  The selectionChanged() signal is emitted, indicating a change in the view's
    *  overall selected items.
    *
    *  @param   pItem
    *           The canvas item to select.  The pointer cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the view should be redrawn after the item is selected, otherwise
    *           FALSE.
    *
    *  @return  TRUE if the item was successfully selected, otherwise FALSE.
    *
    *  @see     WizardView::selectItem
    */
   bool selectItem(Q3CanvasItem* pItem, bool bRefresh);

   /**
    *  Selects view items within the given area.
    *
    *  This method sets items in the given rectangle in the view as selected and creates
    *  corresponding selection nodes.  The selectionChanged() signal is emitted if new
    *  items are selected, indicating a change in the view's overall selected items.
    *
    *  @param   rcSelection
    *           The rectangle containing the canvas items to select.  The canvas item
    *           must be located completely within the rectangle area to be selected.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the selecting the items,
    *           otherwise FALSE.
    *
    *  @see     WizardView::selectItem
    *  @see     WizardView::selectAllItems
    */
   void selectItems(QRect rcSelection, bool bRefresh);

   /**
    *  Deselects a canvas item and destroys its selection nodes.
    *
    *  This method deselects a canvas item and deletes its corresponding selection
    *  nodes.  Valid canvas items include wizard items and wizard line items.  The
    *  selectionChanged() signal is emitted, indicating a change in the view's overall
    *  selected items.
    *
    *  @param   pItem
    *           The canvas item to deselect.  The pointer cannot be NULL.  If the item is
    *           already not selected, the selectionChanged() signal is not emitted.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the deselecting the items,
    *           otherwise FALSE.
    *
    *  @return  TRUE if the item was successfully deselected, otherwise FALSE.
    *
    *  @see     WizardView::deselectItem
    */
   bool deselectItem(Q3CanvasItem* pItem, bool bRefresh);

   /**
    *  Returns the bounding rectangle of all selected items.
    *
    *  @return  The bounding rectangle of all selected items in contents coordinates.
    */
   QRect getSelectedItemsRect() const;

   /**
    *  Increases the canvas size to contain the given item.
    *
    *  This method checks the coordinates of the given item and increases the canvas
    *  size if necessary to completely contain the item.  If the item is already
    *  completely contained within the current canvas area, the canvas is not resized.
    *
    *  @param   pItem
    *           The canvas item by which to increase the canvas size.
    */
   void expandCanvasToItem(Q3CanvasItem* pItem);

   /**
    *  Adds a connection line between two wizard nodes.
    *
    *  @param   pOutputNode
    *           The output wizard node.  Cannot be NULL.
    *  @param   pInputNode
    *           The input wizard node.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connection, otherwise
    *           FALSE.
    *
    *  @return  TRUE if the nodes were connected successfully, otherwise FALSE.
    *
    *  @see     WizardView::removeConnection
    */
   bool addConnection(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh);

   /**
    *  Creates all view line connections for a wizard node.
    *
    *  This method first removed any current connections for the given node, then creates
    *  new connection lines for all of the connected nodes.
    *
    *  @param   pNode
    *           The wizard node for which to create the line connections.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connection are created,
    *           otherwise FALSE.
    *
    *  @return  TRUE if the connection lines were successfully created, otherwise FALSE.
    *
    *  @see     WizardView::addConnection
    *  @see     WizardView::removeConnection
    */
   bool buildNodeConnections(WizardNode* pNode, bool bRefresh);

   /**
    *  Updates the canvas location of the given connection.
    *
    *  This method can be used to update the connected line location if a wizard item changes
    *  position.  Both the output and input node values are necessary to identify the desired
    *  connection line.
    *
    *  @param   pOutputNode
    *           The wizard node representing the output of a wizard item.  Cannot be NULL.
    *  @param   pInputNode
    *           The wizard node representing the input of a wizard item.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connection is repositioned,
    *           otherwise FALSE.
    *
    *  @return  TRUE if the connection line was successfully repositioned, otherwise FALSE.
    *
    *  @see     WizardView::updateConnectionPosition
    */
   bool updateConnectionPosition(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh);

   /**
    *  Updates the canvas location of all connections with the given canvas item.
    *
    *  @param   pWizardItem
    *           The canvas item for which to update all connection locations.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connections are repositioned,
    *           otherwise FALSE.
    *
    *  @return  TRUE if the connection lines were successfully repositioned, otherwise FALSE.
    *
    *  @see     WizardView::updateConnectionPosition
    */
   void updateConnectionPosition(WizardCanvasItem* pWizardItem, bool bRefresh);

   /**
    *  Removes a connection line between two wizard nodes.
    *
    *  @param   pLine
    *           The canvas line item.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connection is removed.
    *           FALSE if the canvas should not be redrawn.
    *
    *  @return  TRUE if the connection was successfully removed, FALSE otherwise.
    *
    *  @see     WizardView::removeConnection
    *  @see     WizardView::removeNodeConnections
    *  @see     WizardView::removeItemConnections
    */
   bool removeConnection(WizardLine* pLine, bool bRefresh);

   /**
    *  Removes a connection line between two wizard nodes.
    *
    *  @param   pOutputNode
    *           The output wizard node.  Cannot be NULL.
    *  @param   pInputNode
    *           The input wizard node.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connection is removed.
    *           FALSE if the canvas should not be redrawn.
    *
    *  @return  TRUE if the connection was successfully removed, FALSE otherwise.
    *
    *  @see     WizardView::removeConnection
    *  @see     WizardView::removeNodeConnections
    *  @see     WizardView::removeItemConnections
    */
   bool removeConnection(WizardNode* pOutputNode, WizardNode* pInputNode, bool bRefresh);

   /**
    *  Removes all connections for the given wizard node.
    *
    *  @param   pNode
    *           The wizard node for which to remove all connection lines.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connections are removed.
    *           FALSE if the canvas should not be redrawn.
    *
    *  @return  TRUE if the connections were successfully removed, FALSE otherwise.
    *
    *  @see     WizardView::removeConnection
    *  @see     WizardView::removeItemConnections
    */
   bool removeNodeConnections(WizardNode* pNode, bool bRefresh);

   /**
    *  Removes all connections for the given wizard canvas item.
    *
    *  @param   pWizardItem
    *           The wizard canvas item.  Cannot be NULL.
    *  @param   bRefresh
    *           TRUE if the canvas should be redrawn after the connections are removed.
    *           FALSE if the canvas should not be redrawn.
    *
    *  @return  TRUE if the connections were successfully removed, FALSE otherwise.
    *
    *  @see     WizardView::removeConnection
    *  @see     WizardView::removeNodeConnections
    */
   bool removeItemConnections(WizardCanvasItem* pWizardItem, bool bRefresh);

   /**
    *  Returns the canvas item for the given wizard item.
    *
    *  @param   pItem
    *           The wizard item.  Cannot be NULL.
    *
    *  @return  A pointer to the canvas item representing the wizard item.  NULL is returned
    *           if the given wizard item does not have a corresponding canvas item, or if an
    *           error occurred.
    *
    *  @see     WizardView::getCanvasItem
    */
   WizardCanvasItem* getCanvasItem(WizardItem* pItem) const;

   /**
    *  Returns the canvas item for the given wizard node.
    *
    *  @param   pNode
    *           The wizard node.  Cannot be NULL.
    *
    *  @return  A pointer to the canvas item representing the wizard item that contains the
    *           given node.  NULL is returned if the wizard node is not contained in a wizard
    *           item that has a corresponding canvas item, or if an error occurred.
    *
    *  @see     WizardView::getCanvasItem
    */
   WizardCanvasItem* getCanvasItem(WizardNode* pNode) const;

   /**
    *  Returns all canvas items in the wizard.
    *
    *  @return  A vector of wizard canvas item pointers.
    *
    *  @see     WizardView::getNumCanvasItems
    */
   std::vector<WizardCanvasItem*> getCanvasItems() const;

   /**
    *  Returns the total number of canvas items representing wizard items.
    *
    *  @return  The number of canvas items.
    *
    *  @see     WizardView::getCanvasItems
    */
   unsigned int getNumCanvasItems() const;

private:
   static int miItemZ;
   static int miLineZ;
   static int miNodeZ;

   Q3CanvasRectangle* mpDragRect;
   Q3CanvasLine* mpDragConnectLine;
   QPoint mptMoveStart;    // Viewport coordinates
   QPoint mptMoveValid;    // Contents coordinates
};

#endif
