/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOITOOLBAR_H
#define AOITOOLBAR_H

#include "ConfigurationSettings.h"
#include "ToolBar.h"
#include "TypesFile.h"

/**
 *  Provides a user interface to edit the selected pixels in an AOI layer.
 *
 *  The AOI toolbar is a toolbar that allows users to edit the active AOI layer
 *  in the active view.  This class provides the capability to set the pixel
 *  selection tools and to modify the way in which new pixels are added to the
 *  AOI layer.
 *
 *  A pointer to the AOI toolbar can be obtained by calling
 *  DesktopServices::getWindow() using "AOI" as the window name and ::TOOLBAR as
 *  the window type.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *    - The toolbar is deleted, which will only occur on application shutdown.
 *    - Other notifications documented in the ToolBar class.
 *
 *  @see     ToolBar, AoiLayer
 */
class AoiToolBar : public ToolBar
{
public:
   SETTING(SelectionTool, AoiToolBar, GraphicObjectType, MULTIPOINT_OBJECT);

   /**
    *  Sets the current AOI pixel selection tool on the toolbar.
    *
    *  This method sets the current object that is created when the user adds a
    *  new object to an AOI layer and the drawing mode that is used when the
    *  user edits the layer with the mouse.  The given object type and drawing
    *  mode are set into the current AOI layer of the current view and into
    *  each AOI layer that is activated in any view.  To set the graphic object
    *  type and drawing mode in the layer separately from the AOI toolbar, call
    *  GraphicLayer::setCurrentGraphicObjectType() or AoiLayer::setMode()
    *  instead.
    *
    *  @param   toolType
    *           The new pixel selection tool.
    *  @param   modeType
    *           The new pixel selection mode.
    */
   virtual void setSelectionTool(GraphicObjectType toolType, ModeType modeType) = 0;

   /**
    *  Returns the current AOI pixel selection tool on the toolbar.
    *
    *  @return  The current pixel selection tool.
    */
   virtual GraphicObjectType getSelectionTool() const = 0;

   /**
    *  Returns the current AOI pixel selection mode on the toolbar.
    *
    *  @return  The current pixel selection mode.
    */
   virtual ModeType getSelectionMode() const = 0;

   /**
    *  Sets the current AOI add mode on the toolbar.
    *
    *  @param   mode
    *           The new add mode.
    */
   virtual void setAddMode(AoiAddMode mode) = 0;

   /**
    *  Returns the current AOI add mode on the toolbar.
    *
    *  @return  The current add mode.
    */
   virtual AoiAddMode getAddMode() const = 0;

protected:
   /**
    *  This toolbar will be destroyed automatically when the application closes.
    *  Plug-ins do not need to destroy it.
    */
   virtual ~AoiToolBar()
   {}
};

#endif
