/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CHIPPING_WINDOW_H
#define CHIPPING_WINDOW_H

#include <QtGui/QDialog>
#include <QtGui/QRadioButton>

#include "TypesFile.h"

class ChippingWidget;
class RasterElement;
class SpatialDataView;

/**
 *  Displays a small thumbnail of a view for selecting image chipping area and creating the chip.
 *
 *  The ChippingWindow is a QDialog that displays a small thumbnail of a view.  A single
 *  layer is displayed, zoomed to the fullest spatial extents.  A selection box appears
 *  in the widget indicating the currently selected area of the image in the corresponding parent view.
 */
class ChippingWindow : public QDialog
{
   Q_OBJECT

public:
   ChippingWindow(SpatialDataView* pView, QWidget* parent = 0);
   ~ChippingWindow();

protected:
   RasterElement* getRasterElement() const;
   SpatialDataView* createChipView() const;
   void createView();
   void createFile();
   bool eventFilter(QObject* pObject, QEvent* pEvent);

protected slots:
   void accept();

private:
   SpatialDataView* mpView;

   ChippingWidget* mpChippingWidget;
   QRadioButton* mpWindowRadio;
   QRadioButton* mpFileRadio;
};

#endif
