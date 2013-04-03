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
#include <QtGui/QListWidget>

#include "AttachmentPtr.h"
#include "PlotWidget.h"
#include "Window.h"

#include <boost/any.hpp>
#include <string>

class DockWindow;

class DesktopAPITestGui : public QDialog, public Window::SessionItemDropFilter
{
   Q_OBJECT

public:
   DesktopAPITestGui(QWidget* pParent = NULL);
   ~DesktopAPITestGui();

   using QDialog::accept;

   bool accept(SessionItem* pItem) const;

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void updateContextMenu(Subject& subject, const std::string& signal, const boost::any& value);
   void updatePropertiesDialog(Subject& subject, const std::string& signal, const boost::any& value);
   void dropSessionItem(Subject& subject, const std::string& signal, const boost::any& value);
   void docked(Subject& subject, const std::string& signal, const boost::any& value);
   void undocked(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void addBrowseButton(bool bAdd);
   void addPrintButton(bool bAdd);
   void enableMouseMode();
   void setCustomMouseMode();
   void setPlotMargin();
   void setBackgroundColor(const QColor& backgroundColor);
   void setPlotBackgroundColor(const QColor& backgroundColor);
   void setLegendBackgroundColor(const QColor& backgroundColor);
   void setTextColor(const QColor& textColor);
   void setTitleColor(const QColor& titleColor);
   void displayProperties();
   void setDocked(bool docked);
   void enableDrops(bool enable);

private:
   DesktopAPITestGui(const DesktopAPITestGui& rhs);
   DesktopAPITestGui& operator=(const DesktopAPITestGui& rhs);

   class SessionItemDropList : public QListWidget
   {
   public:
      SessionItemDropList(QWidget* pParent = NULL);
      virtual ~SessionItemDropList();

      void enableDrops(bool enable);

   protected:
      virtual void dragEnterEvent(QDragEnterEvent* pEvent);
      virtual void dropEvent(QDropEvent* pEvent);

   private:
      SessionItemDropList(const SessionItemDropList& rhs);
      SessionItemDropList& operator=(const SessionItemDropList& rhs);
      bool mSupportsDrops;
   };

   // Toolbar buttons
   QAction* mpOpenAction;
   QAction* mpPrintAction;

   // Plot widget
   AttachmentPtr<PlotWidget> mpPlotWidget;
   QComboBox* mpMouseModeCombo;
   QAction* mpMouseModeAction;
   QLineEdit* mpMarginEdit;
   QCheckBox* mpContextMenuCheck;

   // Dock window
   DockWindow* mpDockWindow;
   PlotWidget* mpDockPlotWidget;
   QCheckBox* mpDockedCheck;
   QCheckBox* mpSigDragDropCheck;
   QCheckBox* mpPropertiesCheck;

   // Drag-and-drop
   SessionItemDropList* mpDragDropList;
};

#endif
