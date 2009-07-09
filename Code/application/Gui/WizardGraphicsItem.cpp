/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtGui/QBitmap>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPainter>

#include "PlugInManagerServices.h"
#include "WizardGraphicsItem.h"
#include "WizardNodeImp.h"

#include <string>
#include <vector>
using namespace std;

const double WizardGraphicsItem::sRectWidth = 80.0;
const double WizardGraphicsItem::sBorderWidth = 4.0;
const double WizardGraphicsItem::sNodeDiameter = 11.0;
const double WizardGraphicsItem::sNodeMargin = 4.0;
QMap<QString, QColor> WizardGraphicsItem::sNodeColors;
const double WizardGraphicsItem::sTextMargin = 2.0;
const double WizardGraphicsItem::sSelectionNodeWidth = 6.0;

WizardGraphicsItem::WizardGraphicsItem(WizardItem* pItem, QGraphicsItem* pParent) :
   QGraphicsItemGroup(pParent),
   mpItem(static_cast<WizardItemImp*>(pItem)),
   mpRect(NULL),
   mpOrder(NULL),
   mpDownButton(NULL),
   mpActiveNode(NULL),
   mpText(NULL)
{
   // Initialize the color map
   Service<PlugInManagerServices> pManager;
   const vector<string>& argTypes = pManager->getArgTypes();

   unsigned int numTypes = argTypes.size();
   if (sNodeColors.count() != (numTypes + 1))
   {
      sNodeColors.clear();

      // Set up the color map
      for (unsigned int i = 0; i < numTypes + 1; ++i)
      {
         int red = std::min((static_cast<int>(i) % 5) * 64, 255);
         int green = std::min(((static_cast<int>(i) / 5) % 5) * 64, 255);
         int blue = std::min(static_cast<int>(i) / 25 * 64, 255);

         QString nodeType = "Unknown";
         QColor nodeColor = Qt::white;

         if (i < numTypes)
         {
            nodeType = QString::fromStdString(argTypes.at(i));
            nodeColor.setRgb(red, green, blue);
         }

         sNodeColors.insert(nodeType, nodeColor);

         // Store a pixmap for the wizard node to use in the tool tip
         QPixmap nodePix = getNodePixmap(nodeType);
         Q3MimeSourceFactory::defaultFactory()->setPixmap(nodeType, nodePix);
      }
   }

   // Rectangle
   mpRect = new QGraphicsRectItem(this);
   mpOrder = new QGraphicsSimpleTextItem(mpRect);

   QColor itemColor = Qt::white;
   if (mpItem.get() != NULL)
   {
      itemColor = getItemBackgroundColor(QString::fromStdString(mpItem->getType()));
   }

   mpRect->setBrush(QBrush(itemColor));
   updateBorder();

   // Text
   mpText = new QGraphicsSimpleTextItem(this);

   QString itemName;
   if (mpItem.get() != NULL)
   {
      itemName = QString::fromStdString(mpItem->getName());
   }

   mpText->setText(itemName);

   // Down arrow button
   mpDownButton = new QToolButton(NULL);
   mpDownButton->setWindowFlags(Qt::SubWindow);
   mpDownButton->setArrowType(Qt::DownArrow);
   mpDownButton->setFixedSize(sNodeDiameter, sNodeDiameter);
   mpDownButton->setDown(false);

   updateNodes();
   addToGroup(mpRect);
   addToGroup(mpText);
   setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

   // Connections
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, Renamed), Slot(this, &WizardGraphicsItem::itemRenamed));
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, PositionChanged), Slot(this, &WizardGraphicsItem::itemPositionChanged));
   mpItem.addSignal(SIGNAL_NAME(WizardItemImp, BatchModeChanged),
      Slot(this, &WizardGraphicsItem::itemBatchModeChanged));
}

WizardGraphicsItem::~WizardGraphicsItem()
{
   for (map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter = mNodes.begin();
      iter != mNodes.end();
      ++iter)
   {
      WizardNodeImp* pNode = static_cast<WizardNodeImp*>(iter->first);
      if (pNode != NULL)
      {
         VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, Renamed), Slot(this, &WizardGraphicsItem::nodeRenamed)));
         VERIFYNR(pNode->detach(SIGNAL_NAME(WizardNodeImp, TypeChanged),
            Slot(this, &WizardGraphicsItem::nodeTypeChanged)));
         VERIFYNR(pNode->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardGraphicsItem::nodeDeleted)));
      }

      // The ellipse and pixmap graphics items are destroyed as children of this QGraphicsItem
   }

   delete mpDownButton;
   hide();
}

WizardItem* WizardGraphicsItem::getWizardItem()
{
   return mpItem.get();
}

const WizardItem* WizardGraphicsItem::getWizardItem() const
{
   return mpItem.get();
}

void WizardGraphicsItem::setExecutionOrder(int iOrder)
{
   if (iOrder == getExecutionOrder())
   {
      return;
   }

   // Update the number text
   mpOrder->setText(QString::number(iOrder));

   // Update the text position in the rectangle
   QFontMetricsF nameMetrics(mpText->font());
   QRectF nameRect = nameMetrics.boundingRect(mpText->text());
   double nameWidth = nameRect.width();

   QFontMetricsF orderMetrics(mpOrder->font());
   QRectF orderRect = orderMetrics.boundingRect(mpOrder->text());
   double orderWidth = orderRect.width();

   double xOffset = 0.0;
   if (sRectWidth < nameWidth)
   {
      xOffset = (nameWidth / 2.0) - (orderWidth / 2.0);
   }
   else
   {
      xOffset = (sRectWidth / 2.0) - (orderWidth / 2.0);
   }

   mpOrder->setPos(xOffset, sTextMargin);
}

int WizardGraphicsItem::getExecutionOrder() const
{
   QString orderText = mpOrder->text();
   if (orderText.isEmpty() == false)
   {
      return orderText.toInt();
   }

   return -1;
}

WizardNode* WizardGraphicsItem::getNode(const QPointF& scenePos) const
{
   if (mpItem.get() == NULL)
   {
      return NULL;
   }

   QPointF itemPos = mapFromScene(scenePos);
   const vector<WizardNode*>& inputNodes = mpItem->getInputNodes();
   const vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();

   for (vector<WizardNode*>::size_type i = 0; i < inputNodes.size() + outputNodes.size(); ++i)
   {
      WizardNode* pNode = NULL;
      if (i < inputNodes.size())
      {
         pNode = inputNodes[i];
      }
      else
      {
         pNode = outputNodes[i - inputNodes.size()];
      }

      if (pNode != NULL)
      {
         QRectF nodeRect = getNodeRect(pNode);
         if (nodeRect.contains(itemPos) == true)
         {
            return pNode;
         }

         QPointF connectPos = getNodeConnectionPoint(pNode);
         if (connectPos == scenePos)
         {
            return pNode;
         }
      }
   }

   return NULL;
}

QRectF WizardGraphicsItem::getNodeRect(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return QRectF();
   }

   map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::const_iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QGraphicsEllipseItem* pEllipse = iter->second.first;
      if (pEllipse != NULL)
      {
         return pEllipse->rect();
      }
   }

   return QRectF();
}

QPointF WizardGraphicsItem::getNodeConnectionPoint(WizardNode* pNode) const
{
   QPointF scenePos;
   if ((pNode != NULL) && (mpItem.get() != NULL))
   {
      QRectF nodeRect = getNodeRect(pNode);
      if (nodeRect.isValid() == true)
      {
         QPointF itemPos;
         if (mpItem->isInputNode(pNode) == true)
         {
            itemPos.setX(nodeRect.left());
         }
         else if (mpItem->isOutputNode(pNode) == true)
         {
            itemPos.setX(nodeRect.right());
         }

         itemPos.setY(nodeRect.center().y());
         scenePos = mapToScene(itemPos);
      }
   }

   return scenePos;
}

QRectF WizardGraphicsItem::boundingRect() const
{
   QRectF itemRect = childrenBoundingRect();
   itemRect.adjust(-(sSelectionNodeWidth + 1.0), -(sSelectionNodeWidth + 1.0),
      sSelectionNodeWidth, sSelectionNodeWidth);

   return itemRect;
}

