/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHSWIDGET_H
#define WAVELENGTHSWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QWidget>

#include "DimensionDescriptor.h"
#include "TypesFile.h"

#include <boost/any.hpp>
#include <string>
#include <vector>

class DynamicObject;
class QPushButton;
class Subject;
class WavelengthModel;
class Wavelengths;
class WavelengthUnitsComboBox;

class WavelengthsWidget : public QWidget
{
   Q_OBJECT

public:
   WavelengthsWidget(QWidget* pParent = NULL);
   ~WavelengthsWidget();

   void setWavelengths(const std::vector<DimensionDescriptor>& bands, DynamicObject* pWavelengthData);
   void setWavelengths(const std::vector<DimensionDescriptor>& bands, Wavelengths* pWavelengths);

   QSize sizeHint() const;

public slots:
   void highlightActiveBands(const std::vector<DimensionDescriptor>& bands);

protected:
   void setWavelengths(const std::vector<DimensionDescriptor>& bands, DynamicObject* pWavelengthData,
      Wavelengths* pWavelengths);
   void wavelengthDataDeleted(Subject& subject, const std::string& signal, const boost::any& value);

   void showEvent(QShowEvent* pEvent);

protected slots:
   void initializeWavelengthsFromElement();
   void loadWavelengths();
   void saveWavelengths();
   void wavelengthChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
   void calculateFwhm();
   void applyScaleFactor();
   void convertWavelengths(WavelengthUnitsType newUnits);
   void updateWidgetsFromWavelengths();

private:
   DynamicObject* mpWavelengthData;
   Wavelengths* mpWavelengths;

   WavelengthModel* mpWavelengthModel;
   QPushButton* mpSaveButton;
   QPushButton* mpFwhmButton;
   QPushButton* mpScaleButton;
   WavelengthUnitsComboBox* mpUnitsCombo;

   static const QString mMetadataFilter;
   static const QString mTextFilter;
};

class WavelengthItemDelegate : public QStyledItemDelegate
{
public:
   WavelengthItemDelegate(QObject* pParent = NULL);
   ~WavelengthItemDelegate();

   QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
   void setEditorData(QWidget* pEditor, const QModelIndex& index) const;
   void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const;
   void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif
