/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDLINE_H
#define WIZARDLINE_H

#include <QtGui/QGraphicsLineItem>
#include <QtGui/QPainter>

class WizardNode;

/**
 *  Connection line for two WizardGraphicsItem objects.
 *
 *  The wizard builder allows for an input node on a wizard item to be connected
 *  to an output node on another wizard item using a WizardLine object.  Based
 *  on the QGraphicsLineItem class, the WizardLine class contains a starting
 *  point and an ending point based on the graphics scene coordinates, which
 *  correspond to an output point and input point respectively.  Moving the line
 *  directly is not allowed, and the line can only be moved by calling the
 *  setInputPoint() and setOutputPoint() methods.  The input and output points
 *  represents the connection location of the node on the wizard item.
 *
 *  The wizard nodes connected by the line are stored internally, so the line
 *  may be queried for its connected nodes with the getInputNode() and
 *  getOutputNode() methods.
 *
 *  @see   WizardGraphicsItem
 */
class WizardLine : public QGraphicsLineItem
{
public:
   /**
    *  Constructs a line connected between two wizard nodes.
    *
    *  @param   pOutputNode
    *           The output wizard node.
    *  @param   pInputNode
    *           The input wizard node.
    *  @param   pParent
    *           An optional parent graphics item.
    */
   WizardLine(WizardNode* pOutputNode, WizardNode* pInputNode, QGraphicsItem* pParent = NULL);

   /**
    *  Destructs the line.
    */
   virtual ~WizardLine();

   /**
    *  Repositions the line by establishing a new input node connection point.
    *
    *  This method updates the location of the line's ending point with the
    *  specified point.  It is typically called by the wizard view to update the
    *  position of the line when the item position changes.
    *
    *  @param   scenePos
    *           The new scene coordinate for the end of the line.
    *
    *  @see     setOutputPoint()
    */
   void setInputPoint(const QPointF& scenePos);

   /**
    *  Repositions the line by establishing a new output node connection point.
    *
    *  This method updates the location of the line's starting point with the
    *  specified point.  It is typically called by the wizard view to update the
    *  position of the line when the item position changes.
    *
    *  @param   scenePos
    *           The new scene coordinate for the start of the line.
    *
    *  @see     setInputPoint()
    */
   void setOutputPoint(const QPointF& scenePos);

   /**
    *  Retrieves the connected input node.
    *
    *  @return  A pointer to the connected input wizard node.
    *
    *  @see     getOutputNode()
    */
   WizardNode* getInputNode() const;

   /**
    *  Retrieves the connected output node.
    *
    *  @return  A pointer to the connected output wizard node.
    *
    *  @see     getInputNode()
    */
   WizardNode* getOutputNode() const;

   virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget = NULL);

private:
   WizardNode* mpInputNode;
   WizardNode* mpOutputNode;
};

#endif
