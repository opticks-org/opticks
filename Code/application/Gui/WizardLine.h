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

#include <Qt3Support/Q3Canvas>
#include <QtGui/QPainter>

#include "WizardNode.h"

/**
 *  Connection line for two WizardCanvasItem objects.
 *
 *  The wizard builder allows for an input node on a wizard item to be connected to an
 *  output node on another wizard item using a WizardLine object.  Based on the QCanvasLine
 *  class, the WizardLine class contains a starting point and an ending point based on the
 *  canvas coordinates, which correspond to an output point and input point respectively.
 *  Moving the line with the moveBy method() is not allowed, and the line can only be moved
 *  by calling the setInputPoint() and setOutputPoint() methods.  The input and output
 *  points represents the connection location of the node on the wizard item.
 *
 *  The wizard nodes connected by the line are stored internally, so the line may be
 *  queried for its connected nodes with the getInputNode() and getOutputNode() methods.
 *
 *  @see   WizardCanvasItem
 */
class WizardLine : public Q3CanvasLine
{
public:
   /**
    *  Constructs a line connected between two wizard nodes.
    *
    *  @param   pOutputNode
    *           The output wizard node.
    *  @param   pInputNode
    *           The input wizard node.
    *  @param   pCanvas
    *           The associated canvas object.
    *
    *  @see     WizardCanvasItem
    */
   WizardLine(WizardNode* pOutputNode, WizardNode* pInputNode, Q3Canvas* pCanvas);

   /**
    *  Destructs the line.
    */
   ~WizardLine();

   /**
    *  Overriden to prevent moving the line.
    *
    *  The normal QCanvasLine inplementation of moveBy() moves the line by the
    *  specified offset distance. This method is overriden with no implementation to
    *  make sure that only the connected items can move the line.
    *
    *  @param   dX
    *           The x-direction distance in which to move the item.
    *  @param   dY
    *           The y-direction distance in which to move the item.
    */
   void moveBy(double dX, double dY);

   /**
    *  Repositions the line by establishing a new input node connection point.
    *
    *  This method updates the location of the lines ending point with the specified
    *  point.  It is typically called by the wizard item to update the position of the
    *  line when the item itself changes location.
    *
    *  @param   iX
    *           The new canvas x-coordinate for the end of the line.
    *  @param   iY
    *           The new canvas y-coordinate for the end of the line.
    */
   void setInputPoint(int iX, int iY);

   /**
    *  Repositions the line by establishing a new output node connection point.
    *
    *  This method updates the location of the lines starting point with the specified
    *  point.  It is typically called by the wizard item to update the position of the
    *  line when the item itself changes location.
    *
    *  @param   iX
    *           The new canvas x-coordinate for the start of the line.
    *  @param   iY
    *           The new canvas y-coordinate for the start of the line.
    */
   void setOutputPoint(int iX, int iY);

   /**
    *  Retrieves the connected input node.
    *
    *  @return  A pointer to the connected input wizard node.
    *
    *  @see     WizardLine::getOutputNode
    */
   WizardNode* getInputNode() const;

   /**
    *  Retrieves the connected output node.
    *
    *  @return  A pointer to the connected output wizard node.
    *
    *  @see     WizardLine::getInputNode
    */
   WizardNode* getOutputNode() const;

protected:
   void drawShape(QPainter& p);

private:
   WizardNode* mpInputNode;
   WizardNode* mpOutputNode;
};

#endif
