/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "WizardLine.h"

WizardLine::WizardLine(WizardNode* pOutputNode, WizardNode* pInputNode, Q3Canvas* canvas) :
   Q3CanvasLine(canvas)
{
   mpOutputNode = pOutputNode;
   mpInputNode = pInputNode;
}

WizardLine::~WizardLine()
{
}

void WizardLine::moveBy(double dX, double dY)
{
   // Prevent line from moving by doing nothing
}

void WizardLine::setInputPoint(int iX, int iY)
{
   setPoints(startPoint().x(), startPoint().y(), iX, iY);
}

void WizardLine::setOutputPoint(int iX, int iY)
{
   setPoints(iX, iY, endPoint().x(), endPoint().y());
}

WizardNode* WizardLine::getInputNode() const
{
   return mpInputNode;
}

WizardNode* WizardLine::getOutputNode() const
{
   return mpOutputNode;
}

void WizardLine::drawShape(QPainter& p)
{
   // Draw the line
   Q3CanvasLine::drawShape(p);

   // Draw the selection nodes
   if (isSelected() == true)
   {
      p.setBrush(Qt::yellow);
      p.setPen(QPen(Qt::black, 1));

      int iNodeWidth = 6;
      p.drawRect(startPoint().x() - (iNodeWidth / 2), startPoint().y() - (iNodeWidth / 2), iNodeWidth, iNodeWidth);
      p.drawRect(endPoint().x() - (iNodeWidth / 2), endPoint().y() - (iNodeWidth / 2), iNodeWidth, iNodeWidth);
   }
}
