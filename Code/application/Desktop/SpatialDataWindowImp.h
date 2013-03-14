/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPATIALDATAWINDOWIMP_H
#define SPATIALDATAWINDOWIMP_H

#include "WorkspaceWindowImp.h"
#include "TypesFile.h"

#include <string>

class OverviewWindow;
class SpatialDataView;

class SpatialDataWindowImp : public WorkspaceWindowImp
{
   Q_OBJECT

public:
   SpatialDataWindowImp(const std::string& id, const std::string& windowName, QWidget* parent = 0);
   virtual ~SpatialDataWindowImp();

   using WorkspaceWindowImp::setIcon;
   using WorkspaceWindowImp::setName;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;

   View* createView(const std::string& viewName, const ViewType& viewType);
   void setWidget(QWidget* pWidget);

   SpatialDataView* getSpatialDataView() const;
   void exportSubset();

   void showOverviewWindow(bool bShow);
   bool isOverviewWindowShown() const;

signals:
   void overviewVisibilityChanged(bool bVisible);

private:
   SpatialDataWindowImp(const SpatialDataWindowImp& rhs);
   SpatialDataWindowImp& operator=(const SpatialDataWindowImp& rhs);

   OverviewWindow* mpOverview;
};

#define SPATIALDATAWINDOWADAPTEREXTENSION_CLASSES \
   WORKSPACEWINDOWADAPTEREXTENSION_CLASSES

#define SPATIALDATAWINDOWADAPTER_METHODS(impClass) \
   WORKSPACEWINDOWADAPTER_METHODS(impClass) \
   SpatialDataView* getSpatialDataView() const \
   { \
      return impClass::getSpatialDataView(); \
   } \
   void exportSubset() \
   { \
      impClass::exportSubset(); \
   } \
   void showOverviewWindow(bool bShow) \
   { \
      impClass::showOverviewWindow(bShow); \
   } \
   bool isOverviewWindowShown() const \
   { \
      return impClass::isOverviewWindowShown(); \
   }

#endif
