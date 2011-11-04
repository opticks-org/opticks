/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAVARIANTEDITOR_H
#define DATAVARIANTEDITOR_H

#include "DataVariant.h"
#include "EnumWrapper.h"
#include "IntValidator.h"

#include <QtCore/QFileSystemWatcher>
#include <QtCore/QString>
#include <QtGui/QWidget>

#include <string>

class CustomColorButton;
class QDateTimeEdit;
class QDoubleValidator;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class QTextEdit;
class SymbolTypeGrid;

class DataVariantEditorDelegate
{
public:
   enum TypeEnum { INTEGRAL, DOUBLE, BOOL, DATE_TIME, ENUMERATION, VECTOR, DYNAMIC_OBJECT, TEXT, COLOR, SYMBOL_TYPE };
   typedef EnumWrapper<TypeEnum> Type;

   enum ButtonTypeEnum { BROWSE, EDIT };
   typedef EnumWrapper<ButtonTypeEnum> ButtonType;

   DataVariantEditorDelegate()
   {}

   DataVariantEditorDelegate(std::string typeName, Type theType, ButtonType buttonType = ButtonType()) :
      mType(theType),
      mTypeName(typeName),
      mButtonType(buttonType)
   {}

   Type getType() const
   {
      return mType;
   }

   void setType(Type newType)
   {
      mType = newType;
   }

   const std::string& getTypeName() const
   {
      return mTypeName;
   }

   void setTypeName(const std::string& newType)
   {
      mTypeName = newType;
   }

   ButtonType getButtonType() const
   {
      return mButtonType;
   }

   const std::vector<std::string>& getEnumValueStrings() const
   {
      return mEnumValues;
   }

   void setEnumValueStrings(const std::vector<std::string>& newValues)
   {
      mEnumValues = newValues;
   }

private:
   Type mType;
   std::string mTypeName;
   ButtonType mButtonType;
   std::vector<std::string> mEnumValues;
};

class DataVariantEditor : public QWidget
{
   Q_OBJECT

public:
   DataVariantEditor(QWidget* pParent = NULL);
   virtual ~DataVariantEditor();

   void setValue(const DataVariant& value, bool useVariantCurrentValue = true);
   const DataVariant &getValue();

   static const std::vector<DataVariantEditorDelegate>& getDelegates();
   static DataVariantEditorDelegate getDelegate(const std::string& type);
   static bool hasDelegate(const std::string& type);
   static const std::string& getVectorStringEditWarningDialogId();

   virtual QSize sizeHint() const;

signals:
   void modified();

protected slots:
   void setStackWidget(std::string type);
   void browse();
   void edit();
   void tempFileChanged();

private:
   DataVariantEditor(const DataVariantEditor& rhs);
   DataVariantEditor& operator=(const DataVariantEditor& rhs);
   QStackedWidget* mpStack;
   QLineEdit* mpValueLineEdit;
   QPushButton* mpBrowseButton;
   QPushButton* mpEditButton;
   QListWidget* mpValueList;
   QTextEdit* mpValueTextEdit;
   QRadioButton* mpFalseRadio;
   QRadioButton* mpTrueRadio;
   QDateTimeEdit* mpDateTimeEdit;
   CustomColorButton* mpColorEdit;
   SymbolTypeGrid* mpSymbolTypeEdit;

   IntValidator<int64_t>* mpIntValidator;
   IntValidator<uint64_t>* mpUIntValidator;
   QDoubleValidator* mpDoubleValidator;

   DataVariant mValue;
   static std::vector<DataVariantEditorDelegate> sDelegates;

   QString mTempFilename;
   QFileSystemWatcher mTempFileWatcher;
};

#endif