QPainterPath WizardGraphicsItem::shape() const
{
   QPolygonF rectPolygon = mapFromItem(mpRect, mpRect->boundingRect());
   QPolygonF textPolygon = mapFromItem(mpText, mpText->boundingRect());

   QPainterPath path;
   path.addPolygon(rectPolygon);
   path.addPolygon(textPolygon);
   return path;
}

void WizardGraphicsItem::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
   // Draw the item
   QGraphicsItemGroup::paint(pPainter, pOption, pWidget);

   // Draw the selection nodes
   if ((isSelected() == true) && (pPainter != NULL))
   {
      QRectF itemRect = boundingRect();
      const int numSelectionNodes = 8;

      QPolygonF points(numSelectionNodes);
      points[0] = itemRect.topLeft();
      points[1] = QPointF(itemRect.center().x() - (sSelectionNodeWidth / 2.0), itemRect.top());
      points[2] = QPointF(itemRect.right() - sSelectionNodeWidth, itemRect.top());
      points[3] = QPointF(itemRect.right() - sSelectionNodeWidth, itemRect.center().y() - (sSelectionNodeWidth / 2.0));
      points[4] = QPointF(itemRect.right() - sSelectionNodeWidth, itemRect.bottom() - sSelectionNodeWidth);
      points[5] = QPointF(itemRect.center().x() - (sSelectionNodeWidth / 2.0), itemRect.bottom() - sSelectionNodeWidth);
      points[6] = QPointF(itemRect.left(), itemRect.bottom() - sSelectionNodeWidth);
      points[7] = QPointF(itemRect.left(), itemRect.center().y() - (sSelectionNodeWidth / 2.0));

      pPainter->setBrush(Qt::white);
      pPainter->setPen(QPen(Qt::black, 1));

      for (int i = 0; i < numSelectionNodes; ++i)
      {
         QRectF selectionNodeRect(points[i].x(), points[i].y(), sSelectionNodeWidth, sSelectionNodeWidth);
         pPainter->drawRect(selectionNodeRect);
      }
   }
}

QColor WizardGraphicsItem::getNodeColor(WizardNode* pNode)
{
   QColor nodeColor = Qt::white;
   if (pNode != NULL)
   {
      QString nodeType = QString::fromStdString(pNode->getType());
      nodeColor = getNodeColor(nodeType);
   }

   return nodeColor;
}

QColor WizardGraphicsItem::getNodeColor(const QString& nodeType)
{
   QColor nodeColor = Qt::white;
   if (nodeType.isEmpty() == false)
   {
      QMap<QString, QColor>::iterator iter = sNodeColors.find(nodeType);
      if (iter != sNodeColors.end())
      {
         nodeColor = iter.value();
      }
   }

   return nodeColor;
}

QPixmap WizardGraphicsItem::getNodePixmap(WizardNode* pNode)
{
   QPixmap nodePix;
   if (pNode != NULL)
   {
      QString nodeType = QString::fromStdString(pNode->getType());
      nodePix = getNodePixmap(nodeType);
   }

   return nodePix;
}

QPixmap WizardGraphicsItem::getNodePixmap(const QString& nodeType)
{
   QPixmap nodePix(sNodeDiameter + 4, sNodeDiameter);
   nodePix.fill(Qt::color0);

   QColor nodeColor = getNodeColor(nodeType);

   QPainter p(&nodePix);
   if (nodeColor == Qt::white)
   {
      p.setPen(Qt::black);
   }
   else
   {
      p.setPen(Qt::NoPen);
   }

   p.setBrush(nodeColor);
   p.drawRoundRect(0, 0, sNodeDiameter, sNodeDiameter, 99, 99);
   p.end();

   nodePix.setMask(nodePix.createHeuristicMask());
   return nodePix;
}

