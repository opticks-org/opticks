/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BmpDetails.h"

#include "AppVersion.h"

std::string BmpDetails::name()
{
   return "Bitmap";
}

std::string BmpDetails::shortDescription()
{
   return "BMP";
}

std::string BmpDetails::description()
{
   return "Bit MaP";
}

std::string BmpDetails::extensions()
{
   return "Bitmap Files (*.bmp)";
}

bool BmpDetails::savePict(QString strFilename, QImage img, const SessionItem *pItem)
{
   if ((strFilename.isEmpty() == true) || (img.isNull() == true))
   {
      return false;
   }

   return img.save(strFilename, "BMP");
}

bool BmpDetails::isProduction() const
{
   return APP_IS_PRODUCTION_RELEASE;
}
