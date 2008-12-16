/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>

#include "WizardCanvasItem.h"
#include "WizardItemImp.h"
#include "WizardNodeImp.h"
#include "PlugInManagerServicesImp.h"

#include <string>
#include <vector>
using namespace std;

QMap<QString, QColor> WizardCanvasItem::mNodeColors;

WizardCanvasItem::WizardCanvasItem(WizardItem* pItem, Q3Canvas* pCanvas) :
   Q3CanvasPolygonalItem(pCanvas),
   mpItem(static_cast<WizardItemImp*>(pItem)),
   mMargin(6),
   mRectWidth(80),
   mRectHeight(0),
   mBorderWidth(4),
   mColor(Qt::white),
   mOrder(0),
   mNodeDiameter(11),
   mNodeMargin(4),
   mpActiveNode(NULL),
   mTextWidth(0),
   mTextHeight(0),
   mSelectionNodeSize(6)
{
   mpDownButton = new QToolButton(NULL);
   mpDownButton->setWindowFlags(Qt::SubWindow);
   mpDownButton->setArrowType(Qt::DownArrow);
   mpDownButton->setFixedSize(mNodeDiameter, mNodeDiameter);

   updateDimensions();

   // Initialize the color map
   PlugInManagerServices* pManager = PlugInManagerServicesImp::instance();
   if (pManager != NULL)
   {
      const vector<string>& argTypes = pManager->getArgTypes();

      unsigned int numTypes = argTypes.size();
      if (mNodeColors.count() != (numTypes + 1))
      {
         mNodeColors.clear();

         // Set up the color map
         for (unsigned int i = 0; i < numTypes + 1; i++)
         {
            int r = (i % 5) * 64;
            int g = ((static_cast<int>(i) / 5) % 5) * 64;
            int b = static_cast<int>(i) / 25 * 64;

            if (r > 255)
            {
               r = 255;
            }

            if (g > 255)
            {
               g = 255;
            }

            if (b > 255)
            {
               b = 255;
            }

            QString strType = "Unknown";
            QColor clrType = Qt::white;

            if (i < numTypes)
            {
               strType = QString::fromStdString(argTypes.at(i));
               clrType.setRgb(r, g, b);
            }

            mNodeColors.insert(strType, clrType);

            // Create a pixmap for the wizard node
            QPixmap nodePix(mNodeDiameter + 4, mNodeDiameter);
            nodePix.fill(Qt::color0);

            QPainter p(&nodePix);
            p.setPen(Qt::NoPen);
            p.setBrush(clrType);
            p.drawRoundRect(0, 0, mNodeDiameter, mNodeDiameter, 99, 99);
            p.end();

            nodePix.setMask(nodePix.createHeuristicMask());
            Q3MimeSourceFactory::defaultFactory()->setPixmap(strType, nodePix);
         }
      }
   }
}

WizardCanvasItem::~WizardCanvasItem()
{
   delete mpDownButton;
   hide();
}

WizardItem* WizardCanvasItem::getWizardItem() const
{
   return mpItem;
}

void WizardCanvasItem::setColor(const QColor& clrItem)
{
   mColor = clrItem;
}

QColor WizardCanvasItem::getColor() const
{
   return mColor;
}

void WizardCanvasItem::setExecutionOrder(int iOrder)
{
   mOrder = iOrder;
}

int WizardCanvasItem::getExecutionOrder() const
{
   return mOrder;
}

void WizardCanvasItem::moveBy(double dX, double dY)
{
   setChanged();
   Q3CanvasPolygonalItem::moveBy(dX, dY);
   setChanged();
}

void WizardCanvasItem::setSelected(bool bSelected)
{
   setChanged();
   Q3CanvasPolygonalItem::setSelected(bSelected);
}