QColor WizardGraphicsItem::getItemBackgroundColor(const QString& itemType)
{
   QColor itemColor;
   string itemTypeString = itemType.toStdString();

   if (itemTypeString == PlugInManagerServices::AlgorithmType())
   {
      itemColor.setRgb(200, 255, 200);
   }
   else if (itemTypeString == PlugInManagerServices::ExporterType())
   {
      itemColor.setRgb(200, 200, 255);
   }
   else if (itemTypeString == PlugInManagerServices::GeoreferenceType())
   {
      itemColor.setRgb(200, 255, 255);
   }
   else if (itemTypeString == PlugInManagerServices::ImporterType())
   {
      itemColor.setRgb(255, 200, 200);
   }
   else if (itemTypeString == PlugInManagerServices::InterpreterType())
   {
      itemColor.setRgb(255, 225, 175);
   }
   else if (itemTypeString == PlugInManagerServices::ViewerType())
   {
      itemColor.setRgb(175, 225, 255);
   }
   else if (itemTypeString == PlugInManagerServices::WizardType())
   {
      itemColor.setRgb(225, 175, 225);
   }
   else
   {
      itemColor.setRgb(255, 255, 200);
   }

   return itemColor;
}

void WizardGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QPointF itemPos = pEvent->pos();

      // Update the button pixmap if necessary
      if (pEvent->button() == Qt::LeftButton)
      {
         map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::const_iterator iter;
         for (iter = mNodes.begin(); iter != mNodes.end(); ++iter)
         {
            QGraphicsPixmapItem* pPixmap = iter->second.second;
            if (pPixmap != NULL)
            {
               QPointF location = pPixmap->pos();
               QPixmap pixmap = pPixmap->pixmap();

               QRectF buttonRect(location, QSizeF(pixmap.size()));
               if (buttonRect.contains(itemPos) == true)
               {
                  // Set the active node member
                  mpActiveNode = iter->first;

                  // Update the button pixmap
                  mpDownButton->setDown(true);

                  QPixmap pixButton = QPixmap::grabWidget(mpDownButton);
                  pPixmap->setPixmap(pixButton);

                  mpDownButton->setDown(false);
               }
            }
         }
      }
   }

   QGraphicsItemGroup::mousePressEvent(pEvent);
}

void WizardGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent)
{
   // Do not move the item if the user is clicking a button pixmap
   if (mpActiveNode == NULL)
   {
      QGraphicsItemGroup::mouseMoveEvent(pEvent);
   }
}

void WizardGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent)
{
   if ((pEvent != NULL) && (mpActiveNode != NULL))
   {
      map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter =
         mNodes.find(mpActiveNode);
      if (iter != mNodes.end())
      {
         // Update the button pixmap
         QGraphicsPixmapItem* pPixmap = iter->second.second;
         if (pPixmap != NULL)
         {
            QPixmap pixButton = QPixmap::grabWidget(mpDownButton);
            pPixmap->setPixmap(pixButton);
         }

         // Break the node connections
         WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(mpActiveNode);
         pNodeImp->clearConnectedNodes();

         // Update the current type
         const string& type = mpActiveNode->getType();
         const vector<string>& validTypes = mpActiveNode->getValidTypes();

         unsigned int index = 0;
         for (unsigned int i = 0; i < (validTypes.size() - 1); ++i)
         {
            string currentType = validTypes.at(i);
            if (currentType == type)
            {
               index = i + 1;
               break;
            }
         }

         string newType = validTypes.at(index);
         if ((newType.empty() == false) && (newType != type))
         {
            pNodeImp->setType(newType);
         }
      }

      mpActiveNode = NULL;
      return;
   }

   QGraphicsItemGroup::mouseReleaseEvent(pEvent);
}

QVariant WizardGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
   if (change == QGraphicsItem::ItemPositionHasChanged)
   {
      if (mpItem.get() != NULL)
      {
         QPointF scenePos = value.toPointF();
         mpItem->setPosition(scenePos.x(), scenePos.y());
      }
   }
   else if (change == QGraphicsItem::ItemSelectedHasChanged)
   {
      emit selectionChanged();
   }

   return QGraphicsItemGroup::itemChange(change, value);
}

void WizardGraphicsItem::itemRenamed(Subject& subject, const string& signal, const boost::any& data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);
   VERIFYNRV(pItem == mpItem.get());

   QString itemName = QString::fromStdString(boost::any_cast<string>(data));
   if (itemName != mpText->text())
   {
      // Set the text
      mpText->setText(itemName);

      // Update the position of the rectangle and text items
      updateGeometry();
   }
}

void WizardGraphicsItem::itemPositionChanged(Subject& subject, const string& signal, const boost::any& data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);
   VERIFYNRV(pItem == mpItem.get());

   LocationType position = boost::any_cast<LocationType>(data);
   if ((position.mX != x()) || (position.mY != y()))
   {
      setPos(position.mX, position.mY);
   }
}

