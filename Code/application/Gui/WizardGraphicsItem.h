/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDGRAPHICSITEM_H
#define WIZARDGRAPHICSITEM_H

#include <QtCore/QMap>
#include <QtGui/QGraphicsItemGroup>
#include <QtGui/QToolButton>

#include "AttachmentPtr.h"
#include "WizardItemImp.h"

#include <boost/any.hpp>
#include <map>
#include <string>

class WizardItem;
class WizardNode;

class WizardGraphicsItem : public QObject, public QGraphicsItemGroup
{
   Q_OBJECT

public:
   WizardGraphicsItem(WizardItem* pItem, QGraphicsItem* pParent = NULL);
   virtual ~WizardGraphicsItem();

   WizardItem* getWizardItem();
   const WizardItem* getWizardItem() const;

   void setExecutionOrder(int iOrder);
   int getExecutionOrder() const;
   WizardNode* getNode(const QPointF& scenePos) const;
   QPointF getNodeConnectionPoint(WizardNode* pNode) const;

   virtual QRectF boundingRect() const;
   virtual QPainterPath shape() const;
   virtual void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget = NULL);

   static QColor getNodeColor(WizardNode* pNode);
   static QColor getNodeColor(const QString& nodeType);
   static QPixmap getNodePixmap(WizardNode* pNode);
   static QPixmap getNodePixmap(const QString& nodeType);
   static QColor getItemBackgroundColor(const QString& itemType);

signals:
   void selectionChanged();

protected:
   virtual void mousePressEvent(QGraphicsSceneMouseEvent* pEvent);
   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent);
   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent);
   virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

   void itemRenamed(Subject& subject, const std::string& signal, const boost::any& data);
   void itemPositionChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void itemBatchModeChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeRenamed(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeTypeChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeDeleted(Subject& subject, const std::string& signal, const boost::any& data);

   QRectF getNodeRect(WizardNode* pNode) const;
   QString getNodeToolTip(WizardNode* pNode) const;
   void updateGeometry();
   void updateBorder();
   void updateNodes();

private:
   AttachmentPtr<WizardItemImp> mpItem;

   // Rectangle
   QGraphicsRectItem* mpRect;
   static const double sRectWidth;
   static const double sBorderWidth;

   // Execution order
   QGraphicsSimpleTextItem* mpOrder;

   // Nodes
   std::map<WizardNode*, std::pair<QGraphicsEllipseItem*, QGraphicsPixmapItem*> > mNodes;
   static const double sNodeDiameter;
   static const double sNodeMargin;
   QToolButton* mpDownButton;
   WizardNode* mpActiveNode;
   static QMap<QString, QColor> sNodeColors;

   // Text
   QGraphicsSimpleTextItem* mpText;
   static const double sTextMargin;

   // Selection nodes
   static const double sSelectionNodeWidth;
};

#endif
