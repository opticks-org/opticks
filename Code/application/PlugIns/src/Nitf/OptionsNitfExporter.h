/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSNITFEXPORTER_H
#define OPTIONSNITFEXPORTER_H

#include <QtGui/QWidget>

class Classification;
class DateTime;
class QDate;
class QLabel;
class QWidget;

class OptionsNitfExporter : public QWidget
{
   Q_OBJECT

public:
   OptionsNitfExporter(const Classification* const pClassification, QWidget* pParent = NULL);
   ~OptionsNitfExporter();

   bool isValid(std::string& errorMessage);
   std::string getModifiedValue(const QString& name);

   static const QString LEVEL;
   static const QString SYSTEM;
   static const QString CODEWORDS;
   static const QString FILE_CONTROL;
   static const QString FILE_RELEASING;
   static const QString DECLASSIFICATION_TYPE;
   static const QString DECLASSIFICATION_DATE;
   static const QString DECLASSIFICATION_EXEMPTION;
   static const QString FILE_DOWNGRADE;
   static const QString DOWNGRADE_DATE;
   static const QString AUTHORITY_TYPE;
   static const QString AUTHORITY;
   static const QString CLASSIFICATION_REASON;
   static const QString SOURCE_DATE;
   static const QString SECURITY_CONTROL_NUMBER;
   static const QString FILE_COPY_NUMBER;
   static const QString FILE_NUMBER_OF_COPIES;
   static const QString DESCRIPTION;

   private slots:
   void valueModified(const QString& newValue);
   void dateModified(const QDate& newDate);

private:
   struct OptionsNitfExporterElement
   {
      QString mName;
      std::string mOriginalValue;
      std::string mModifiedValue;
      QString mDefaultValue;
      bool mModifiedValueIsTruncated;
      unsigned int mMaxLength;
      QLabel* mpLabel;
      QWidget* mpSelf;
      QWidget* mpOther;

      OptionsNitfExporterElement(const QString& name, const std::string& originalValue,
         unsigned int maxLength, const QString& defaultValue, QWidget* pSelf, QWidget* pOther = NULL);

      void setValue(const std::string& newValue);
   };
   std::vector<OptionsNitfExporterElement> mElements;
   const std::string mDateTimeFormat;
   const std::string mDateParseFormat;
   const QString mQDateFormat;
   const Classification* const mpClassification;

   bool createLineEditElement(const QString& name, const QString& toolTip, const std::string& originalValue,
      unsigned int maxLength = 0, const QString& defaultValue = QString(),
      const QString& inputMask = QString());

   bool createComboBoxElement(const QString& name, const QString& toolTip, const std::string& originalValue,
      const QStringList& availableValues, unsigned int maxLength = 0);

   bool createDateEditElement(const QString& name, const QString& toolTip, const DateTime* pOriginalValue,
      unsigned int maxLength = 0);

   bool createElements();
};

#endif
