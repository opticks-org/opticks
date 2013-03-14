/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QTCLUSTERGUI_H
#define QTCLUSTERGUI_H

#include "EnumWrapper.h"
#include "StringUtilities.h"
#include <QtGui/QDialog>

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;

enum DisplayTypeEnum
{
   CENTROID,
   BOUNDARY,
   CENTROID_AND_BOUNDARY,
   PSEUDO
};
typedef EnumWrapper<DisplayTypeEnum> DisplayType;

class QtClusterGui : public QDialog
{
   Q_OBJECT

public:
   QtClusterGui(QWidget* pParent = NULL);
   virtual ~QtClusterGui();

   double getClusterSize() const;
   QString getResultName() const;
   DisplayType getDisplayType() const;

public slots:
   void setClusterSize(double size);
   void setResultName(const QString& name);
   void setDisplayType(DisplayType type);

private:
   QtClusterGui(const QtClusterGui& rhs);
   QtClusterGui& operator=(const QtClusterGui& rhs);
   QDoubleSpinBox* mpClusterSize;
   QLineEdit* mpResultName;
   QComboBox* mpDisplayType;
};

namespace StringUtilities
{
   template<>
   std::string toDisplayString(const DisplayType& value, bool* pError);

   template<>
   std::string toXmlString(const DisplayType& value, bool* pError);

   template<>
   DisplayType fromDisplayString<DisplayType>(std::string valueText, bool* pError);

   template<>
   DisplayType fromXmlString<DisplayType>(std::string valueText, bool* pError);

   template<>
   std::string toDisplayString(const std::vector<DisplayType>& value, bool* pError);

   template<>
   std::string toXmlString(const std::vector<DisplayType>& value, bool* pError);

   template<>
   std::vector<DisplayType> fromDisplayString<std::vector<DisplayType> >(
      std::string valueText, bool* pError);

   template<>
   std::vector<DisplayType> fromXmlString<std::vector<DisplayType> >(
      std::string valueText, bool* pError);
}

#endif
