/*
* The information in this file is
* Copyright(c) 2008 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef SHAPEFILEOPTIONSWIDGET_H
#define SHAPEFILEOPTIONSWIDGET_H

#include <QtCore/QMap>
#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>

#include <vector>

class AoiElement;
class CustomTreeWidget;
class Feature;
class RasterElement;
class ShapeFile;

class ShapeFileOptionsWidget : public QWidget
{
   Q_OBJECT

public:
   ShapeFileOptionsWidget(ShapeFile* pShapefile, const std::vector<AoiElement*>& aois, RasterElement* pRaster);
   ~ShapeFileOptionsWidget();

protected:
   int getColumn(const QString& strField) const;

protected slots:
   void updateFilenames();
   void browse();
   void setShape(const QString& strShape);
   void addFeature();
   void removeFeature();
   void clearFeatures();
   void addField();
   void addField(const QString& strName, const QString& strType);
   void removeField();
   void removeField(const QString& strName);
   void setFieldValue(QTreeWidgetItem* pItem, int iColumn);
   void updateFieldValues();

private:
   ShapeFileOptionsWidget(const ShapeFileOptionsWidget& rhs);
   ShapeFileOptionsWidget& operator=(const ShapeFileOptionsWidget& rhs);
   ShapeFile* mpShapeFile;
   std::vector<AoiElement*> mAois;
   RasterElement* mpGeoref;
   QLineEdit* mpFilePathEdit;
   QLineEdit* mpBaseNameEdit;
   QLabel* mpShpFileLabel;
   QLabel* mpShxFileLabel;
   QLabel* mpDbfFileLabel;
   QComboBox* mpShapeCombo;
   CustomTreeWidget* mpFeatureTree;
   QMap<QTreeWidgetItem*, Feature*> mFeatures;
};
#endif
