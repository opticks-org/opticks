/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERUNDO_H
#define LAYERUNDO_H

#include "TypesFile.h"
#include "UndoAction.h"

#include <string>

class DataDescriptor;
class Layer;
class SpatialDataView;

class ShowLayer : public UndoAction
{
public:
   ShowLayer(Layer* pLayer);

   void executeUndo();
   void executeRedo();
};


class HideLayer : public UndoAction
{
public:
   HideLayer(Layer* pLayer);

   void executeUndo();
   void executeRedo();
};


class SetLayerName : public UndoAction
{
public:
   SetLayerName(Layer* pLayer, const std::string& oldName, const std::string& newName);

   void executeUndo();
   void executeRedo();

protected:
   void renameLayer(const std::string& name);

private:
   std::string mOldName;
   std::string mNewName;
};


class SetLayerDisplayIndex : public UndoAction
{
public:
   SetLayerDisplayIndex(Layer* pLayer, int oldIndex, int newIndex);

   void executeUndo();
   void executeRedo();

protected:
   void setLayerDisplayIndex(int index);

private:
   int mOldIndex;
   int mNewIndex;
};


class LayerMemento
{
public:
   LayerMemento() {};
   virtual ~LayerMemento() {};

public:
   virtual void toLayer(Layer* pLayer) const = 0;
};


class CreateDestroyLayer : public UndoAction
{
public:
   CreateDestroyLayer(SpatialDataView* pView, Layer* pLayer);
   ~CreateDestroyLayer();

   void updateSessionItem(const std::string& oldId, const std::string& newId);

protected:
   void createLayer();
   void destroyLayer();

private:
   std::string mLayerId;
   std::string mLayerName;
   LayerType mLayerType;
   LayerMemento* mpLayerMemento;

   std::string mElementId;
   std::string mParentId;
   DataDescriptor* mpElementDescriptor;
};


class AddLayer : public CreateDestroyLayer
{
public:
   AddLayer(SpatialDataView* pView, Layer* pLayer);

   void executeUndo();
   void executeRedo();
};


class DeleteLayer : public CreateDestroyLayer
{
public:
   DeleteLayer(SpatialDataView* pView, Layer* pLayer);

   void executeUndo();
   void executeRedo();
};

#endif
