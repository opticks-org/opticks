/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef SHAPEFILEDLG_H
#define SHAPEFILEDLG_H

#include <QtCore/QMap>
#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QTreeWidgetItem>

class CustomTreeWidget;
class Feature;
class ShapeFile;

class ShapeFileDlg : public QDialog
{
   Q_OBJECT

public:
   ShapeFileDlg(ShapeFile* pShapeFile, QWidget* parent = 0);
   ~ShapeFileDlg();

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
   void save();

private:
   ShapeFile* mpShapeFile;
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
