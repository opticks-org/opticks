/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorMap.h"
#include "FileResource.h"

#include <memory>
#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

ColorMap::ColorMap() :
   mIsDefault(false),
   mpGradient(NULL)
{
   resetToDefault();
}

ColorMap::ColorMap(const ColorMap& colorMap) :
   mTable(colorMap.mTable),
   mName(colorMap.mName),
   mIsDefault(colorMap.mIsDefault),
   mpGradient(NULL)
{
}

ColorMap::ColorMap(const std::string& filename) :
   mIsDefault(false),
   mpGradient(NULL)
{
   if (!loadFromFile(filename))
   {
      throw std::runtime_error("Bad ColorMap file in Colormap constructor");
   }
}

ColorMap::ColorMap(const std::string& name, const std::vector<ColorType>& table) :
   mIsDefault(false),
   mpGradient(NULL)
{
   if (!setTable(name, table))
   {
      throw std::runtime_error("Bad Color Table in ColorMap constructor");
   }
}

ColorMap::ColorMap(const std::string& name, const Gradient &gradient) :
   mIsDefault(false),
   mpGradient(NULL)
{
   mpGradient = new Gradient (gradient);
   if (!setTable(name, tableFromGradient(gradient)))
   {
      throw std::runtime_error("Bad Color Table in ColorMap constructor");
   }
}

ColorMap::~ColorMap()
{
   if (mpGradient != NULL)
   {
      delete mpGradient;
   }
}

ColorMap& ColorMap::operator=(const ColorMap&colorMap)
{
   Gradient* pOldGradient = mpGradient;
   mName = colorMap.mName;
   mTable = colorMap.mTable;
   mIsDefault = colorMap.mIsDefault;
   if (colorMap.mpGradient != NULL)
   {
      mpGradient = new Gradient(*colorMap.mpGradient);
   }
   else
   {
      mpGradient = NULL;
   }
   if (pOldGradient)
   {
      delete pOldGradient;
   }
   return *this;
}

bool ColorMap::operator==(const ColorMap& colorMap) const
{
   if ((mName == colorMap.mName) && (mIsDefault == colorMap.mIsDefault) && (mTable == colorMap.mTable))
   {
      return true;
   }

   return false;
}


const std::string& ColorMap::getName() const
{
   return mName;
}

bool ColorMap::isFullyOpaque() const
{
   bool opaque = true;

   for (unsigned int i = 0; i < mTable.size(); ++i)
   {
      if (mTable[i].mAlpha != 255)
      {
         opaque = false;
         break;
      }
   }

   return opaque;
}

bool ColorMap::tableIsValid(const std::vector<ColorType>& table)
{
   if (table.size() <= 0)
   {
      return false;
   }

   for (unsigned int i = 0; i < table.size(); ++i)
   {
      if (table[i].isValid() == false)
      {
         return false;
      }
   }

   return true;
}

static std::string stripTrailingWhiteSpace(std::string text)
{
   int index = text.size()-1;
   while (index >= 0 && isspace(text[index])) --index;
   ++index;
   return text.substr(0, index);
}

bool ColorMap::loadFromFile(const std::string& filename)
{
   QFile file(QString::fromStdString(filename));
   if (!file.open(QIODevice::ReadOnly))
   {
      return false;
   }

   return deserialize(file);
}

bool ColorMap::loadFromBuffer(const std::string& buffer)
{
   QByteArray byteBuffer(buffer.c_str());
   QBuffer buf(&byteBuffer);
   if (!buf.open(QIODevice::ReadOnly))
   {
      return false;
   }
   return deserialize(buf);
}

