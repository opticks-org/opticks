/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRODUCTVIEW_H
#define PRODUCTVIEW_H

#include "ConfigurationSettings.h"
#include "PerspectiveView.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class AnnotationLayer;
class ClassificationLayer;
class GraphicLayer;
class GraphicObject;

/**
 *  A view for custom product generation.
 *
 *  The product view provides a means by which custom output products can be created.
 *  The view contains an annotation layer called the layout layer that is displayed on
 *  a sheet of paper.  Annotation objects can be added to the layout layer to create a
 *  finished product.  View objects are often used in the layout layer to display
 *  data or results.  Because views can be modified separately, the product view contains
 *  an active edit view.  The active edit view object is identified in the layout layer
 *  with selection handles.  The view also contains a classification layer to specify
 *  classification markings for the product.
 *
 *  The product view defines the following mouse modes, where the name given is the name
 *  populated by MouseMode::getName():
 *  - LayerMode
 *  - PanMode
 *  - ZoomInMode
 *  - ZoomOutMode
 *  - ZoomBoxMode
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setPaperSize(), setPaperColor(),
 *    setDpi(), setActiveLayer().
 *  - Everything else documented in PerspectiveView.
 *
 *  @see     AnnotationLayer, ClassificationLayer
 */
class ProductView : public PerspectiveView
{
public:
   SETTING_PTR(TemplateFile, ProductView, Filename)
   SETTING_PTR(TemplatePath, ProductView, Filename)
   SETTING(ClassificationMarkingPositions, ProductView, PositionType, CENTER)

   /**
    *  Emitted with boost::any<std::pair<double,double> > when the paper size
    *  is changed.
    */
   SIGNAL_METHOD(ProductView, PaperSizeChanged)

   /**
    *  Emitted with boost::any<ColorType> when the paper color is changed.
    */
   SIGNAL_METHOD(ProductView, PaperColorChanged)

   /**
    *  Emitted with boost::any<unsigned int> when the paper dpi changes.
    */
   SIGNAL_METHOD(ProductView, DpiChanged)

   /**
    *  Emitted with boost::any<Layer*> when a layer is activated.
    */
   SIGNAL_METHOD(ProductView, LayerActivated)

   /**
    *  Sets the paper size.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dWidth
    *           The new width of the displayed paper in inches.
    *  @param   dHeight
    *           The new height of the displayed paper in inches.
    *
    *  @notify  This method will notify signalPaperSizeChanged with any<std::pair<double,double> >.
    */
   virtual void setPaperSize(double dWidth, double dHeight) = 0;

   /**
    *  Retrieves the paper size.
    *
    *  @param   dWidth
    *           Populated with the width of the displayed paper in inches.
    *  @param   dHeight
    *           Populated with the height of the displayed paper in inches.
    */
   virtual void getPaperSize(double& dWidth, double& dHeight) const = 0;

   /**
    *  Sets the paper color.
    *
    *  The paper color extends to the edge of the paper area.  Depending on the view
    *  zoom level, this may or may not extend to the edge of the view area.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   paperColor
    *           The new paper color.
    *
    *  @notify  This method will notify signalPaperColorChanged with any<ColorType>.
    *
    *  @see     View::setBackgroundColor()
    */
   virtual void setPaperColor(const ColorType& paperColor) = 0;

   /**
    *  Returns the paper color.
    *
    *  @return  The current paper color.
    *
    *  @see     View::getBackgroundColor()
    */
   virtual ColorType getPaperColor() const = 0;

   /**
    *  Sets the dots per inch (DPI) of the paper.
    *
    *  The paper DPI is used in conjunction with the paper size to establish
    *  the coordinate extents of the paper used by the Layout annotation layer.
    *  The default DPI is 100, so a paper size of 8.5 by 11 has paper coordinate
    *  extents of 850 and 1100.
    *
    *  This method does not call View::refresh() so that multiple calls to modify
    *  view settings can be made without refreshing the view after each modification.
    *
    *  @param   dpi
    *           The new dots per inch value of the paper.
    *
    *  @notify  This method will notify signalDpiChanged with any<unsigned int>.
    */
   virtual void setDpi(unsigned int dpi) = 0;

   /**
    *  Returns the dots per inch (DPI) of the displayed paper.
    *
    *  The paper DPI is used in conjunction with the paper size to establish
    *  the coordinate extents of the paper used by the Layout annotation layer.
    *  The default DPI is 100, so a paper size of 8.5 by 11 has paper coordinate
    *  extents of 850 and 1100.
    *
    *  @return  The dots per inch value of the paper.
    */
   virtual unsigned int getDpi() const = 0;

