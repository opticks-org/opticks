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
#include <QtGui/QWidget>

#include <string>
#include <vector>

class CustomTreeWidget;
class Feature;
class FileDescriptor;
class GraphicElement;
class GraphicObject;
class QAction;
class QActionGroup;
class QComboBox;
class QLabel;
class QLineEdit;
class QToolButton;
class QTreeWidgetItem;
class RasterElement;
class ShapeFile;

class ShapeFileOptionsWidget : public QWidget
{
   Q_OBJECT

public:
   ShapeFileOptionsWidget(FileDescriptor* pFileDesc, ShapeFile* pShapefile, GraphicElement* pDefaultGraphicElement,
      const std::vector<GraphicElement*>& graphicElements, RasterElement* pRaster, QWidget* pParent = NULL);
   virtual ~ShapeFileOptionsWidget();

protected:
   int getColumn(const QString& strField) const;
   std::vector<Feature*> addFeatures(GraphicElement* pGraphicElement, GraphicObject* pObject, QString& message);
   void applyFeatureClass(const std::string& className);

protected slots:
   void updateFilenames();
   void browse();
   void setShape(const QString& strShape);
   void displayFeatureContextMenu(const QPoint& pos);
   void featureDisplayModeChanged(QAction* pAction);
   void currentFeatureChanged(QTreeWidgetItem* pItem);
   void zoomToFeature(QTreeWidgetItem* pItem);
   void panToFeature(QTreeWidgetItem* pItem);
   void selectFeature(QTreeWidgetItem* pItem, bool select);
   void addFeature();
   void selectFeature(bool select);
   void removeFeature();
   void clearFeatures();
   void applyFeatureClass();
   void editFeatureClasses();
   void addField();
   void addField(const QString& strName, const QString& strType);
   void removeField();
   void removeField(const QString& strName);
   void setFieldValue(QTreeWidgetItem* pItem, int iColumn);
   void updateFieldValues();

private:
   ShapeFileOptionsWidget(const ShapeFileOptionsWidget& rhs);
   ShapeFileOptionsWidget& operator=(const ShapeFileOptionsWidget& rhs);

   FileDescriptor* mpFileDesc;
   ShapeFile* mpShapeFile;
   GraphicElement* mpDefaultGraphicElement;
   std::vector<GraphicElement*> mGraphicElements;
   RasterElement* mpGeoref;

   QLineEdit* mpFilePathEdit;
   QLineEdit* mpBaseNameEdit;
   QLabel* mpShpFileLabel;
   QLabel* mpShxFileLabel;
   QLabel* mpDbfFileLabel;
   QComboBox* mpShapeCombo;
   CustomTreeWidget* mpFeatureTree;
   QLineEdit* mpIntEdit;
   QLineEdit* mpDoubleEdit;
   QActionGroup* mpFeatureDisplayModeGroup;
   QAction* mpFeatureZoomAction;
   QAction* mpFeaturePanAction;
   QToolButton* mpSelectFeatureButton;

   QMap<QTreeWidgetItem*, Feature*> mFeatures;
};
#endif