bool ColorMap::deserialize(QIODevice &io)
{
   const int BUFFER_SIZE = 256;
   char buffer[BUFFER_SIZE];

   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if ((strncmp(buffer, "Cosmec Color Table", 18) != 0) &&
      (strncmp(buffer, "COMET Color Table", 17) != 0) &&
      (strncmp(buffer, "COMET Color Gradient", 20) &&
      (strncmp(buffer, "Opticks Color Table", 19) != 0) &&
      (strncmp(buffer, "Opticks Color Gradient", 22))))
   {
      return false;
   }

   int version = 0;
   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (sscanf(buffer, "Version%d", &version) != 1)
   {
      return false;
   }

   if (version != 1 && version != 2 && version != 3 && version != 4) 
   {
      return false;
   }

   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   buffer[strlen(buffer)-1] = 0;
   std::string name = buffer;
   name = stripTrailingWhiteSpace(name);

   if (version == 4)
   {
      if (deserializeGradient(io, version) == false)
      {
         return false;
      }
      mName = name;
      mIsDefault = false;
      return true;
   }

   char buffer1[BUFFER_SIZE];
   char buffer2[BUFFER_SIZE];
   char buffer3[BUFFER_SIZE];
   char buffer4[BUFFER_SIZE];
   char buffer5[BUFFER_SIZE];
   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (version == 3)
   {
      if (sscanf(buffer, "%s%s%s%s%s", buffer1, buffer2, buffer3, buffer4, buffer5) != 5) 
      {
         return false;
      }
   }
   else
   {
      if (sscanf(buffer, "%s%s%s%s", buffer1, buffer2, buffer3, buffer4) != 4) 
      {
         return false;
      }
   }

   if (strcmp(buffer1, "Index") != 0)
   {
      return false;
   }

   if (strcmp(buffer2, "Red") != 0)
   {
      return false;
   }

   if (strcmp(buffer3, "Green") != 0)
   {
      return false;
   }

   if (strcmp(buffer4, "Blue") != 0)
   {
      return false;
   }

   if (version >= 3 && strcmp(buffer5, "Alpha") != 0) 
   {
      return false;
   }

   std::vector<ColorType> colorMap;
   for (int i = 0; ; ++i)
   {
      int count = 0;
      int index = 0;
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 255;
      if (io.readLine(buffer, BUFFER_SIZE) == -1)
      {
         break;
      }

      count = sscanf(buffer, "%d%d%d%d%d", &index, &red, &green, &blue, &alpha);
      if (count <= 0)
      {
         break;
      }

      if ((version == 3 && count != 5) || (version != 3 && count != 4)) 
      {
         return false;
      }

      if (index != i) 
      {
         return false;
      }
      colorMap.push_back(ColorType(red, green, blue, alpha));
   }

   if (version == 1 && colorMap.size() != VERSION_ONE_TABLE_SIZE)
   {
      return false;
   }

   if (!ColorMap::tableIsValid(colorMap))
   {
      return false;
   }

   mName = name;
   mTable.swap(colorMap);
   mIsDefault = false;

   return true;
}

bool ColorMap::deserializeGradient(QIODevice &io, int version)
{
   const int BUFFER_SIZE = 256;
   char buffer[BUFFER_SIZE];
   char buffer1[BUFFER_SIZE];
   char buffer2[BUFFER_SIZE];
   char buffer3[BUFFER_SIZE];
   char buffer4[BUFFER_SIZE];
   char buffer5[BUFFER_SIZE];

   std::auto_ptr<Gradient> pGradient (new Gradient);
   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (sscanf (buffer, "%s%d", buffer1, &pGradient->mStartPosition) != 2 && strcmp(buffer1, "Start") != 0)
   {
      return false;
   }

   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (sscanf (buffer, "%s%d", buffer1, &pGradient->mStopPosition) != 2 && strcmp(buffer1, "Stop") != 0)
   {
      return false;
   }

   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (sscanf (buffer, "%s%d", buffer1, &pGradient->mNumIndices) != 2 && strcmp(buffer1, "Indices") != 0)
   {
      return false;
   }

   if (io.readLine(buffer, BUFFER_SIZE) == -1)
   {
      return false;
   }

   if (sscanf(buffer, "%s%s%s%s%s", buffer1, buffer2, buffer3, buffer4, buffer5) != 5) 
   {
      return false;
   }

   if (strcmp(buffer1, "Pos") != 0) 
   {
      return false;
   }
   if (strcmp(buffer2, "Red") != 0) 
   {
      return false;
   }
   if (strcmp(buffer3, "Green") != 0) 
   {
      return false;
   }
   if (strcmp(buffer4, "Blue") != 0) 
   {
      return false;
   }
   if (strcmp(buffer5, "Alpha") != 0) 
   {
      return false;
   }
   int i = 0;
   for (i = 0; ; ++i)
   {
      int count = 0;
      int index = 0;
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 255;
      if (io.readLine(buffer, BUFFER_SIZE) == -1)
      {
         break;
      }
      count = sscanf(buffer, "%d%d%d%d%d", &index, &red, &green, &blue, &alpha);
      if (count <= 0) 
      {
         break;
      }
      if (count != 5)
      {
         return false;
      }
      Gradient::Control control;
      control.mColor = ColorType(red, green, blue, alpha);
      control.mPosition = index;
      pGradient->mControls.push_back(control);
   }

   if (i > Gradient::MAX_CONTROLS)
   {
      return false;
   }

   std::vector<ColorType> colorTable;
   std::vector<ColorType> table = tableFromGradient(*pGradient);
   colorTable.swap(table);
   if (!ColorMap::tableIsValid(colorTable)) 
   {
      return false;
   }
   if (mpGradient != NULL)
   {
      delete mpGradient;
      mpGradient = NULL;
   }
   mpGradient = pGradient.release();
   mTable.swap(colorTable);

   return true;
}

