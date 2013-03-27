/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOGRAPHICFEATURESWIDGET_H
#define GEOGRAPHICFEATURESWIDGET_H

#include <QtGui/QStackedWidget>

#include <boost/any.hpp>
#include <string>

class GraphicLayer;
class Layer;
class QComboBox;
class Subject;

class GeographicFeaturesWidget : public QStackedWidget
{
   Q_OBJECT

public:
   GeographicFeaturesWidget(QWidget* pParent = NULL);
   virtual ~GeographicFeaturesWidget();

   bool displayLayer(GraphicLayer* pLayer);

protected:
   void windowAdded(Subject& subject, const std::string& signal, const boost::any& data);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& data);
   void layerAdded(Subject& subject, const std::string& signal, const boost::any& data);
   void layerDeleted(Subject& subject, const std::string& signal, const boost::any& data);
   void featureClassDeleted(Subject& subject, const std::string& signal, const boost::any& data);
   void initializeSession(Subject& subject, const std::string& signal, const boost::any& data);

   void removeLayer(Layer* pLayer);

private:
   QComboBox* mpViewCombo;
   QStackedWidget* mpViewStack;
};

#endif