void WizardGraphicsItem::itemBatchModeChanged(Subject& subject, const string& signal, const boost::any& data)
{
   WizardItemImp* pItem = dynamic_cast<WizardItemImp*>(&subject);
   VERIFYNRV(pItem != NULL);
   VERIFYNRV(pItem == mpItem.get());

   updateBorder();
   updateNodes();
}

void WizardGraphicsItem::nodeRenamed(Subject& subject, const string& signal, const boost::any& data)
{
   WizardNode* pNode = dynamic_cast<WizardNode*>(&subject);
   VERIFYNRV(pNode != NULL);

   // Update the node tool tip to contain the new name
   map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QGraphicsEllipseItem* pEllipse = iter->second.first;
      if (pEllipse != NULL)
      {
         QString tipText = getNodeToolTip(pNode);
         pEllipse->setToolTip(tipText);
      }
   }
}

void WizardGraphicsItem::nodeTypeChanged(Subject& subject, const string& signal, const boost::any& data)
{
   WizardNode* pNode = dynamic_cast<WizardNode*>(&subject);
   VERIFYNRV(pNode != NULL);

   // Update the node item properties
   map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      QGraphicsEllipseItem* pEllipse = iter->second.first;
      if (pEllipse != NULL)
      {
         // Pen
         QColor nodeColor = getNodeColor(pNode);
         QColor itemColor = mpRect->brush().color();

         if (nodeColor == itemColor)
         {
            pEllipse->setPen(QPen(Qt::black));
         }
         else
         {
            pEllipse->setPen(QPen(Qt::NoPen));
         }

         // Brush
         pEllipse->setBrush(QBrush(nodeColor));

         // Tool tip
         QString tipText = getNodeToolTip(pNode);
         pEllipse->setToolTip(tipText);
      }
   }
}

void WizardGraphicsItem::nodeDeleted(Subject& subject, const string& signal, const boost::any& data)
{
   WizardNode* pNode = dynamic_cast<WizardNode*>(&subject);
   VERIFYNRV(pNode != NULL);

   map<WizardNode*, pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter = mNodes.find(pNode);
   if (iter != mNodes.end())
   {
      WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(iter->first);
      VERIFYNRV(pNodeImp != NULL);

      // Remove the graphics items
      QGraphicsEllipseItem* pEllipse = iter->second.first;
      if (pEllipse != NULL)
      {
         delete pEllipse;
      }

      QGraphicsPixmapItem* pPixmap = iter->second.second;
      if (pPixmap != NULL)
      {
         delete pPixmap;
      }

      // Remove the node from the map
      mNodes.erase(iter);
   }
}

QString WizardGraphicsItem::getNodeToolTip(WizardNode* pNode) const
{
   if (pNode == NULL)
   {
      return QString();
   }

   QString name = QString::fromStdString(pNode->getName());

   QString originalType = QString::fromStdString(pNode->getOriginalType());
   if (originalType.isEmpty() == false)
   {
      QString imageType = originalType;
      if (Q3MimeSourceFactory::defaultFactory()->data(originalType) == NULL)
      {
         imageType = "Unknown";
      }

      imageType.replace("<", "&lt;");
      imageType.replace(">", "&gt;");
      originalType.replace("<", "&lt;");
      originalType.replace(">", "&gt;");

      originalType = "<img source=\"" + imageType + "\">" + originalType;
   }

   QString type = QString::fromStdString(pNode->getType());
   if (type.isEmpty() == false)
   {
      QString imageType = type;
      if (Q3MimeSourceFactory::defaultFactory()->data(type) == NULL)
      {
         imageType = "Unknown";
      }

      imageType.replace("<", "&lt;");
      imageType.replace(">", "&gt;");
      type.replace("<", "&lt;");
      type.replace(">", "&gt;");

      type = "<img source=\"" + imageType + "\">" + type;
   }

   QStringList validTypes;

   const vector<string>& validTypesVec = pNode->getValidTypes();
   for (vector<string>::const_iterator iter = validTypesVec.begin(); iter != validTypesVec.end(); ++iter)
   {
      QString validType = QString::fromStdString(*iter);
      if (validType.isEmpty() == false)
      {
         QString imageType = validType;
         if (Q3MimeSourceFactory::defaultFactory()->data(validType) == NULL)
         {
            imageType = "Unknown";
         }

         imageType.replace("<", "&lt;");
         imageType.replace(">", "&gt;");
         validType.replace("<", "&lt;");
         validType.replace(">", "&gt;");

         validType = "<img source=\"" + imageType + "\">" + validType;
         validTypes.append(validType);
      }
   }

   QString description = QString::fromStdString(pNode->getDescription());
   if (description.isEmpty() == true)
   {
      description = "&nbsp;";
   }

   QString tipText = "<qt><table width=235 cellspacing=0>"
      "<tr><td width=90><b>Name:</b></td><td width=145>" + name + "</td></tr>"
      "<tr><td width=90><b>Type:</b></td><td width=145>" + originalType + "</td></tr>"
      "<tr><td width=90><b>Description:</b></td><td width=145>" + description + "</td></tr>"
      "</table>"
      "<hr>"
      "<table width=235 cellspacing=0>"
      "<tr><td width=90><b>Current Type:</b></td><td width=145>" + type + "</td></tr>"
      "<tr><td width=90><b>Valid Types:</b></td><td width=145>" + validTypes.join("<br>") + "</td></tr>"
      "</table></qt>";

   return tipText;
}

