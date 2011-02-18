/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLORMAP_H
#define COLORMAP_H

#include "ColorType.h"

#include <stdio.h>
#include <string>
#include <vector>

class QIODevice;

/**
 * This class manages a color map.
 *
 * A colormap can be a hold any number of colors, and may contain RGB or RGBA
 * color.  Each channel must have a value of between 0 and 256.
 */
class ColorMap
{
public:

   /**
    * This struct defines a color table.
    *
    * This struct specifies a color lookup table by defining a series of colors and their
    * positions in the lookup table. All entries in the lookup table that fall between
    * defined colors will be computed by interpolating between the defined color Controls.
    */
   struct Gradient
   {
      /**
       * This struct specifies a color and position in a color gradient.
       */
      struct Control
      {
         ColorType mColor;   /**< Specifies a color in the Gradient. */
         int mPosition;   /**< The index of the color specified by the control. Indices between 
                          two controls will be linearly interpolated between the controls. */
      };
      static const int MAX_CONTROLS = 20;   /**< The maximum number of controls that can 
                                            be specified. If more than this number of controls are specified 
                                            the Gradient will be rejected. */
      std::vector<Control> mControls;   /**< The controls specifying the color lookup table 
                                        defined by the Gradient. The positions of the controls must be 
                                        monotonically increasing (i.e. the position of Control n must be 
                                        greater than or equal to the position of Control n-1). The position 
                                        of the first Control must be greater than or equal to mStartPosition. 
                                        The position of the last Control must be less than or equal to 
                                        mStopPosition. */
      int mStartPosition;   /**< The index in the color lookup table that the gradient begins. 
                            All entries in the table before this value will be filled with grayscale values. 
                            All entries in the table after this value but before the first control will be 
                            filled with the color of the first control. If no grayscale is desired, this 
                            should be set to 0. This value must be greater than or equal to 0 and less 
                            than or equal to the position of the first Control. */
      int mStopPosition;   /**< The index in the color lookup table that the gradient ends. 
                            All entries in the table after this value will be filled with grayscale values. 
                            All entries in the table before this value but after the last control will be 
                            filled with the color of the last control. If no grayscale is desired, this 
                            should be set to mNumIndices-1. This value must be less than mNumIndices 
                            and greater than or equal to the position of the last Control. */
      int mNumIndices;   /**< The number of colors in the color lookup table defined by the Gradient. */
   };

   /**
    * Construct a default ColorMap.
    */
   ColorMap();

   /**
    * Construct a ColorMap by loading it from the file specified.
    * The object's name is specified within the file.
    *
    * @param filename
    *        The file to load the ColorMap from.
    *
    * @throw std::runtime_error
    *        This exception is thrown if the file load fails.
    */
   explicit ColorMap(const std::string& filename);

   /**
    * Construct a new ColorMap from a vector of ColorType.
    *
    * @param name
    *        The name of the new ColorMap.
    * @param table
    *        The colors to place in the ColorMap.
    *
    * @throw std::runtime_error
    *        This exception is thrown if there is an invalid color specified.
    */
   ColorMap(const std::string& name, const std::vector<ColorType>& table);

   /**
    * Construct a new ColorMap from a Gradient definition.
    *
    * @param name
    *        The name of the new ColorMap.
    * @param gradient
    *        The gradient object defining the colormap
    *
    * @throw std::runtime_error
    *        This exception is thrown if there is an invalid color specified.
    */
   ColorMap(const std::string& name, const Gradient &gradient);

   /**
    * Copy an existing ColorMap.
    *
    * The new ColorMap will have the same name as the existing one.
    *
    * @param colorMap
    *        The ColorMap to copy.
    */
   ColorMap(const ColorMap& colorMap);

   /**
    * Frees all resources associated with a ColorMap.
    */
   ~ColorMap();

   /**
    * Assign from an existing ColorMap.
    *
    * The assigned ColorMap will have the same name as the existing one.
    *
    * @param colorMap
    *        The ColorMap to assign from.
    */
   ColorMap& operator=(const ColorMap& colorMap);

   /**
    * Determine if ColorMaps are equal.
    *
    * @param colorMap
    *        The map to compare against
    *
    * @return \c true if the maps are equal, \c false otherwise.
    */
   bool operator==(const ColorMap& colorMap) const;

   /**
    * Get the name of the ColorMap.
    *
    * @return The name of the object.
    */
   const std::string& getName() const;

   /**
    * Save the ColorMap to a file.
    *
    * @param filename
    *        The name of the file to save to.
    *
    * @return \c true if the operation was a success, \c false otherwise.
    */
   bool saveToFile(const std::string& filename) const;

   /**
    * Save the ColorMap to a string buffer.
    *
    * @param buffer
    *        The ColorMap data will be written to this string. The string
    *        will be resized to accomidate the data.
    *
    * @return \c true if the operation was a success, \c false otherwise.
    */
   bool saveToBuffer(std::string& buffer) const;

   /**
    * Get the ColorMap's color table.
    *
    * @return The vector of ColorTypes for the ColorMap.
    */
   const std::vector<ColorType>& getTable() const;

   /**
    * Get a specific color from the map.
    *
    * @param index
    *        The index to retrieve.  This must be within the bounds of the map.
    *
    * @return The associated ColorType.
    */
   const ColorType &operator[](int index) const;

   /**
    * Get a specific color from the map as a non-const reference.
    *
    * This allows for code like map[i] = ColorType(255, 255, 255)
    *
    * @param index
    *        The index to retrieve.  This must be within the bounds of the map.
    *
    * @return The associated ColorType.
    */
   ColorType &operator[](int index);

   /**
    * Clear the table and name, replacing it with a default grayscale.
    */
   void resetToDefault();

   /**
    * Determine if the ColorMap has not been changed from the default.
    *
    * @return \c true if the object is still default, \c false otherwise.
    */
   bool isDefault() const;
   
   /**
    * Determine if the ColorMap contains only fully opaque colors.
    *
    * @return \c true if the object is fully opaque, \c false otherwise.
    */
   bool isFullyOpaque() const;

   /**
    * Returns a pointer to the Gradient object that was used to define the
    * ColorMap, if any.
    *
    * @return A pointer to the Gradient object that was used to define the
    *           ColorMap, or \c NULL if none was used.
    */
   const Gradient *getGradientDefinition() const;

   /**
    * Load ColorMap data from a file.
    *
    * @param filename
    *        The file containing the ColorMap data.
    *
    * @return \c true if successful, \c false otherwise.
    */
   bool loadFromFile(const std::string& filename);

   /**
    * Load ColorMap data from a string buffer.
    *
    * @param buffer
    *        The string containing the ColorMap data.
    *
    * @return \c true if successful, \c false otherwise.
    */
   bool loadFromBuffer(const std::string& buffer);

private:
   static const int VERSION_ONE_TABLE_SIZE = 256;

   bool serialize(QIODevice &io) const;
   bool deserialize(QIODevice &io);
   bool setTable(const std::string& name, const std::vector<ColorType>& table);
   static bool tableIsValid(const std::vector<ColorType>& table);
   std::vector<ColorType> tableFromGradient(const Gradient &gradient);
   bool deserializeGradient(QIODevice &io, int version);
   bool serializeGradient(QIODevice &io) const;

   std::vector<ColorType> mTable;
   std::string mName;
   bool mIsDefault;
   Gradient* mpGradient;
};

#endif