bool ColorMap::setTable(const std::string& name, const std::vector<ColorType>& table)
{
   if (!ColorMap::tableIsValid(table))
   {
      return false;
   }

   mName = name;
   mTable = table;
   mIsDefault = false;

   return true;
}

std::vector<ColorType> ColorMap::tableFromGradient(const Gradient &gradient)
{
   std::vector<ColorType> colormap;
   if (gradient.mControls.size() < 2 || gradient.mControls.size() > Gradient::MAX_CONTROLS)
   {
      return colormap;
   }

   if (gradient.mNumIndices < 2)
   {
      return colormap;
   }

   int i = 0;
   int prevPosition = 0;
   int gradSize = gradient.mControls.size();
   for (i = 0; i < gradSize; ++i)
   {
      if (gradient.mControls[i].mPosition < prevPosition || 
         gradient.mControls[i].mPosition >= gradient.mNumIndices)
      {
         return colormap;
      }
      prevPosition = gradient.mControls[i].mPosition;
   }

   colormap.resize(gradient.mNumIndices);
   int size = colormap.size();

   // Fill with gray
   for (i = 0; i < size; ++i)
   {
      int shade = (i*255 + (size-1)/2)/(size-1);
      colormap[i] = ColorType(shade, shade, shade);
   }

   // Fill from the start of the range up to the first primary
   int prevPrimary = 0;
   int nextPrimary = 1;
   for (i = gradient.mStartPosition; i < gradient.mControls[prevPrimary].mPosition; ++i)
   {
      colormap[i] = gradient.mControls[0].mColor;
   }

   // Fill between the first and last primary
   double denom = gradient.mControls[1].mPosition - gradient.mControls[0].mPosition;
   for (; i <= gradient.mStopPosition; ++i)
   {
      while (i > gradient.mControls[nextPrimary].mPosition ||
         gradient.mControls[prevPrimary].mPosition == gradient.mControls[nextPrimary].mPosition)
      {
         ++prevPrimary;
         ++nextPrimary;
         if (nextPrimary >= static_cast<int>(gradient.mControls.size()))
         {
            break;
         }
         denom = gradient.mControls[nextPrimary].mPosition - gradient.mControls[prevPrimary].mPosition;
      }
      if (nextPrimary >= static_cast<int>(gradient.mControls.size()))
      {
         break;
      }

      double frac = 0.0;
      if (i >= gradient.mControls[prevPrimary].mPosition)
      {
         frac = (i-gradient.mControls[prevPrimary].mPosition) / denom;
      }
      int red = static_cast<int>(gradient.mControls[prevPrimary].mColor.mRed * (1.0-frac) + 
         gradient.mControls[nextPrimary].mColor.mRed * frac + 0.5);
      int green = static_cast<int>(gradient.mControls[prevPrimary].mColor.mGreen * (1.0-frac) + 
         gradient.mControls[nextPrimary].mColor.mGreen * frac + 0.5);
      int blue = static_cast<int>(gradient.mControls[prevPrimary].mColor.mBlue * (1.0-frac) + 
         gradient.mControls[nextPrimary].mColor.mBlue * frac + 0.5);
      int alpha = static_cast<int>(gradient.mControls[prevPrimary].mColor.mAlpha * (1.0-frac) + 
         gradient.mControls[nextPrimary].mColor.mAlpha * frac + 0.5);
      colormap[i] = ColorType(red, green, blue, alpha);
   }

   // Fill between the last primary and the end of the range
   for (; i <= gradient.mStopPosition; ++i)
   {
      colormap[i] = gradient.mControls.back().mColor;
   }

   return colormap;
}