WizardNode* WizardCanvasItem::getNode(const QPoint& pos) const
{
   if (mpItem == NULL)
   {
      return NULL;
   }

   unsigned int i = 0;

   // Try the input nodes
   const vector<WizardNode*>& inputNodes = mpItem->getInputNodes();
   for (i = 0; i < inputNodes.size(); i++)
   {
      WizardNode* pNode = inputNodes[i];
      if (pNode != NULL)
      {
         QRect rcNode = getNodeRect(pNode);
         if (rcNode.contains(pos) == true)
         {
            return pNode;
         }

         QRect rcButton = getNodeToggleRect(pNode);
         if (rcButton.contains(pos) == true)
         {
            return pNode;
         }

         QPoint ptConnect = getNodeConnectionPoint(pNode);
         if (ptConnect == pos)
         {
            return pNode;
         }
      }
   }

   // Try the output nodes
   const vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();
   for (i = 0; i < outputNodes.size(); i++)
   {
      WizardNode* pNode = outputNodes[i];
      if (pNode != NULL)
      {
         QRect rcNode = getNodeRect(pNode);
         if (rcNode.contains(pos) == true)
         {
            return pNode;
         }

         QRect rcButton = getNodeToggleRect(pNode);
         if (rcButton.contains(pos) == true)
         {
            return pNode;
         }

         QPoint ptConnect = getNodeConnectionPoint(pNode);
         if (ptConnect == pos)
         {
            return pNode;
         }
      }
   }

   return NULL;
}

QColor WizardCanvasItem::getNodeColor(WizardNode* pNode) const
{
   QColor clrNode = Qt::white;
   if (pNode != NULL)
   {
      const string& nodeType = pNode->getType();
      if (nodeType.empty() == false)
      {
         QString strType = QString::fromStdString(nodeType);

         QMap<QString, QColor>::Iterator iter = mNodeColors.find(strType);
         if (iter != mNodeColors.end())
         {
            clrNode = iter.value();
         }
      }
   }

   return clrNode;
}

QRect WizardCanvasItem::getNodeRect(WizardNode* pNode) const
{
   QRect rcNode;
   if ((pNode == NULL) || (mpItem == NULL))
   {
      return rcNode;
   }

   int iIndex = getNodeIndex(pNode);
   if (iIndex == -1)
   {
      return rcNode;
   }

   int iX = 0;
   int iY = mNodeMargin;

   if (mRectWidth < mTextWidth)
   {
      iX = (mTextWidth / 2) - (mRectWidth / 2);
   }

   int iNodes = 0;

   // Adjust translation coordinates and get the number of nodes
   if (mpItem->isInputNode(pNode) == true)
   {
      iX += mNodeMargin;
      iNodes = mpItem->getNumInputNodes();
   }
   else if (mpItem->isOutputNode(pNode) == true)
   {
      iX += mRectWidth - 1 - mNodeMargin - mNodeDiameter;
      iNodes = mpItem->getNumOutputNodes();
   }

   for (int i = 0; i < iNodes; i++)
   {
      if (i == iIndex)
      {
         rcNode.setRect(static_cast<int>(x()) + iX, static_cast<int>(y()) + iY, mNodeDiameter, mNodeDiameter);
         break;
      }

      iY += mNodeDiameter + mNodeMargin;
   }

   return rcNode;
}

QRect WizardCanvasItem::getNodeToggleRect(WizardNode* pNode) const
{
   QRect rcButton;
   if ((pNode == NULL) || (mpItem == NULL))
   {
      return rcButton;
   }

   const vector<string>& validTypes = static_cast<WizardNodeImp*>(pNode)->getValidTypes();
   if (validTypes.size() <= 1)
   {
      return rcButton;
   }

   int iIndex = getNodeIndex(pNode);
   if (iIndex == -1)
   {
      return rcButton;
   }

   int iX = 0;
   int iY = mNodeMargin;

   if (mRectWidth < mTextWidth)
   {
      iX = (mTextWidth / 2) - (mRectWidth / 2);
   }

   int iNodes = 0;

   // Adjust translation coordinates and get the number of nodes
   if (mpItem->isInputNode(pNode) == true)
   {
      iX += mNodeMargin + mNodeDiameter + mNodeMargin;
      iNodes = mpItem->getNumInputNodes();
   }
   else if (mpItem->isOutputNode(pNode) == true)
   {
      iX += mRectWidth - 1 - (mNodeMargin * 2) - (mNodeDiameter * 2);
      iNodes = mpItem->getNumOutputNodes();
   }

   for (int i = 0; i < iNodes; i++)
   {
      if (i == iIndex)
      {
         rcButton.setRect(static_cast<int>(x()) + iX, static_cast<int>(y()) + iY, mNodeDiameter, mNodeDiameter);
         break;
      }

      iY += mNodeDiameter + mNodeMargin;
   }

   return rcButton;
}

