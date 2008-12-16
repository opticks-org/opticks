/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOUSEMODETESTGUI_H
#define MOUSEMODETESTGUI_H

#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QRadioButton>

#include "AttachmentPtr.h"
#include "SpatialDataView.h"

#include <boost/any.hpp>
#include <string>

class MouseMode;
class QRadioButton;

class MouseModeTestGui : public QDialog
{
   Q_OBJECT

public:
   MouseModeTestGui(SpatialDataView* pView, QWidget* pParent = NULL);
   ~MouseModeTestGui();

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void updateMouseMode(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setMouseMode(QAbstractButton* pButton);
   void setMouseMode(const MouseMode* pMouseMode);
   void enableCustomMouseMode(bool bEnable);

private:
   QRadioButton* mpNoModeRadio;
   QRadioButton* mpLayerRadio;
   QRadioButton* mpMeasurementRadio;
   QRadioButton* mpPanRadio;
   QRadioButton* mpRotateRadio;
   QRadioButton* mpZoomInRadio;
   QRadioButton* mpZoomOutRadio;
   QRadioButton* mpZoomBoxRadio;
   QRadioButton* mpCustomRadio;

   AttachmentPtr<SpatialDataView> mpView;
   MouseMode* mpCustomMouseMode;
   QAction* mpCustomAction;
};

#endif
