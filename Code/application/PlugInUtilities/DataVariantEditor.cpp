/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariantEditor.h"

#include "AppVerify.h"
#include "ColorType.h"
#include "CustomColorButton.h"
#include "DateTimeImp.h"
#include "FilenameImp.h"
#include "IconImages.h"
#include "StringUtilities.h"
#include "DynamicObjectAdapter.h"
#include "ObjectResource.h"
#include "SymbolTypeGrid.h"

#include <QtCore/QDateTime>
#include <QtGui/QBitmap>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QValidator>
#include <string>
#include <vector>
using namespace std;

vector<DataVariantEditorDelegate> DataVariantEditor::sDelegates;

DataVariantEditor::DataVariantEditor(QWidget* parent) :
   QWidget(parent)
{
   // Value
   QLabel* pValueLabel = new QLabel("Value:", this);

   mpStack = new QStackedWidget(this);

   QWidget* pValueWidget = new QWidget(mpStack);
   mpValueLineEdit = new QLineEdit(pValueWidget);

   QVBoxLayout* pValueLayout = new QVBoxLayout(pValueWidget);
   pValueLayout->setMargin(0);
   pValueLayout->setSpacing(5);
   pValueLayout->addWidget(mpValueLineEdit);
   pValueLayout->addStretch();

   mpValueList = new QListWidget(mpStack);
   mpValueTextEdit = new QTextEdit(mpStack);

   QWidget* pBoolWidget = new QWidget(mpStack);
   QGroupBox* pBoolGroup = new QGroupBox(pBoolWidget);
   mpFalseRadio = new QRadioButton("False", pBoolGroup);
   mpTrueRadio = new QRadioButton("True", pBoolGroup);

   QVBoxLayout* pBoolGroupGrid = new QVBoxLayout(pBoolGroup);
   pBoolGroupGrid->setMargin(10);
   pBoolGroupGrid->setSpacing(5);
   pBoolGroupGrid->addWidget(mpFalseRadio);
   pBoolGroupGrid->addWidget(mpTrueRadio);

   QVBoxLayout* pBoolGrid = new QVBoxLayout(pBoolWidget);
   pBoolGrid->setMargin(0);
   pBoolGrid->setSpacing(10);
   pBoolGrid->addWidget(pBoolGroup, 0, Qt::AlignLeft);
   pBoolGrid->addStretch();

   QWidget* pDateTimeWidget = new QWidget(mpStack);
   mpDateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), pDateTimeWidget);

   QVBoxLayout* pDateTimeLayout = new QVBoxLayout(pDateTimeWidget);
   pDateTimeLayout->setMargin(0);
   pDateTimeLayout->setSpacing(5);
   pDateTimeLayout->addWidget(mpDateTimeEdit);
   pDateTimeLayout->addStretch(10);

   mpIntValidator = new QIntValidator(NULL);
   mpDoubleValidator = new QDoubleValidator(NULL);

   QWidget* pColorWidget = new QWidget(mpStack);
   mpColorEdit = new CustomColorButton(pColorWidget);
   QVBoxLayout* pColorGrid = new QVBoxLayout(pColorWidget);
   pColorGrid->setMargin(0);
   pColorGrid->setSpacing(10);
   pColorGrid->addWidget(mpColorEdit, 0, Qt::AlignLeft);
   pColorGrid->addStretch();

   QWidget* pSymbolTypeWidget = new QWidget(mpStack);
   mpSymbolTypeEdit = new SymbolTypeGrid(pSymbolTypeWidget);
   mpSymbolTypeEdit->setBorderedSymbols(true);
   QVBoxLayout* pSymbolTypeGrid = new QVBoxLayout(pSymbolTypeWidget);
   pSymbolTypeGrid->setMargin(0);
   pSymbolTypeGrid->setSpacing(10);
   pSymbolTypeGrid->addWidget(mpSymbolTypeEdit, 0, Qt::AlignLeft);
   pSymbolTypeGrid->addStretch();

   mpStack->addWidget(pValueWidget);
   mpStack->addWidget(mpValueList);
   mpStack->addWidget(mpValueTextEdit);
   mpStack->addWidget(pBoolWidget);
   mpStack->addWidget(pDateTimeWidget);
   mpStack->addWidget(pColorWidget);
   mpStack->addWidget(pSymbolTypeWidget);

   // Browse button
   QPixmap pixOpen(IconImages::OpenIcon);
   pixOpen.setMask(pixOpen.createHeuristicMask());
   QIcon icnBrowse(pixOpen);

   mpBrowseButton = new QPushButton(icnBrowse, QString(), this);
   mpBrowseButton->setFixedWidth(27);
   mpBrowseButton->hide();

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pValueLabel, 0, 0, 1, 2);
   pGrid->addWidget(mpStack, 1, 0);
   pGrid->addWidget(mpBrowseButton, 1, 1, Qt::AlignTop);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setStackWidget("");

   // Connections
   VERIFYNR(connect(mpBrowseButton, SIGNAL(clicked()), this, SLOT(browse())));
   VERIFYNR(connect(mpValueLineEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpValueList, SIGNAL(itemSelectionChanged()), this, SIGNAL(modified())));
   VERIFYNR(connect(mpValueTextEdit, SIGNAL(textChanged()), this, SIGNAL(modified())));
   VERIFYNR(connect(mpFalseRadio, SIGNAL(toggled(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpTrueRadio, SIGNAL(toggled(bool)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpDateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime&)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpColorEdit, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(modified())));
   VERIFYNR(connect(mpSymbolTypeEdit, SIGNAL(valueChanged(SymbolType)), this, SIGNAL(modified())));
}

DataVariantEditor::~DataVariantEditor()
{
}

const vector<DataVariantEditorDelegate>& DataVariantEditor::getDelegates()
{
   if (sDelegates.empty() == true)
   {
      sDelegates.push_back(DataVariantEditorDelegate("char", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("unsigned char", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("short", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("unsigned short", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("int", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("unsigned int", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("long", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("unsigned long", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("int64_t", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("Int64", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("UInt64", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("uint64_t", DataVariantEditorDelegate::INTEGRAL));
      sDelegates.push_back(DataVariantEditorDelegate("float", DataVariantEditorDelegate::DOUBLE));
      sDelegates.push_back(DataVariantEditorDelegate("double", DataVariantEditorDelegate::DOUBLE));
      sDelegates.push_back(DataVariantEditorDelegate("bool", DataVariantEditorDelegate::BOOL));
      sDelegates.push_back(DataVariantEditorDelegate("string", DataVariantEditorDelegate::TEXT));
      sDelegates.push_back(DataVariantEditorDelegate("Filename", DataVariantEditorDelegate::TEXT, true));
      sDelegates.push_back(DataVariantEditorDelegate("ColorType", DataVariantEditorDelegate::COLOR));
      sDelegates.push_back(DataVariantEditorDelegate("DateTime", DataVariantEditorDelegate::DATE_TIME));
      DataVariantEditorDelegate temp = DataVariantEditorDelegate("DisplayMode", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<DisplayMode>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("EncodingType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<EncodingType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("EndianType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<EndianType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("GeocoordType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<GeocoordType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("InterleaveFormatType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<InterleaveFormatType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("LayerType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<LayerType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("UnitType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<UnitType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("PassArea", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<PassArea>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("ProcessingLocation", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<ProcessingLocation>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("RasterChannelType", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<RasterChannelType>());
      sDelegates.push_back(temp);
      temp = DataVariantEditorDelegate("RegionUnits", DataVariantEditorDelegate::ENUMERATION);
      temp.setEnumValueStrings(StringUtilities::getAllEnumValuesAsDisplayString<RegionUnits>());
      sDelegates.push_back(temp);
      sDelegates.push_back(DataVariantEditorDelegate("SymbolType", DataVariantEditorDelegate::SYMBOL_TYPE));
      sDelegates.push_back(DataVariantEditorDelegate("vector<char>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<unsigned char>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<short>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<unsigned short>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<int>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<unsigned int>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<long>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<unsigned long>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<int64_t>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<uint64_t>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<Int64>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<UInt64>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<float>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<double>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<bool>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<string>", DataVariantEditorDelegate::VECTOR));
      sDelegates.push_back(DataVariantEditorDelegate("vector<Filename>", DataVariantEditorDelegate::VECTOR, true));
      sDelegates.push_back(DataVariantEditorDelegate("DynamicObject", DataVariantEditorDelegate::DYNAMIC_OBJECT));
   }

   return sDelegates;
}

void DataVariantEditor::setValue(const DataVariant& value, bool useVariantCurrentValue)
{
   // Type
   string type = value.getTypeName();

   // Value
   mpValueLineEdit->clear();
   mpValueTextEdit->clear();

   if (value.isValid() == false)
   {
      return;
   }

   // Get the value string
   string valueText = value.toDisplayString();

   QString strValue = QString::fromStdString(valueText);

   // Update the widgets
   DataVariantEditorDelegate curDelegate = getDelegate(type);
   if (curDelegate.getType() == DataVariantEditorDelegate::BOOL)
   {
      string value = strValue.toStdString();
      if (value == StringUtilities::toDisplayString(true) || (!useVariantCurrentValue))
      {
         mpTrueRadio->setChecked(true);
      }
      else if (value == StringUtilities::toDisplayString(false))
      {
         mpFalseRadio->setChecked(true);
      }
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DATE_TIME)
   {
      QDateTime dateTime;
      if (useVariantCurrentValue)
      {
         char month[128];
         int iMonth = 0;
         int iDay = 0;
         int iYear = 0;
         int iHour = 0;
         int iMinute = 0;
         int iSecond = 0;

         int iValues = sscanf(valueText.c_str(), "%s %d, %d, %d:%d:%d", &month, &iDay, &iYear,
            &iHour, &iMinute, &iSecond);
         if (iValues > 0)
         {
            iMonth = DateTimeImp::getMonth(string(month));
         }
         dateTime = QDateTime(QDate(iYear, iMonth, iDay), QTime(iHour, iMinute, iSecond));
      }
      mpDateTimeEdit->setDateTime(dateTime);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::ENUMERATION)
   {
      mpValueList->clear();
      QStringList items;
      vector<string> enumValues = curDelegate.getEnumValueStrings();
      for (vector<string>::iterator iter = enumValues.begin();
           iter != enumValues.end(); ++iter)
      {
         items.append(QString::fromStdString(*iter));
      }
      mpValueList->addItems(items);

      int iValue = 0;
      if (useVariantCurrentValue)
      {
         QList<QListWidgetItem*> listItems = mpValueList->findItems(strValue, Qt::MatchExactly);
         if (listItems.empty() == false)
         {
            QListWidgetItem* pItem = listItems.front();
            if (pItem != NULL)
            {
               iValue = mpValueList->row(pItem);
            }
         }
      }

      mpValueList->setCurrentRow(iValue);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::VECTOR)
   {
      if (useVariantCurrentValue)
      {
         QStringList strlValues = strValue.split(", ");
         for (int j = 0; j < strlValues.count(); j++)
         {
            QString strLine = strlValues[j];
            if (strLine.isEmpty() == false)
            {
               mpValueTextEdit->append(strLine);
            }
         }
      }
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::COLOR)
   {
      if (useVariantCurrentValue)
      {
         mpColorEdit->setColor(dv_cast<ColorType>(value));
      }
      else
      {
         mpColorEdit->setColor(ColorType());
      }
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::SYMBOL_TYPE)
   {
      if (useVariantCurrentValue)
      {
         mpSymbolTypeEdit->setCurrentValue(dv_cast<SymbolType>(value));
      }
      else
      {
         mpSymbolTypeEdit->setCurrentValue(SymbolType());
      }
   }
   else
   {
      if (useVariantCurrentValue)
      {
         mpValueLineEdit->setText(strValue);
      }
   }

   setStackWidget(type);
   mValue = value;
}

const DataVariant& DataVariantEditor::getValue()
{
   QString strValue;

   DataVariantEditorDelegate curDelegate = getDelegate(mValue.getTypeName());
   if (curDelegate.getType() == DataVariantEditorDelegate::BOOL)
   {
      if (mpTrueRadio->isChecked() == true)
      {
         strValue = QString::fromStdString(StringUtilities::toDisplayString(true));
      }
      else if (mpFalseRadio->isChecked() == true)
      {
         strValue = QString::fromStdString(StringUtilities::toDisplayString(false));
      }
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DATE_TIME)
   {
      QDateTime dateTimeValue = mpDateTimeEdit->dateTime();
      strValue = dateTimeValue.toString("MMMM d, yyyy, hh:mm:ss");
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::ENUMERATION)
   {
      QListWidgetItem* pItem = mpValueList->currentItem();
      if (pItem != NULL)
      {
         strValue = pItem->text();
      }
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DYNAMIC_OBJECT)
   {
      FactoryResource<DynamicObject> pObj;
      mValue = *pObj.get();
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::VECTOR)
   {
      strValue = mpValueTextEdit->toPlainText();
      strValue.replace("\n", ", ");
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::COLOR)
   {
      mValue = mpColorEdit->getColorType();
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::SYMBOL_TYPE)
   {
      mValue = mpSymbolTypeEdit->getCurrentValue();
   }
   else
   {
      strValue = mpValueLineEdit->text();
   }

   if (mValue.isValid())
   {
      string valueText = strValue.toStdString();

      mValue.fromDisplayString(mValue.getTypeName(), valueText);
   }

   return mValue;
}

void DataVariantEditor::setStackWidget(string type)
{
   mpValueLineEdit->setEnabled(true);
   mpBrowseButton->hide();

   DataVariantEditorDelegate curDelegate = getDelegate(type);
   if (curDelegate.getTypeName().empty())
   {
      return;
   }

   if (curDelegate.getNeedBrowseButton() == true)
   {
      mpBrowseButton->show();
   }

   if (curDelegate.getType() == DataVariantEditorDelegate::BOOL)
   {
      mpStack->setCurrentIndex(3);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DATE_TIME)
   {
      mpStack->setCurrentIndex(4);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::INTEGRAL)
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Consider using one or more custom QValidator subclasses "\
   "and removing mpIntValidator when implementing OPTICKS-588 (dadkins)")
      mpValueLineEdit->setValidator(NULL);
      mpStack->setCurrentIndex(0);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DOUBLE)
   {
      mpValueLineEdit->setValidator(mpDoubleValidator);
      mpStack->setCurrentIndex(0);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::TEXT)
   {
      mpValueLineEdit->setValidator(NULL);
      mpStack->setCurrentIndex(0);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::ENUMERATION)
   {
      mpStack->setCurrentIndex(1);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::DYNAMIC_OBJECT)
   {
      mpStack->setCurrentIndex(0);
      mpValueLineEdit->setEnabled(false);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::VECTOR)
   {
      mpStack->setCurrentIndex(2);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::COLOR)
   {
      mpStack->setCurrentIndex(5);
   }
   else if (curDelegate.getType() == DataVariantEditorDelegate::SYMBOL_TYPE)
   {
      mpStack->setCurrentIndex(6);
   }
   else
   {
      VERIFYNRV_MSG(false, "Unrecognized editor type value");
   }
}

void DataVariantEditor::browse()
{
   QWidget* pVisible = mpStack->currentWidget();
   if (pVisible == mpValueTextEdit)
   {
      QStringList strlFilenames = QFileDialog::getOpenFileNames(this, "Get Filenames", QString(), "All Files (*.*)");
      if (strlFilenames.isEmpty() == true)
      {
         return;
      }

      mpValueTextEdit->clear();

      int iCount = 0;
      iCount = strlFilenames.count();
      for (int i = 0; i < iCount; i++)
      {
         QString strFilename = strlFilenames[i];
         if (strFilename.isEmpty() == false)
         {
            mpValueTextEdit->append(strFilename);
         }
      }
   }
   else
   {
      QString strFilename = QFileDialog::getOpenFileName(this, "Get Filename", QString(), "All Files (*.*)");
      if (strFilename.isEmpty() == false)
      {
         mpValueLineEdit->setText(strFilename);
      }
   }
}

DataVariantEditorDelegate DataVariantEditor::getDelegate(const string& type)
{
   DataVariantEditorDelegate retVal;

   const vector<DataVariantEditorDelegate>& delegates = getDelegates();
   for (vector<DataVariantEditorDelegate>::const_iterator iter = delegates.begin(); iter != delegates.end(); ++iter)
   {
      if (iter->getTypeName() == type)
      {
         retVal = *iter;
         break;
      }
   }
   return retVal;
}

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Use a QListView to allow editing of vector data, " \
   "since the fromDisplayString parser is brittle? (kstreith)")