QPoint WizardCanvasItem::getNodeConnectionPoint(WizardNode* pNode) const
{
   QPoint ptConnect;
   if ((pNode != NULL) && (mpItem != NULL))
   {
      QRect rcNode = getNodeRect(pNode);
      if (rcNode.isValid() == true)
      {
         if (mpItem->isInputNode(pNode) == true)
         {
            ptConnect.setX(rcNode.left());
         }
         else if (mpItem->isOutputNode(pNode) == true)
         {
            ptConnect.setX(rcNode.right());
         }

         ptConnect.setY(rcNode.center().y());
      }
   }

   return ptConnect;
}

void WizardCanvasItem::setActiveNode(WizardNode* pNode)
{
   if (pNode != mpActiveNode)
   {
      mpActiveNode = pNode;
      setChanged();

      Q3Canvas* pCanvas = canvas();
      if (pCanvas != NULL)
      {
         pCanvas->update();
      }
   }
}

WizardNode* WizardCanvasItem::getActiveNode() const
{
   return mpActiveNode;
}

Q3PointArray WizardCanvasItem::areaPoints() const
{
   Q3PointArray points(8);

   int iX = static_cast<int>(x());
   int iY = static_cast<int>(y());

   if (mRectWidth > mTextWidth)
   {
      points[0] = QPoint(iX - mMargin, iY - mMargin);
      points[1] = QPoint(iX + mRectWidth + mMargin, iY - mMargin);
      points[2] = QPoint(iX + mRectWidth + mMargin, iY + mRectHeight + mMargin);
      points[3] = QPoint(iX + (mRectWidth / 2) + (mTextWidth / 2) + mMargin, iY + mRectHeight + mMargin);
      points[4] = QPoint(iX + (mRectWidth / 2) + (mTextWidth / 2) + mMargin, iY + mRectHeight + mTextHeight + mMargin);
      points[5] = QPoint(iX + (mRectWidth / 2) - (mTextWidth / 2) - mMargin, iY + mRectHeight + mTextHeight + mMargin);
      points[6] = QPoint(iX + (mRectWidth / 2) - (mTextWidth / 2) - mMargin, iY + mRectHeight + mMargin);
      points[7] = QPoint(iX - mMargin, iY + mRectHeight + mMargin);
   }
   else
   {
      points[0] = QPoint(iX + (mTextWidth / 2) - (mRectWidth / 2) - mMargin, iY - mMargin);
      points[1] = QPoint(iX + (mTextWidth / 2) + (mRectWidth / 2) + mMargin, iY - mMargin);
      points[2] = QPoint(iX + (mTextWidth / 2) + (mRectWidth / 2) + mMargin, iY + mRectHeight - mMargin);
      points[3] = QPoint(iX + mTextWidth + mMargin, iY + mRectHeight - mMargin);
      points[4] = QPoint(iX + mTextWidth + mMargin, iY + mRectHeight + mTextHeight + mMargin);
      points[5] = QPoint(iX - mMargin, iY + mRectHeight + mTextHeight + mMargin);
      points[6] = QPoint(iX - mMargin, iY + mRectHeight - mMargin);
      points[7] = QPoint(iX + (mTextWidth / 2) - (mRectWidth / 2) - mMargin, iY + mRectHeight - mMargin);
   }

   return points;
}

void WizardCanvasItem::drawShape(QPainter& p)
{
   // Compute the rectangle translation coordinates
   int iX = 0;
   int iY = 0;
   int iCenterX = mRectWidth / 2;

   if (mRectWidth < mTextWidth)
   {
      iX = (mTextWidth / 2) - (mRectWidth / 2);
      iCenterX = mTextWidth / 2;
   }

   // Draw the rectangle
   QColor clrBorder = Qt::darkGray;
   QColor clrItem = Qt::lightGray;

   bool bSupported = false;
   if (mpItem != NULL)
   {
      bSupported = mpItem->isCurrentModeSupported();
      if (bSupported == true)
      {
         clrBorder = Qt::black;
         clrItem = mColor;

         if (mpItem->getBatchMode() == true)
         {
            clrBorder = Qt::red;
         }
      }
      else
      {
         if (mpItem->getBatchMode() == true)
         {
            clrBorder = Qt::darkRed;
         }
      }
   }

   p.setBrush(clrItem);
   p.setPen(QPen(QBrush(clrBorder), mBorderWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
   p.drawRect(static_cast<int>(x()) + iX, static_cast<int>(y()) + iY, mRectWidth, mRectHeight);

   // Draw the nodes
   int iNumInputNodes = 0;
   int iNumOutputNodes = 0;
   if (mpItem != NULL)
   {
      iNumInputNodes = mpItem->getNumInputNodes();
      iNumOutputNodes = mpItem->getNumOutputNodes();
   }

   int i = 0;
   for (i = 0; i < (iNumInputNodes + iNumOutputNodes); i++)
   {
      WizardNode* pNode = NULL;

      // Get the node color
      QColor clrNode = Qt::white;
      if (i < iNumInputNodes)
      {
         const vector<WizardNode*>& inputNodes = mpItem->getInputNodes();

         pNode = inputNodes.at(i);
         if (pNode != NULL)
         {
            clrNode = getNodeColor(pNode);
         }
      }
      else
      {
         const vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();

         pNode = outputNodes.at(i - iNumInputNodes);
         if (pNode != NULL)
         {
            clrNode = getNodeColor(pNode);
         }
      }

      // Get the node rect
      QRect rcNode = getNodeRect(pNode);

      // Draw the node
      if (clrNode == clrItem)
      {
         p.setPen(Qt::black);
      }
      else
      {
         p.setPen(Qt::NoPen);
      }

      p.setBrush(clrNode);
      p.drawRoundRect(rcNode, 99, 99);

      // Down button
      if (pNode != NULL)
      {
         const vector<string>& validTypes = static_cast<WizardNodeImp*>(pNode)->getValidTypes();
         if (validTypes.size() > 1)
         {
            if (pNode == mpActiveNode)
            {
               mpDownButton->setDown(true);
            }

            QRect rcButton = getNodeToggleRect(pNode);
            QPixmap pixButton = QPixmap::grabWidget(mpDownButton);
            p.drawPixmap(rcButton, pixButton);

            mpDownButton->setDown(false);
         }
      }
   }

   // Draw the execution order
   QString strOrder = QString::number(mOrder);

   QFontMetrics ftMetrics = qApp->fontMetrics();
   QRect rcText = ftMetrics.boundingRect(strOrder);

   iX = iCenterX - (rcText.width() / 2) /*- 1*/;
   iY = rcText.height() + 1;

   if (bSupported == false)
   {
      p.setPen(Qt::white);
      p.drawText(static_cast<int>(x()) + iX + 1, static_cast<int>(y()) + iY + 1, strOrder);
      p.setPen(Qt::darkGray);
   }

   p.setPen(Qt::black);
   p.drawText(static_cast<int>(x()) + iX, static_cast<int>(y()) + iY, strOrder);

   // Compute the label translation coordinates
   iX = 0;
   iY = mRectHeight + mTextHeight;

   if (mRectWidth > mTextWidth)
   {
      iX = (mRectWidth / 2) - (mTextWidth / 2);
   }

   // Draw the item label
   if (mpItem != NULL)
   {
      const string& itemName = mpItem->getName();
      if (itemName.empty() == false)
      {
         p.drawText(static_cast<int>(x()) + iX, static_cast<int>(y()) + iY, QString::fromStdString(itemName));
      }
   }

   // Draw the selection nodes
   if (isSelected() == true)
   {
      QRect rcItem = boundingRect();

      QPolygon points(8);
      points[0] = rcItem.topLeft();
      points[1] = QPoint(rcItem.center().x(), rcItem.top());
      points[2] = rcItem.topRight();
      points[3] = QPoint(rcItem.right(), rcItem.center().y());
      points[4] = rcItem.bottomLeft();
      points[5] = QPoint(rcItem.center().x(), rcItem.bottom());
      points[6] = rcItem.bottomRight();
      points[7] = QPoint(rcItem.left(), rcItem.center().y());

      p.setBrush(Qt::white);
      p.setPen(QPen(Qt::black, 1));

      for (i = 0; i < 8; i++)
      {
         int iWidth = 6;
         iX = points[i].x() - (iWidth / 2);
         iY = points[i].y() - (iWidth / 2);

         p.drawRect(iX, iY, iWidth, iWidth);
      }
   }
}

int WizardCanvasItem::getNodeIndex(WizardNode* pNode) const
{
   if (mpItem == NULL)
   {
      return -1;
   }

   int iCount = 0;
   int i = 0;

   // Try the input nodes
   const vector<WizardNode*>& inputNodes = mpItem->getInputNodes();

   iCount = inputNodes.size();
   for (i = 0; i < iCount; i++)
   {
      WizardNode* pCurrentNode = inputNodes[i];
      if ((pCurrentNode == pNode) && (pCurrentNode != NULL))
      {
         return i;
      }
   }

   // Try the output nodes
   const vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();

   iCount = outputNodes.size();
   for (i = 0; i < iCount; i++)
   {
      WizardNode* pCurrentNode = outputNodes[i];
      if ((pCurrentNode == pNode) && (pCurrentNode != NULL))
      {
         return i;
      }
   }

   return -1;
}

void WizardCanvasItem::setChanged()
{
   Q3Canvas* pCanvas = canvas();
   if (pCanvas != NULL)
   {
      QRect rcItem = boundingRect();
      rcItem.setLeft(rcItem.left() - mSelectionNodeSize);
      rcItem.setTop(rcItem.top() - mSelectionNodeSize);
      rcItem.setRight(rcItem.right() + mSelectionNodeSize);
      rcItem.setBottom(rcItem.bottom() + mSelectionNodeSize);

      pCanvas->setChanged(rcItem);
   }
}

void WizardCanvasItem::updateDimensions()
{
   if (mpItem == NULL)
   {
      return;
   }

   setChanged();

   // Rectangle height
   int iNumInputNodes = mpItem->getNumInputNodes();
   int iNumOutputNodes = mpItem->getNumOutputNodes();

   int iCount = iNumInputNodes;
   if (iNumOutputNodes > iNumInputNodes)
   {
      iCount = iNumOutputNodes;
   }

   // Ensure space for execution order text
   if (iCount < 1)
   {
      iCount = 1;
   }

   mRectHeight = iCount * (mNodeDiameter + mNodeMargin) + mNodeMargin + 1;

   // Text width and height
   QString strName;

   string itemName = mpItem->getName();
   if (itemName.empty() == false)
   {
      strName = QString::fromStdString(itemName);
   }

   QFontMetrics ftMetrics = qApp->fontMetrics();
   QRect rcName = ftMetrics.boundingRect(strName);

   mTextWidth = rcName.width();
   mTextHeight = rcName.height();

   setChanged();
}
