/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DOCKWINDOWIMP_H
#define DOCKWINDOWIMP_H

#include <QtGui/QAction>
#include <QtGui/QDockWidget>

#include "ViewWindowImp.h"

#include <list>
#include <string>

/**
 *  A window that is docked to the main application window.
 *
 *  A dock window is a window that can be attached to an edge of the main window or float
 *  freely anywhere on the screen.  The window provides all of the capability of a
 *  QDockWindow with the additional capability of being an View.  The setMargin() method
 *  can also be used to set an appropriate layout margin.
 *
 *  @see       ViewWindowImp
 */
class DockWindowImp : public QDockWidget, public ViewWindowImp
{
   Q_OBJECT

public:
   /**
    *  Creates the dock window.
    *
    *  @param   id
    *           The unique ID for the window.
    *  @param   windowName
    *           The name for the window.
    *  @param   parent
    *           The dock window parent widget.
    */
   DockWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);

   /**
    *  Destroys the dock window.
    */
   ~DockWindowImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   /**
    *  Returns the window type.
    *
    *  The implementation of this method returns the DOCK_WINDOW type.  Derived
    *  classes can override this implementation to set another type as necessary.
    *
    *  @return   The window type.
    */
   WindowType getWindowType() const;
   using WindowImp::setName;

   View* createView(const QString& strViewName, const ViewType& viewType);

   /**
    *  Sets the widget for the dock window
    *
    *  This method overrides the QDockWindow method to always reparent the widget
    *  to this window.  This is necessary when the widget is set from a plug-in.
    *
    *  @param   pWidget
    *           The widget to set as the primary widget in the dock window.
    */
   void setWidget(QWidget* pWidget);

   QWidget* getWidget() const;
   std::list<ContextMenuAction> getContextMenuActions() const;

   void saveState() const;
   void restoreState();
   bool isShown() const;

public slots:
   void dock();
   void undock();
   virtual void setVisible(bool visible);

signals:
   void visibilityChanged(bool bVisible);

protected:
   bool event(QEvent* pEvent);
   void showEvent(QShowEvent* pEvent);
   void hideEvent(QHideEvent* pEvent);
   void contextMenuEvent(QContextMenuEvent* pEvent);

protected slots:
   void undocked(bool isUndocked);

private:
   QAction* mpDockAction;
   QAction* mpUndockAction;
   QAction* mpShowAction;
   QAction* mpHideAction;
   QAction* mpSeparatorAction;

   WINDOWIMPDROP_METHODS(ViewWindowImp);
};

#define DOCKWINDOWADAPTEREXTENSION_CLASSES \
   VIEWWINDOWADAPTEREXTENSION_CLASSES

#define DOCKWINDOWADAPTER_METHODS(impClass) \
   VIEWWINDOWADAPTER_METHODS(impClass) \
   void setIcon(const QIcon& icon) \
   { \
      impClass::setIcon(icon); \
   } \
   void dock() \
   { \
      impClass::dock(); \
   } \
   void undock() \
   { \
      impClass::undock(); \
   } \
   void show() \
   { \
      impClass::show(); \
   } \
   void hide() \
   { \
      impClass::hide(); \
   } \
   bool isShown() const \
   { \
      return impClass::isShown(); \
   }

#endif
