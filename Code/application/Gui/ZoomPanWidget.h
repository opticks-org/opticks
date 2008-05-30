/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ZOOM_PAN_WIDGET_H
#define ZOOM_PAN_WIDGET_H

#include <QtCore/QPoint>
#include <QtGui/QWidget>

#include "LocationType.h"

#include <vector>

class SpatialDataViewImp;
class Layer;

/**
 *  Provides area selection and panning for thumbnail views.
 *
 *  The ZoomPanWidget is a widget that displays a small thumbnail of a view.  A single
 *  layer is displayed, zoomed to the fullest spatial extents.  A selection box appears
 *  in the widget indicating the currently selected area of the image in the corresponding view.
 *
 *  Zooming and panning signals are sent to slots in the parent widget which updates the 
 *  corresponding view.
 *
 *  @see    PreviewWidget, OverviewWindow, ChippingWindow
 */
class ZoomPanWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Constructs the widget.
    *
    *  @param   pView
    *           The layer in the view
    *  @param   parent
    *           The parent widget.  This value may be NULL.
    */
   ZoomPanWidget(SpatialDataViewImp* pView, QWidget* parent = 0);

   /**
    *  Destroys the widget.
    */
   ~ZoomPanWidget();

   /**
    * The current selection box.
    *
    * @return Data coordinates of the current selection.
    */
   std::vector<LocationType> getSelection() const;

   void zoomExtents();

   QSize sizeHint() const;

public slots:
   /**
    * Set the selection.
    *
    * @param selection
    *        The data coordinates of selection.
    */
   void setSelection(const std::vector<LocationType>& selectionData);

signals:
   /**
    * Selection has changed.
    *
    * @param selection
    *        The data coordinates of selection.
    */
   void selectionChanged(const std::vector<LocationType>& selection);

   /**
    * The location of the mouse changed.
    *
    * @param location
    *        The data coordinates of the mouse cursor.
    */
   void locationChanged(const LocationType& location);

protected:
   /**
    *  Intercepts event notification for registered objects.
    *
    *  The implementation of this override calls the local mouse event handlers
    *  before sending the event on to the registered object.
    *
    *  @param   QObject* o
    *           The object sending the event message.
    *  @param   QEvent* e
    *           The event message.
    *
    *  @return  TRUE if the event should not be passed to the respective object.  
    *           FALSE if the event should be processed normally.
    */
   bool eventFilter(QObject* o, QEvent* e);

   void enterEvent(QEvent* e);

   /**
    *  Performs special actions when the user presses a mouse button.
    *
    *  The implementation of this method updates the selection box for zooming if the
    *  left mouse button is pressed.  If the right button is pressed and the mouse is
    *  outside of the selection box and the pan mode is set to PAN_INSTANT, the view is
    *  panned to the such that the pressed location becomes the center of the display.
    *
    *  @param   e
    *           The mouse message containing information about the event.
    */
   void mousePressEvent(QMouseEvent* e);

   /**
    *  Performs special actions when the user moves the mouse over the widget.
    *
    *  The implementation of this method updates the selection box for zooming if the
    *  left mouse button is pressed.  If the right button is pressed and the pan mode
    *  is set to PAN_INSTANT, the view is panned to new area outlined by the selection
    *  box.
    *
    *  @param   e
    *           The mouse message containing information about the event.
    */
   void mouseMoveEvent(QMouseEvent* e);

   /**
    *  Performs special actions when the user releases a mouse button.
    *
    *  The implementation of this method updates the displayed area of the corresponding
    *  view to the area outlined by the selection box if the left mouse button is pressed.
    *  If the right button is pressed and the pan mode is set to PAN_DELAY, the view is
    *  panned to area outlined by the selection box.
    *
    *  @param   e
    *           The mouse message containing information about the event.
    */
   void mouseReleaseEvent(QMouseEvent* e);

   /**
    *  Performs special actions when the user double clicks a mouse button.
    *
    *  If the left mouse button is double clicked, the selectionChanged() signal with full
    *  image extents is sent to the parent widget followed by the endZoom signal.  If the
    *  right button is pressed, the resetOrientation() signal is sent.
    *
    *  @param   e
    *           The mouse message containing information about the event.
    */
   void mouseDoubleClickEvent(QMouseEvent* e);

   void leaveEvent(QEvent* e);

   void keyPressEvent(QKeyEvent* e);
   void keyReleaseEvent(QKeyEvent* pEvent);

   bool selectionHit(const QPoint& screenCoord) const;

protected slots:
   void updateMouseCursor();

private:
   void translateDataToWorld(const std::vector<LocationType> &data, 
      std::vector<LocationType> &world) const;
   void translateWorldToData(const std::vector<LocationType> &world,
      std::vector<LocationType> &data) const;


   SpatialDataViewImp* mpView;
   Layer *mpLayer;

   QPoint mMouseStart;
   QPoint mMouseCurrent;
};

#endif
