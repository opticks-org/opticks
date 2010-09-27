/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLOR_TYPE_H
#define COLOR_TYPE_H

#include <algorithm>
#include <vector>

#include "AppConfig.h"
#include "DataVariantValidator.h"

/**
 *  @relates ColorType
 *
 *  Converts a ColorType to a QColor.
 *
 *  @param   colorType
 *           The ColorType to convert to a QColor.  If an invalid ColorType is
 *           passed in, an invalid QColor will be created.
 *
 *  @see     ColorType::isValid()
 */
#define COLORTYPE_TO_QCOLOR(colorType) \
   (colorType.isValid() ? QColor(colorType.mRed, colorType.mGreen, colorType.mBlue, colorType.mAlpha) : QColor())

/**
 *  @relates ColorType
 *
 *  Converts a QColor to a ColorType.
 *
 *  @param   qcolor
 *           The QColor to convert to a ColorType.  If an invalid QColor is
 *           passed in, an invalid ColorType will be created.
 *
 *  @see     ColorType::isValid()
 */
#define QCOLOR_TO_COLORTYPE(qcolor) \
   (qcolor.isValid() ? ColorType(qcolor.red(), qcolor.green(), qcolor.blue(), qcolor.alpha()) : ColorType())

/**
 *  An RGB triplet.
 */
class ColorType
{
public:
   /**
    *  Creates a ColorType object with given RGB values.
    *
    *  Valid RGB values range from 0 to 255.
    *
    *  @param   r
    *           The red value.
    *  @param   g
    *           The green value.
    *  @param   b
    *           The blue value.
    *  @param   a
    *           The alpha (opacity) value.
    */
   ColorType(unsigned int r,unsigned int g,unsigned int b, unsigned int a = 255){mRed = r;mGreen = g;mBlue = b; mAlpha = a;};

   /**
    *  Creates an invalid ColorType object.
    */
   ColorType() { mRed = -1; mGreen = -1; mBlue = -1; mAlpha = 255;};

   /**
    *  Returns a unique color that is farthest away in color space from given colors.
    *
    *  @param   excludedColors
    *           The colors from which the unique color is calculated.
    *
    *  @return  A color that is as different as possible from the given exclusion
    *           color.
    */
   static ColorType getUniqueColor(const std::vector<ColorType>& excludedColors)
   {
      unsigned int i, j, k;
      ColorType uniqueColor(0, 0, 0);

      if (excludedColors.size() == 0)
      {
         return uniqueColor;  // No excluded colors, so any color will do
      }

      if (excludedColors.size() == 1)
      {
         int maxDist = -1;
         int red = excludedColors[0].mRed;
         int green = excludedColors[0].mGreen;
         int blue = excludedColors[0].mBlue;

         for (i = 0; i < 256; i += 255)
         {
            for (j = 0; j < 256; j += 255)
            {
               for (k = 0; k < 256; k += 255)
               {
                  int dist = (i - red) * (i - red) + (j - green) * (j - green) + (k - blue) * (k - blue);
                  if (dist > maxDist)
                  {
                     maxDist = dist;
                     uniqueColor.mRed = i;
                     uniqueColor.mGreen = j;
                     uniqueColor.mBlue = k;
                  }
               }
            }
         }

         return uniqueColor;
      }

      std::vector<unsigned int> badColors;
      badColors.reserve(excludedColors.size());

      std::vector<ColorType>::const_iterator iter;
      for (iter = excludedColors.begin(); iter !=  excludedColors.end(); iter++)
      {
         badColors.push_back((iter->mBlue << 16) + (iter->mGreen << 8) + iter->mRed);
      }

      std::sort(badColors.begin(), badColors.end());

      unsigned int duplicates = 0;
      unsigned int next;
      for (i = 0, next = 1; i < badColors.size() - duplicates; i++)
      {
         for (j = next; j < badColors.size(); j++)
         {
            if (badColors[i] != badColors[j])
            {
               badColors[i + 1] = badColors[j];
               next = j + 1;
               break;
            }
            else
            {
               duplicates++;
            }
         }
      }

      badColors.erase(badColors.begin() + (badColors.size() - duplicates), badColors.end());

      std::vector<unsigned int>::iterator ulit = badColors.begin();
      unsigned int testColor;
      for (testColor = 0; testColor < 0xffffff; testColor++)
      {
         if (testColor == *ulit)
         {
            ulit++;
         }
         else
         {
            break;
         }
      }

      if (testColor == 0x01000000)  // Searched all 16.7 million colors and didn't find a unique one
      {
         uniqueColor = ColorType();
      }
      else
      {
         uniqueColor.mBlue = (testColor&0xff0000)>>16;
         uniqueColor.mGreen = (testColor&0xff00)>>8;
         uniqueColor.mRed = testColor&0xff;
      }

      return uniqueColor;
   }