void WizardGraphicsItem::updateGeometry()
{
   // Rectangle size
   int numInputNodes = mpItem->getNumInputNodes();
   int numOutputNodes = mpItem->getNumOutputNodes();

   int numNodes = std::max(numInputNodes, numOutputNodes);
   numNodes = std::max(numNodes, 1);

   double rectHeight = numNodes * (sNodeDiameter + sNodeMargin) + sNodeMargin + 1;

   // Name size
   QFontMetricsF nameMetrics(mpText->font());
   QRectF nameRect = nameMetrics.boundingRect(mpText->text());
   double nameWidth = nameRect.width();

   // Order size
   QFontMetricsF orderMetrics(mpOrder->font());
   QRectF orderRect = orderMetrics.boundingRect(mpOrder->text());
   double orderWidth = orderRect.width();

   // Update the item rectangle and text positions
   prepareGeometryChange();
   if (sRectWidth < nameWidth)
   {
      mpRect->setRect((nameWidth / 2.0) - (sRectWidth / 2.0), 0.0, sRectWidth, rectHeight);
      mpOrder->setPos((nameWidth / 2.0) - (orderWidth / 2.0), sTextMargin);
      mpText->setPos(0.0, rectHeight + sTextMargin);
   }
   else
   {
      mpRect->setRect(0.0, 0.0, sRectWidth, rectHeight);
      mpOrder->setPos((sRectWidth / 2.0) - (orderWidth / 2.0), sTextMargin);
      mpText->setPos((sRectWidth / 2.0) - (nameWidth / 2.0), rectHeight + sTextMargin);
   }

   // Update the wizard node and button positions
   map<WizardNode*, std::pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> >::iterator iter;
   for (iter = mNodes.begin(); iter != mNodes.end(); ++iter)
   {
      WizardNode* pNode = iter->first;

      int nodeIndex = mpItem->getNodeIndex(pNode);
      if (nodeIndex == -1)
      {
         continue;
      }

      QGraphicsEllipseItem* pEllipse = iter->second.first;
      if (pEllipse != NULL)
      {
         double xOffset = 0.5;   // Rounding value
         double yOffset = sNodeMargin + (nodeIndex * (sNodeDiameter + sNodeMargin)) + 0.5;

         if (sRectWidth < nameWidth)
         {
            xOffset += (nameWidth / 2.0) - (sRectWidth / 2.0);
         }

         if (mpItem->isInputNode(pNode) == true)
         {
            xOffset += sNodeMargin;
         }
         else
         {
            xOffset += sRectWidth - 1 - sNodeMargin - sNodeDiameter;
         }

         pEllipse->setRect(static_cast<int>(xOffset), static_cast<int>(yOffset), sNodeDiameter, sNodeDiameter);
      }

      // Down button item
      QGraphicsPixmapItem* pPixmap = iter->second.second;
      if (pPixmap != NULL)
      {
         double xOffset = 0.5;   // Rounding value
         double yOffset = sNodeMargin + (nodeIndex * (sNodeDiameter + sNodeMargin)) + 0.5;

         if (sRectWidth < nameWidth)
         {
            xOffset += (nameWidth / 2.0) - (sRectWidth / 2.0);
         }

         if (mpItem->isInputNode(pNode) == true)
         {
            xOffset += sNodeMargin + sNodeDiameter + sNodeMargin;
         }
         else
         {
            xOffset += sRectWidth - 1.0 - (sNodeMargin * 2.0) - (sNodeDiameter * 2.0);
         }

         pPixmap->setPos(static_cast<int>(xOffset), static_cast<int>(yOffset));
      }
   }
}

