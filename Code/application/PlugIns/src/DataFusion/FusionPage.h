/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSION_PAGE
#define FUSION_PAGE

#include <QtGui/QWidget>

#include <boost/any.hpp>

class Layer;
class SpatialDataView;
class Subject;

class FusionPage : public QWidget
{
   Q_OBJECT

public:
   FusionPage(QWidget* pParent);
   virtual ~FusionPage();

   void viewDeleted(Subject &subject, const std::string &signal, const boost::any &v);

   virtual void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);
   virtual SpatialDataView* getPrimaryView() const { return mpPrimaryView; }
   virtual SpatialDataView* getSecondaryView() const { return mpSecondaryView; }

   virtual std::string getPreferredPrimaryMouseMode() const { return ""; } 
   virtual std::string getPreferredSecondaryMouseMode() const { return ""; } 
   virtual Layer* getPreferredPrimaryActiveLayer() const { return NULL; }
   virtual Layer* getPreferredSecondaryActiveLayer() const { return NULL; }

   virtual bool isValid() const = 0;

signals:
   void modified();

private:
   SpatialDataView* mpPrimaryView;
   SpatialDataView* mpSecondaryView;
};

#endif
