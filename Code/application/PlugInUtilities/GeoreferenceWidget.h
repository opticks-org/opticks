/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCEWIDGET_H
#define GEOREFERENCEWIDGET_H

#include "PlugIn.h"
#include "TypesFile.h"

#include <QtCore/QMetaType>
#include <QtGui/QWidget>

#include <boost/any.hpp>
#include <string>

class DmsFormatTypeComboBox;
class GeocoordTypeComboBox;
class Georeference;
class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class RasterDataDescriptor;
class Subject;

Q_DECLARE_METATYPE(PlugIn*)

class GeoreferenceWidget : public QWidget
{
   Q_OBJECT

public:
   GeoreferenceWidget(QWidget* pParent = NULL);
   virtual ~GeoreferenceWidget();

   void setDataDescriptor(RasterDataDescriptor* pDescriptor);
   RasterDataDescriptor* getDataDescriptor();
   const RasterDataDescriptor* getDataDescriptor() const;

   Georeference* getSelectedPlugIn() const;

protected:
   enum ItemDataRole
   {
      PlugInRole = Qt::UserRole,
      WidgetRole = Qt::UserRole + 1
   };

   QListWidgetItem* addPlugInItem(const std::string& plugInName);
   void removePlugInItem(QListWidgetItem* pItem);
   void updatePlugInItems();
   void updateNoWidgetLabel();

   void metadataModified(Subject& subject, const std::string& signal, const boost::any& value);
   void georeferencePlugInNameModified(Subject& subject, const std::string& signal, const boost::any& value);
   void createLayerModified(Subject& subject, const std::string& signal, const boost::any& value);
   void layerNameModified(Subject& subject, const std::string& signal, const boost::any& value);
   void displayLayerModified(Subject& subject, const std::string& signal, const boost::any& value);
   void geocoordTypeModified(Subject& subject, const std::string& signal, const boost::any& value);
   void latLonFormatModified(Subject& subject, const std::string& signal, const boost::any& value);
   void descriptorDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setGeoreferencePlugIn();
   void setCreateLayer(bool createLayer);
   void setLayerName(const QString& layerName);
   void setGeocoordType(GeocoordType geocoordType);
   void setLatLonFormat(DmsFormatType latLonFormat);
   void setDisplayLayer(bool displayLayer);

private:
   GeoreferenceWidget(const GeoreferenceWidget& rhs);
   GeoreferenceWidget& operator=(const GeoreferenceWidget& rhs);

   RasterDataDescriptor* mpDescriptor;

   QListWidget* mpPlugInList;
   QStackedWidget* mpPlugInStack;
   QLabel* mpNoWidgetLabel;
   QCheckBox* mpCreateLayerCheck;
   QLabel* mpLayerNameLabel;
   QLineEdit* mpLayerNameEdit;
   QLabel* mpCoordTypeLabel;
   GeocoordTypeComboBox* mpCoordTypeCombo;
   QLabel* mpLatLonFormatLabel;
   DmsFormatTypeComboBox* mpLatLonFormatCombo;
   QCheckBox* mpDisplayLayerCheck;
};

#endif
