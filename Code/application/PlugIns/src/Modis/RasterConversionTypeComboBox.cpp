/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "RasterConversionTypeComboBox.h"
#include "StringUtilities.h"

#include <string>

RasterConversionTypeComboBox::RasterConversionTypeComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   addItem(QString::fromStdString(StringUtilities::toDisplayString(ModisUtilities::NO_CONVERSION)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(ModisUtilities::CONVERT_TO_RADIANCE)));
   addItem(QString::fromStdString(StringUtilities::toDisplayString(ModisUtilities::CONVERT_TO_REFLECTANCE)));
   setEditable(false);

   VERIFYNR(connect(this, SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(translateIndexChanged(const QString&))));
}

RasterConversionTypeComboBox::~RasterConversionTypeComboBox()
{}

void RasterConversionTypeComboBox::setRasterConversion(ModisUtilities::RasterConversionType rasterConversion)
{
   if (rasterConversion != getRasterConversion())
   {
      int index = -1;
      if (rasterConversion.isValid() == true)
      {
         std::string typeText = StringUtilities::toDisplayString(rasterConversion);
         if (typeText.empty() == false)
         {
            index = findText(QString::fromStdString(typeText));
         }
      }

      setCurrentIndex(index);
   }
}

ModisUtilities::RasterConversionType RasterConversionTypeComboBox::getRasterConversion() const
{
   if (currentIndex() != -1)
   {
      std::string typeText = currentText().toStdString();
      if (typeText.empty() == false)
      {
         return StringUtilities::fromDisplayString<ModisUtilities::RasterConversionType>(typeText);
      }
   }

   return ModisUtilities::RasterConversionType();
}

void RasterConversionTypeComboBox::translateIndexChanged(const QString& text)
{
   ModisUtilities::RasterConversionType rasterConversion;
   if (text.isEmpty() == false)
   {
      rasterConversion = StringUtilities::fromDisplayString<ModisUtilities::RasterConversionType>(text.toStdString());
   }

   emit rasterConversionChanged(rasterConversion);
}
