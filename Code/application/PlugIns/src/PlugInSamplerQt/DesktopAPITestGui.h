/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DESKTOPAPITESTGUI_H
#define DESKTOPAPITESTGUI_H

#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>

#include "AttachmentPtr.h"
#include "PlotWidget.h"
#include "Window.h"

#include <boost/any.hpp>
#include <string>

class PlotWindow;

class DesktopAPITestGui : public QDialog, public Window::SessionItemDropFilter
{
   Q_OBJECT

public:
   DesktopAPITestGui(QWidget* pParent = NULL);
   ~DesktopAPITestGui();

   bool accept(SessionItem* pItem) const;

protected:
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void updatePropertiesDialog(Subject& subject, const std::string& signal, const boost::any& value);
   void dropSessionItem(Subject& subject, const std::string& signal, const boost::any& value);
   void docked(Subject& subject, const std::string& signal, const boost::any& value);
   void undocked(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void addBrowseButton(bool bAdd);
   void addPrintButton(bool bAdd);
   void enableMouseMode();
   void setClassificationText();
   void setPlotMargin();
   void setTextColor(const QColor& textColor);
   void setTitleColor(const QColor& titleColor);
   void displayProperties();
   void setDocked(bool docked);

private:
   // Toolbar buttons
   QAction* mpOpenAction;
   QAction* mpPrintAction;

   // Plot widget
   AttachmentPtr<PlotWidget> mpPlotWidget;
   QLineEdit* mpClassificationEdit;
   QComboBox* mpMouseModeCombo;
   QLineEdit* mpMarginEdit;
   QCheckBox* mpContextMenuCheck;

   // Dock window
   PlotWindow* mpDockWindow;
   PlotWidget* mpDockPlotWidget;
   QCheckBox* mpDockedCheck;
   QCheckBox* mpDragDropCheck;
   QCheckBox* mpPropertiesCheck;
};

#endif