   /**
    *  Loads a product template from a file.
    *
    *  A product template is a set of annotation objects displayed in the layout
    *  layer. Loading a template from a file removes all objects currently displayed
    *  in the layout layer.
    *
    *  @param   filename
    *           The filename of the template to load.  An empty string will cause a
    *           file selection dialog box to be displayed for the user to select
    *           a file.
    *
    *  @return  TRUE if the template was loaded successfully, otherwise FALSE.
    *
    *  @see     getLayoutLayer()
    */
   virtual bool loadTemplate(const std::string& filename) = 0;

   /**
    *  Saves the annotation objects in the layout layer as a template.
    *
    *  @param   filename
    *           The destination filename of the template to save.  An empty string
    *           will cause a file selection dialog box to be displayed for the user
    *           to select a file.
    *
    *  @return  TRUE if the template was saved successfully, otherwise FALSE.
    */
   virtual bool saveTemplate(const std::string& filename) const = 0;

   /**
    *  Returns the annotation layer for the product's annotation objects.
    *
    *  @return  A pointer to the annotation layer containing the annotation objects.
    */
   virtual AnnotationLayer* getLayoutLayer() const = 0;

   /**
    *  Returns the classification layer used to display the product classification
    *  markings.
    *
    *  @return  A pointer to the classification layer.
    */
   virtual ClassificationLayer* getClassificationLayer() const = 0;

   /**
    *  Sets the active layer in the view.
    *
    *  The active layer will appear on the toolbar if there is no active edit view.
    *
    *  @param   pLayer
    *           The layer to activate.  This must be either the layout layer or the
    *           classification layer.
    *
    *  @see     getLayoutLayer(), getClassificationLayer()
    *
    *  @notify  This method will notify signalLayerActivated with any<GraphicLayer*>.
    */
   virtual void setActiveLayer(GraphicLayer* pLayer) = 0;

   /**
    *  Returns the active product view layer.
    *
    *  @return  A pointer to the active layer, either the layout layer or the
    *           classification layer.  NULL is returned if there is an active
    *           edit view object.
    *
    *  @see     getLayoutLayer(), getClassificationLayer(), getActiveEditObject(),
    *           getActiveEditView()
    */
   virtual GraphicLayer* getActiveLayer() const = 0;

   /**
    *  Sets the active view object that processes mouse and other events.
    *
    *  This method enables and disables editing of a view inside of its annotation
    *  view object.  The active view object appears with selection handles and all
    *  mouse events are processed by the view instead of the layout layer.
    *
    *  The active edit view object can also be set according to its view by calling
    *  setActiveEditView().
    *
    *  @param   pObject
    *           The annotation view object to set as the active edit object.  If
    *           this value is non-NULL, the active layer becomes NULL.  If the value
    *           is NULL, editing of the view is disabled and editing of the layout
    *           layer is enabled.
    *
    *  @return  This method returns true if the annotation view object was successfully
    *           activated, thereby enabling editing of its view, otherwise false.
    *
    *  @see     setActiveEditView()
    */
   virtual bool setActiveEditObject(GraphicObject* pObject) = 0;

   /**
    *  Returns the annotation view object that has view editing enabled.
    *
    *  This method returns the current annotation object containing the view that can
    *  be edited.  The view inside the annotation object can be obtained directly by
    *  calling getActiveEditView().
    *
    *  @return  A pointer to the active annotation view object that allows editing of
    *           the view contained in the object.  NULL is returned if view editing is
    *           disabled and the layout layer or classification layer is enabled.
    *
    *  @see     getActiveEditView()
    */
   virtual GraphicObject* getActiveEditObject() const = 0;

   /**
    *  Sets the active view that processes mouse and other events.
    *
    *  This method enables and disables editing of a view inside of its annotation
    *  view object.  The active view object appears with selection handles and all
    *  mouse events are processed by the view instead of the layout layer.
    *
    *  The active edit view object can also be set according to its annotation
    *  object by calling setActiveEditObject().
    *
    *  @param   pView
    *           The view to set as the active edit view.  The view must be contained
    *           in a valid annotation view object in the layout layer.  If the view
    *           object containing this view is found, the active layer becomes NULL.
    *           Set this value to NULL to disable view editing and enable editing of
    *           the layout layer.
    *
    *  @return  This method returns true if the annotation view object containing the
    *           given view was successfully activated, thereby enabling editing of
    *           view.  Otherwise this method returns false.
    *
    *  @see     setActiveEditObject()
    */
   virtual bool setActiveEditView(View* pView) = 0;

   /**
    *  Returns the view that has editing enabled.
    *
    *  This method returns the view inside the annotation view object that is
    *  currently active.  This is identical to getActiveEditObject()->getObjectView().
    *
    *  @return  A pointer to the view inside of an annotation view object that can be
    *           actively edited.  NULL is returned if view editing is disabled and the
    *           layout layer or classification layer is enabled.
    *
    *  @see     getActiveEditObject()
    */
   virtual View* getActiveEditView() const = 0;

protected:
   /**
    * This should be destroyed by calling DesktopServices::deleteView.
    */
   virtual ~ProductView() {}
};

#endif
