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

#include <QtGui/QGraphicsView>
#include <QtGui/QRubberBand>

#include "AttachmentPtr.h"
#include "WizardObjectAdapter.h"

#include <memory>
#include <vector>

class WizardGraphicsItem;
class WizardItem;
class WizardLine;
class WizardNode;

class WizardView : public QGraphicsView
{
   Q_OBJECT

public:
   WizardView(QGraphicsScene* pScene, QWidget* pParent = NULL);
   virtual ~WizardView();

   bool setWizard(const QString& filename);
   void setWizard(WizardObject* pObject);
   WizardObject* getWizard();
   const WizardObject* getWizard() const;

   void selectItem(WizardItem* pItem);
   void deselectItem(WizardItem* pItem);
   std::vector<WizardItem*> getSelectedItems(const std::string& type = std::string()) const;
   bool isItemSelected(WizardItem* pItem) const;

   bool editItem(WizardItem* pItem);

   bool isModified() const;

public slots:
   void selectAllItems();
   void deselectAllItems();
   void removeSelectedItems();
   void zoomIn();
   void zoomOut();
   void zoomToFit();
   void zoomBy(double factor);
   void zoomTo(double factor);
   void zoomTo(const QRectF& sceneRect);
   void resetZoom();
   void print();
   void execute();
   bool save();
   bool saveAs();
   bool editItems();

signals:
   void filenameChanged(const QString& filename);
   void selectionChanged();

protected:
   virtual void keyPressEvent(QKeyEvent* pEvent);
   virtual void keyReleaseEvent(QKeyEvent* pEvent);
   virtual void dragEnterEvent(QDragEnterEvent* pEvent);
   virtual void dragMoveEvent(QDragMoveEvent* pEvent);
   virtual void dropEvent(QDropEvent* pEvent);
   virtual void enterEvent(QEvent* pEvent);
   virtual void mousePressEvent(QMouseEvent* pEvent);
   virtual void mouseMoveEvent(QMouseEvent* pEvent);
   virtual void mouseReleaseEvent(QMouseEvent* pEvent);
   virtual void mouseDoubleClickEvent(QMouseEvent* pEvent);
   virtual void contextMenuEvent(QContextMenuEvent* pEvent);
   virtual void wheelEvent(QWheelEvent* pEvent);

   void itemAdded(Subject& subject, const std::string& signal, const boost::any& data);
   void itemRemoved(Subject& subject, const std::string& signal, const boost::any& data);
   void executionOrderChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void itemPositionChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeAdded(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeRemoved(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeConnected(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeDisconnected(Subject& subject, const std::string& signal, const boost::any& data);
   void setModified(Subject& subject, const std::string& signal, const boost::any& data);

   void addItem(WizardItem* pItem);
   void addItems(const std::vector<WizardItem*>& wizardItems);
   void removeItem(WizardItem* pItem);
   unsigned int getNumItems() const;
   void clear();

   void addConnection(WizardNode* pOutputNode, WizardNode* pInputNode);
   void removeConnection(WizardNode* pOutputNode, WizardNode* pInputNode);
   void updateConnectionPosition(WizardItem* pItem);
   void updateConnectionPosition(WizardNode* pOutputNode, WizardNode* pInputNode);
   void updateExecutionOrder();

   WizardGraphicsItem* getGraphicsItem(const QPoint& viewPos) const;
   WizardGraphicsItem* getGraphicsItem(WizardItem* pItem) const;
   WizardGraphicsItem* getGraphicsItem(WizardNode* pNode) const;
   std::vector<WizardGraphicsItem*> getGraphicsItems() const;

   bool save(const QString& filename);

private:
   static int sItemZ;
   static int sLineZ;
   static const double sZoomIn;
   static const double sZoomOut;
   static const double sZoomMin;
   static const double sZoomMax;

   AttachmentPtr<WizardObjectAdapter> mpWizard;
   QString mFilename;
   QPoint mMousePressPoint;
   std::auto_ptr<QRubberBand> mpZoomRect;
   std::auto_ptr<WizardLine> mpConnectionLine;
   bool mModified;
};

#endif
