/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QStyleOptionGraphicsItem>

#include "AppVerify.h"
#include "WizardLine.h"

WizardLine::WizardLine(WizardNode* pOutputNode, WizardNode* pInputNode, QGraphicsItem* pParent) :
   QGraphicsLineItem(pParent),
   mpInputNode(pInputNode),
   mpOutputNode(pOutputNode)
{
   setFlags(QGraphicsItem::ItemIsSelectable);
}

WizardLine::~WizardLine()
{}

void WizardLine::setInputPoint(const QPointF& scenePos)
{
   setLine(QLineF(line().p1(), scenePos));
}

void WizardLine::setOutputPoint(const QPointF& scenePos)
{
   setLine(QLineF(scenePos, line().p2()));
}

WizardNode* WizardLine::getInputNode() const
{
   return mpInputNode;
}

WizardNode* WizardLine::getOutputNode() const
{
   return mpOutputNode;
}

void WizardLine::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
   VERIFYNRV(pOption != NULL);

   // Disable the default selection rectangle
   bool selected = isSelected();

   QStyleOptionGraphicsItem option = *pOption;
   if (selected == true)
   {
      option.state &= ~(QStyle::State_Selected);
   }

   // Draw the line
   QGraphicsLineItem::paint(pPainter, &option, pWidget);

   // Draw the selection nodes
   if ((selected == true) && (pPainter != NULL))
   {
      pPainter->setBrush(Qt::yellow);
      pPainter->setPen(QPen(Qt::black, 1));

      const int nodeWidth = 6;
      pPainter->drawRect(line().x1() - (nodeWidth / 2), line().y1() - (nodeWidth / 2), nodeWidth, nodeWidth);
      pPainter->drawRect(line().x2() - (nodeWidth / 2), line().y2() - (nodeWidth / 2), nodeWidth, nodeWidth);
   }
}
