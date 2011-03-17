/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSIONPAGE_H
#define FUSIONPAGE_H

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

   void viewDeleted(Subject& subject, const std::string& signal, const boost::any& v);

   virtual void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);
   virtual SpatialDataView* getPrimaryView() const;
   virtual SpatialDataView* getSecondaryView() const;

   virtual std::string getPreferredPrimaryMouseMode() const;
   virtual std::string getPreferredSecondaryMouseMode() const;
   virtual Layer* getPreferredPrimaryActiveLayer() const;
   virtual Layer* getPreferredSecondaryActiveLayer() const;

   virtual bool isValid() const = 0;

signals:
   void modified();

private:
   SpatialDataView* mpPrimaryView;
   SpatialDataView* mpSecondaryView;
};

#endif