void ColorMap::resetToDefault()
{
   mTable.clear();
   mTable.reserve(VERSION_ONE_TABLE_SIZE);
   int i;
   for (i = 0; i < VERSION_ONE_TABLE_SIZE; ++i)
   {
      mTable.push_back(ColorType(i, i, i));
   }
   mName = "Default Grayscale";
   mIsDefault = true;
}

bool ColorMap::saveToFile(const std::string& filename) const
{
   QFile file(QString::fromStdString(filename));
   if (!file.open(QIODevice::WriteOnly))
   {
      return false;
   }
   return serialize(file);
}

bool ColorMap::saveToBuffer(std::string& buffer) const
{
   QByteArray byteBuffer(10 * 1024 * 1024, '\0'); // 10MiB should be plenty, if it's not, bump this
   QBuffer buf(&byteBuffer);
   if (!buf.open(QIODevice::WriteOnly))
   {
      return false;
   }
   if (!serialize(buf))
   {
      return false;
   }
   buffer = byteBuffer.data();
   return true;
}

bool ColorMap::serialize(QIODevice &io) const
{
   if (mpGradient != NULL)
   {
      return serializeGradient(io);
   }
   QTextStream out(&io);

   out << "Opticks Color Table" << endl;

   int version = 3;
   if (isFullyOpaque())
   {
      if (mTable.size() == 256)
      {
         version = 1;
      }
      else
      {
         version = 2;
      }
   }

   out << "Version " << version << endl;
   out << QString::fromStdString(mName) << endl;
   if (version == 3)
   {
      out << "Index\tRed\tGreen\tBlue\tAlpha" << endl;
   }
   else
   {
      out << "Index\tRed\tGreen\tBlue" << endl;
   }

   unsigned int i;
   for (i = 0; i < mTable.size(); ++i)
   {
      if (version == 3)
      {
         out << i << "\t" << mTable[i].mRed << "\t" << mTable[i].mGreen << "\t" << mTable[i].mBlue << "\t" <<
            mTable[i].mAlpha << endl;
      }
      else
      {
         out << i << "\t" << mTable[i].mRed << "\t" << mTable[i].mGreen << "\t" << mTable[i].mBlue << endl;
      }
   }

   return true;
}

bool ColorMap::serializeGradient(QIODevice &io) const
{
   if (mpGradient == NULL)
   {
      return false;
   }

   QTextStream out(&io);

   out << "Opticks Color Gradient" << endl;
   out << "Version\t4" << endl;
   out << QString::fromStdString(mName) << endl;
   out << "Start\t" << mpGradient->mStartPosition << endl;
   out << "Stop\t" << mpGradient->mStopPosition << endl;
   out << "Indices\t" << mpGradient->mNumIndices << endl;
   out << "Pos\tRed\tGreen\tBlue\tAlpha" << endl;
   unsigned int i;
   for (i = 0; i < mpGradient->mControls.size(); ++i)
   {
      out << mpGradient->mControls[i].mPosition << "\t"
          << mpGradient->mControls[i].mColor.mRed << "\t"
          << mpGradient->mControls[i].mColor.mGreen << "\t"
          << mpGradient->mControls[i].mColor.mBlue << "\t"
          << mpGradient->mControls[i].mColor.mAlpha << endl;
   }

   // Append the color table for greater interoperability potential. 
   // This is ignored on load by the application.
   out << "Index\tRed\tGreen\tBlue\tAlpha" << endl;
   for (i = 0; i < mTable.size(); ++i)
   {
      out << i << "\t" << mTable[i].mRed << "\t" << mTable[i].mGreen <<
         "\t" << mTable[i].mBlue << "\t" << mTable[i].mAlpha << endl;
   }
   return true;
}

const std::vector<ColorType>& ColorMap::getTable() const
{
   return mTable; 
}

const ColorMap::Gradient *ColorMap::getGradientDefinition() const
{
   return mpGradient;
}

const ColorType &ColorMap::operator[](int index) const
{ 
   return mTable[index];
}

ColorType &ColorMap::operator[](int index)
{
   return mTable[index]; 
}

bool ColorMap::isDefault() const 
{
   return mIsDefault;
}