   /**
   *  Generates one or more colors that are unique to each other.
   *
   *  @param   count
   *           The requested number of colors to generate.
   *  @param   colors
   *           A vector that is populated with the generated colors.
   *  @param   excludeColors
   *           Colors that should not be included in the generated colors.
   *
   *  @return  The number of unique colors that were generated.
   */
   static unsigned int getUniqueColors(unsigned int count, std::vector<ColorType>& colors,
      const std::vector<ColorType>& excludeColors)
   {
      unsigned int i, j, k, l;
      unsigned int numUnique = 0;
      double slValues[] = {0.0, 1.0, 0.5, 0.8, 0.3, 0.6, 0.9, 0.2, 0.7, 0.4, 0.1};
      ColorType baseColors[] =
      {
         ColorType(0,0,255),
         ColorType(0,255,0),
         ColorType(255,0,0),
         ColorType(0,255,255),
         ColorType(255,255,0),
         ColorType(255,0,255),
         ColorType(255,255,255)
      };

      for (i = 0; i < sizeof(slValues) / sizeof(slValues[0]); i++)
      {
         for (j = 0; j < sizeof(slValues) / sizeof(slValues[0]); j++)
         {
            for (k = 0; k < sizeof(baseColors) / sizeof(baseColors[0]); k++)
            {
               int newColor[3];
               int maxValue;

               newColor[0] = (int) (baseColors[k].mRed * slValues[j] + 0.5);
               newColor[1] = (int) (baseColors[k].mGreen * slValues[j] + 0.5);
               newColor[2] = (int) (baseColors[k].mBlue * slValues[j] + 0.5);

               maxValue = 0;
               for (l = 0; l < 3; l++)
               {
                  if (newColor[l] > maxValue)
                  {
                     maxValue = newColor[l];
                  }
               }

               maxValue = (int) (maxValue * slValues[i] + 0.5);
               for (l = 0; l < 3; l++)
               {
                  if (newColor[l] < maxValue)
                  {
                     newColor[l] = maxValue;
                  }
               }

               ColorType colorToInsert;
               colorToInsert.mRed = newColor[0];
               colorToInsert.mGreen = newColor[1];
               colorToInsert.mBlue = newColor[2];

               for (l=0; l<excludeColors.size(); l++)
               {
                  if (excludeColors[l].mRed == colorToInsert.mRed &&
                     excludeColors[l].mGreen == colorToInsert.mGreen &&
                     excludeColors[l].mBlue == colorToInsert.mBlue)
                  {
                     break;
                  }
               }
               if (l == excludeColors.size())
               {
                  for (l = 0; l < colors.size(); l++)
                  {
                     if (colors[l].mRed == colorToInsert.mRed &&
                        colors[l].mGreen == colorToInsert.mGreen &&
                        colors[l].mBlue == colorToInsert.mBlue)
                     {
                        break;
                     }
                  }
                  if (l == colors.size())
                  {
                     colors.push_back (colorToInsert);
                     ++numUnique;
                     if (colors.size() == count)
                     {
                        return numUnique;
                     }
                  }
               }
            }
         }
      }
      return numUnique;
   }

   BROKEN_INLINE_HINT ColorType& operator=(const ColorType& color)
   {
      if (this != &color)
      {
         mRed = color.mRed;
         mGreen = color.mGreen;
         mBlue = color.mBlue;
         mAlpha = color.mAlpha;
      }

      return *this;
   }

   bool operator!=(const ColorType& color) const
   {
      if ((mRed != color.mRed) || (mGreen != color.mGreen) || (mBlue != color.mBlue) || (mAlpha != color.mAlpha))
      {
         return true;
      }

      return false;
   }

   bool operator==(const ColorType& color) const
   {
      if ((mRed == color.mRed) && (mGreen == color.mGreen) && (mBlue == color.mBlue) && (mAlpha == color.mAlpha))
      {
         return true;
      }

      return false;
   }

   /**
    *  Queries the color for its validity.
    *
    *  A valid color has RGB values ranging from 0 to 255.
    *
    *  @return  TRUE if the color is valid, otherwise FALSE.
    */
   bool isValid() const
   {
      if ((mRed > -1) && (mRed < 256) &&
         (mGreen > -1) && (mGreen < 256) &&
         (mBlue > -1) && (mBlue < 256) &&
         (mAlpha > -1) && (mAlpha < 256))
      {
         return true;
      }

      return false;
   }

   int mRed;
   int mGreen;
   int mBlue;
   int mAlpha;
};

/**
 * \cond INTERNAL
 * These template specializations are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<ColorType> {};
/// \endcond

#endif
