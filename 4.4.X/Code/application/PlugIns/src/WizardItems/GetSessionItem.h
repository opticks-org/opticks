/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GETSESSIONITEM_H
#define GETSESSIONITEM_H

#include "DesktopItems.h"

#include <string>
#include <QtCore/QObject>

class PlotWidget;
class QTreeWidgetItem;

template<class T>
class GetSessionItemBase : public DesktopItems
{
public:
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual bool populateOutputArgs(PlugInArgList* pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   GetSessionItemBase(const std::string& descriptorId);
   virtual ~GetSessionItemBase();
   virtual void populateTreeWidgetItem(QTreeWidgetItem* pRoot) = 0;

   enum ItemDataRole
   {
      SessionItemRole = Qt::UserRole
   };

   enum ItemColumn
   {
      NameColumn = 0,
      TypeColumn = 1
   };

private:
   std::string mDialogCaption;
   T* mpSessionItem;
};

template<class T>
class GetDataElement : public GetSessionItemBase<T>
{
public:
   GetDataElement(const std::string& descriptorId);
   virtual ~GetDataElement();

protected:
   void populateTreeWidgetItem(QTreeWidgetItem* pRoot);
};

template<class T>
class GetLayer : public GetSessionItemBase<T>
{
public:
   GetLayer(const std::string& descriptorId);
   virtual ~GetLayer();

protected:
   void populateTreeWidgetItem(QTreeWidgetItem* pRoot);

private:
   void populateTreeWidgetItemWithLayers(QTreeWidgetItem* pRoot);
};

template<class T>
class GetView : public GetSessionItemBase<T>
{
public:
   GetView(const std::string& descriptorId);
   virtual ~GetView();

protected:
   void populateTreeWidgetItem(QTreeWidgetItem* pRoot);
};

class GetPlotWidget : public GetSessionItemBase<PlotWidget>
{
public:
   GetPlotWidget(const std::string& descriptorId);
   virtual ~GetPlotWidget();

protected:
   void populateTreeWidgetItem(QTreeWidgetItem* pRoot);
};

#endif
