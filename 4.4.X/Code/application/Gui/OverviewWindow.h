/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OVERVIEWWINDOW_H
#define OVERVIEWWINDOW_H

#include <QtGui/QDialog>

#include "ColorType.h"
#include "LocationType.h"

#include <vector>

class AnnotationLayerAdapter;
class SpatialDataViewImp;
class TrailObjectImp;
class ZoomPanWidget;

/**
 *  Display a small thumbnail of a view.
 *
 *  The OverviewWindow is a widget that displays a small thumbnail of a view.  A single
 *  layer is displayed, zoomed to the fullest spatial extents.  A selection box appears
 *  in the widget indicating the currently displayed area of the corresponding view.
 *
 *  Zooming and panning functions are provided by the member instance of a 
 *  ZoomPanWidget. The pan mode for the window can be set with the setPanMode() method.  The
 *  updateSelectionBox() method can be used to force an update of the selection box location
 *  in the corresponding view.
 *
 *  @see   ZoomPanWidget
 */
class OverviewWindow : public QDialog
{
   Q_OBJECT

public:
   OverviewWindow(SpatialDataViewImp* pView, QWidget* parent = 0);
   ~OverviewWindow();

signals:
   void visibilityChanged(bool bVisible);

protected:
   bool eventFilter(QObject* o, QEvent* e);
   void showEvent(QShowEvent* e);
   void resizeEvent(QResizeEvent* e);
   void closeEvent(QCloseEvent* e);
   void contextMenuEvent(QContextMenuEvent* pEvent);

   SpatialDataViewImp* createOverview();
   TrailObjectImp* createSnailTrail(SpatialDataViewImp* pOverview);

protected slots:
   void updateView(const std::vector<LocationType>& selectionArea);
   void updateSelectionBox();
   void changeTrailColor();
   void clearTrail();
   void changeTrailOpacity();
   void changeTrailThreshold();
   void takeSnapshot();

private:
   SpatialDataViewImp* mpView;
   SpatialDataViewImp* mpOverview;
   ZoomPanWidget* mpSelectionWidget;
   AnnotationLayerAdapter* mpTrailLayer;
   TrailObjectImp* mpTrail;
   ColorType mTrailColor;  // also contains alpha value
   int mZoomThreshold;
};

#endif
