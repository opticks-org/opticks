/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WINDOWMODEL_H
#define WINDOWMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include "SessionItemModel.h"

class GraphicObject;
class Layer;
class PlotSet;
class View;
class Window;

class WindowModel : public QSortFilterProxyModel
{
public:
   WindowModel(QObject* pParent = 0);
   virtual ~WindowModel();

   virtual Qt::DropActions supportedDropActions() const;
   virtual bool dropMimeData(const QMimeData* pData, Qt::DropAction action, int row, int column,\
      const QModelIndex& dropIndex);

protected:
   virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
   class WindowSourceModel : public SessionItemModel
   {
   public:
      WindowSourceModel(QObject* pParent = 0);
      virtual ~WindowSourceModel();

      virtual Qt::ItemFlags flags(const QModelIndex& index) const;
      virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

      void refreshModel(Subject &subject, const std::string &signal, const boost::any &v);
      void detachModel(Subject &subject, const std::string &signal, const boost::any &v);
      void addWindow(Subject& subject, const std::string& signal, const boost::any& value);
      void removeWindow(Subject& subject, const std::string& signal, const boost::any& value);
      void setCurrentWindow(Subject& subject, const std::string& signal, const boost::any& value);
      void addView(Subject& subject, const std::string& signal, const boost::any& value);
      void removeView(Subject& subject, const std::string& signal, const boost::any& value);
      void addLayer(Subject& subject, const std::string& signal, const boost::any& value);
      void removeLayer(Subject& subject, const std::string& signal, const boost::any& value);
      void updateLayerOrder(Subject& subject, const std::string& signal, const boost::any& value);
      void updateLayerDisplay(Subject& subject, const std::string& signal, const boost::any& value);
      void updateToolbarDisplay(Subject& subject, const std::string& signal, const boost::any& value);
      void updateDockDisplay(Subject& subject, const std::string& signal, const boost::any& value);
      void activateLayer(Subject& subject, const std::string& signal, const boost::any& value);
      void addGraphicObject(Subject& subject, const std::string& signal, const boost::any& value);
      void removeGraphicObject(Subject& subject, const std::string& signal, const boost::any& value);
      void addPlotSet(Subject& subject, const std::string& signal, const boost::any& value);
      void removePlotSet(Subject& subject, const std::string& signal, const boost::any& value);
      void setCurrentPlotSet(Subject& subject, const std::string& signal, const boost::any& value);
      void addPlotWidget(Subject& subject, const std::string& signal, const boost::any& value);
      void removePlotWidget(Subject& subject, const std::string& signal, const boost::any& value);
      void setCurrentPlotWidget(Subject& subject, const std::string& signal, const boost::any& value);

   protected:
      SessionItemWrapper* addWindowItem(Window* pWindow);
      void removeWindowItem(Window* pWindow);
      SessionItemWrapper* addViewItem(SessionItemWrapper* pWindowWrapper, View* pView, bool bAddViewItem = true);
      void removeViewItem(SessionItemWrapper* pWindowWrapper, View* pView);
      SessionItemWrapper* addLayerItem(SessionItemWrapper* pViewWrapper, Layer* pLayer);
      void removeLayerItem(SessionItemWrapper* pViewWrapper, Layer* pLayer);
      SessionItemWrapper* addGraphicObjectItem(SessionItemWrapper* pLayerWrapper, GraphicObject* pObject);
      void removeGraphicObjectItem(SessionItemWrapper* pLayerWrapper, GraphicObject* pObject);
      SessionItemWrapper* addPlotSetItem(SessionItemWrapper* pPlotWindowWrapper, PlotSet* pPlotSet);
      void removePlotSetItem(SessionItemWrapper* pPlotWindowWrapper, PlotSet* pPlotSet);

   private:
      SessionItemWrapper* mpWorkspaceWindowWrapper;
      SessionItemWrapper* mpDockWindowWrapper;
      SessionItemWrapper* mpToolBarWrapper;
   };
};

#endif