void WizardGraphicsItem::updateBorder()
{
   if (mpItem.get() == NULL)
   {
      return;
   }

   QColor borderColor = Qt::darkGray;
   if (mpItem->isCurrentModeSupported() == true)
   {
      if (mpItem->getBatchMode() == true)
      {
         borderColor = Qt::red;
      }
      else
      {
         borderColor = Qt::black;
      }
   }
   else if (mpItem->getBatchMode() == true)
   {
      borderColor = Qt::darkRed;
   }

   mpRect->setPen(QPen(QBrush(borderColor), sBorderWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
}

void WizardGraphicsItem::updateNodes()
{
   if ((mpItem.get() == NULL) || (mNodes.empty() == false))
   {
      return;
   }

   // Add the nodes
   const vector<WizardNode*>& inputNodes = mpItem->getInputNodes();
   const vector<WizardNode*>& outputNodes = mpItem->getOutputNodes();
   unsigned int numInputNodes = inputNodes.size();
   unsigned int numOutputNodes = outputNodes.size();

   for (unsigned int i = 0; i < (numInputNodes + numOutputNodes); ++i)
   {
      // Get the wizard node
      int nodeIndex = -1;
      WizardNode* pNode = NULL;

      if (i < numInputNodes)
      {
         nodeIndex = i;
         pNode = inputNodes.at(nodeIndex);
      }
      else
      {
         nodeIndex = i - numInputNodes;
         pNode = outputNodes.at(nodeIndex);
      }

      if (pNode == NULL)
      {
         continue;
      }

      WizardNodeImp* pNodeImp = static_cast<WizardNodeImp*>(pNode);
      VERIFYNRV(pNodeImp != NULL);

      // Node item
      QGraphicsEllipseItem* pEllipse = new QGraphicsEllipseItem(mpRect);
      if (pEllipse != NULL)
      {
         // Pen
         QColor nodeColor = getNodeColor(pNode);
         QColor itemColor = mpRect->brush().color();

         if ((nodeColor == itemColor) || (nodeColor == Qt::white))
         {
            pEllipse->setPen(QPen(Qt::black));
         }
         else
         {
            pEllipse->setPen(QPen(Qt::NoPen));
         }

         // Brush
         pEllipse->setBrush(QBrush(nodeColor));

         // Tool tip
         QString tipText = getNodeToolTip(pNode);
         pEllipse->setToolTip(tipText);
      }

      // Down button item
      QGraphicsPixmapItem* pPixmap = NULL;

      const vector<string>& validTypes = pNodeImp->getValidTypes();
      if (validTypes.size() > 1)
      {
         pPixmap = new QGraphicsPixmapItem(mpRect);

         // Pixmap
         QPixmap pixButton = QPixmap::grabWidget(mpDownButton);
         pPixmap->setPixmap(pixButton);
      }

      // Connections
      VERIFYNR(pNodeImp->attach(SIGNAL_NAME(WizardNodeImp, Renamed), Slot(this, &WizardGraphicsItem::nodeRenamed)));
      VERIFYNR(pNodeImp->attach(SIGNAL_NAME(WizardNodeImp, TypeChanged),
         Slot(this, &WizardGraphicsItem::nodeTypeChanged)));
      VERIFYNR(pNodeImp->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &WizardGraphicsItem::nodeDeleted)));

      mNodes[pNode] = pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*>(pEllipse, pPixmap);
   }

   // Update the item rectangle and text positions
   updateGeometry();
}
