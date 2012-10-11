/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMLAYER_H
#define CUSTOMLAYER_H

#include "ConfigurationSettings.h"
#include "Layer.h"

class DrawObject;
class QCursor;

/**
 *  Manages drawing and storage of custom data in a layer.
 *
 *  A custom layer provides the capability for a plug-in to perform custom
 *  drawing in a view.  The drawing and specific information needed by the view
 *  (e.g. layer extents) is implemented in a special DrawObject instance that is
 *  set into the custom layer.  The custom layer also maintains an Any data
 *  element, where custom data drawn by the DrawObject can optionally be stored.
 *  The sections below provide details on working with custom layers.
 *
 *  <b>Creating and managing a custom layer</b><br>
 *  One approach for creating and managing a custom layer is to create a plug-in
 *  that manages a single custom layer instance.  This type of plug-in can be
 *  structured as follows:
 *  <ul>
 *      <li>Since the DrawObject provides the actual drawing implementation for
 *          the layer, the plug-in must be running throughout the lifetime of
 *          the layer.  The plug-in should therefore be an executable plug-in
 *          where Executable::isDestroyedAfterExecute() returns \c false.</li>
 *      <li>The layer should be created and configured in Executable::execute(),
 *          which would also create an instance of a custom DrawObject
 *          and set it into the custom layer. In addition, Executable::execute()
 *          would also create/import any custom data (via an AnyData subclass)
 *          and set it into the layer's Any element.</li>
 *      <li>To destroy the plug-in when the custom layer is destroyed, the
 *          plug-in should inherit ViewerShell.  A slot method should be created
 *          that is attached to the Layer::signalDeleted() signal, where
 *          Executable::abort() is called from within the slot.  This will
 *          automatically register a callback with DesktopServices through
 *          ViewerShell that destroys the plug-in.</li>
 *  </ul>
 *  The CustomLayerPlugIn sample plug-in demonstrates this approach of creating
 *  and managing a custom layer.
 *
 *  <b>Implementing session save/restore</b><br>
 *  To implement session save/restore, the plug-in managing the custom layer
 *  should implement SessionItem::serialize() and SessionItem::deserialize().
 *  The serialize() implementation should save the layer ID and the Any element
 *  ID if custom data is stored in the element.  The method should also serialize
 *  the custom data stored in the AnyData subclass.  The element ID needs to be
 *  stored separately from the layer ID because the layer's element pointer may
 *  not have yet been restored when the plug-in is restored.
 *
 *  The deserialize() method should retrieve the layer from SessionManager based
 *  on its ID, create the custom DrawObject and set the DrawObject into the layer.
 *  The method should also retrieve the Any element from SessionManager, create the
 *  custom AnyData container, deserialize the custom data into the AnyData, and
 *  set the custom data into the Any element.
 *
 *  The CustomLayerPlugIn sample plug-in demonstrates implementing session
 *  save/restore for a custom layer.
 *
 *  <b>Interacting with mouse events</b><br>
 *  The layer also provides the capability through the DrawObject to process
 *  mouse events via the LayerEdit mouse mode, with an option to set a custom
 *  mouse cursor.  Call setAcceptsMouseEvents() to enable/disable the support
 *  for the LayerEdit mouse mode.  The mouse cursor can then be set by calling
 *  setEditMouseCursor().  The layer could also support a custom mouse mode,
 *  however the implementation of this must be entirely contained in the
 *  plug-in.
 *
 *  The CustomLayerPlugIn sample plug-in demonstrates enabling support for the
 *  LayerEdit mouse mode for a custom layer.  See the MouseModePlugIn sample
 *  plug-in for details on creating a custom mouse mode.
 *
 *  <b>Copying a custom layer</b><br>
 *  When a custom layer is copied by the user, the layer will automatically be
 *  created, however the custom DrawObject and any custom data will not be
 *  created.  Therefore, to support copying the layer, a plug-in must exist that
 *  is attached to LayerList::signalLayerAdded().  When the connected slot
 *  method is called, the custom DrawObject and data can be created and set into
 *  the layer if a custom layer was created.
 *
 *  <b>Supporting undo/redo</b><br>
 *  Similar to copying a custom layer, when the user invokes undo/redo actions
 *  that delete and recreate a custom layer, the layer will automatically be
 *  created, however the custom DrawObject and any custom data will not be
 *  created.  Therefore, to support undo/redo a plug-in must exist that
 *  is attached to LayerList::signalLayerAdded().  When the connected slot
 *  method is called, the custom DrawObject and data can be created and set into
 *  the layer if a custom layer was created.
 *
 *  <b>%Subject Notifications</b><br>
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: setDrawObject().
 *  - Everything else documented in Layer.
 *
 *  @see     Layer, DrawObject
 */
class CustomLayer : public Layer
{
public:
   /**
    *  Emitted with boost::any<\link DrawObject\endlink*> when the draw object is changed.
    */
   SIGNAL_METHOD(CustomLayer, DrawObjectChanged)

   /**
    *  Sets the draw object for the custom layer and sets the layer into the draw object
    *  if the draw object is not \c NULL.
    *
    *  @param   pDrawObject
    *           The new draw object.
    *
    *  @notify  This method will notify signalDrawObjectChanged() with boost::any<\link DrawObject\endlink*>.
    *
    *  @see     getDrawObject()
    */
   virtual void setDrawObject(DrawObject* pDrawObject) = 0;

   /**
    *  Returns the draw object of the custom layer.
    *
    *  @return  The draw object.
    *
    *  @see     setDrawObject()
    */
   virtual const DrawObject* getDrawObject() const = 0;

   /**
    *  Returns the draw object of the custom layer.
    *
    *  @return  The draw object.
    *
    *  @see     setDrawObject()
    */
   virtual DrawObject* getDrawObject() = 0;

   /**
    *  This method returns the capability of the custom layer to accept mouse events.
    *
    *  @return  Returns \c true if the layer accepts mouse events, otherwise it returns \c false.
    */
   virtual bool acceptsMouseEvents() const = 0;

   /**
    *  This method returns the custom layer's mouse cursor.
    *
    *  @return  Returns a const reference to the custom layer's mouse cursor. The default mouse cursor
    *           is set as Qt::ArrowCursor.
    *
    *  @note    The edit cursor is enabled when the LayerEdit mouse mode is activated in the view.
    */
   virtual const QCursor& getEditMouseCursor() const = 0;

   /**
    *  This method sets the capability of the custom layer to accept mouse events.
    *
    *  @param   accept
    *           Boolean value for the capability of the custom layer to accept mouse events.
    */
   virtual void setAcceptsMouseEvents(bool accept) = 0;

   /**
    *  This method sets the mouse cursor for the custom layer.
    *
    *  @param    cursor
    *            The cursor to set as the custom layer mouse cursor.
    *
    *  @note    The edit cursor is enabled when the LayerEdit mouse mode is activated in the view.
    */
   virtual void setEditMouseCursor(const QCursor& cursor) = 0;

protected:
   /**
    * This should be destroyed by calling SpatialDataView::deleteLayer.
    */
   virtual ~CustomLayer() {}
};

#endif
