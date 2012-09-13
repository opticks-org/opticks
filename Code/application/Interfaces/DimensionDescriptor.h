/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DIMENSIONDESCRIPTOR_H
#define DIMENSIONDESCRIPTOR_H

#include "AppConfig.h"
#include <string>

/**
 *  Index infomation for a row, column, or band within a data set.
 *
 *  This class is used to maintain information specific for a particular row,
 *  column, or band within a data set.  There will be one instance of this
 *  class for each row, column, and band in the data set.
 *
 *  The object contains three identifying numbers: original, on-disk, and
 *  active as described below.  Each number will have a valid or invalid state
 *  that indicates whether the get methods should be called.
 *
 *  <b>Original Number</b><br>
 *  The original number represents the row, column, or band number that
 *  pertains to the original raw data from the sensor.  This value is set by an
 *  importer inside of Importer::getImportDescriptors() or by an object that
 *  creates a data set as a result of running an algorithm.  All instances of
 *  this object should have a valid original number.
 *
 *  <b>On-Disk Number</b><br>
 *  The on-disk number represents the row, column, or band number as it
 *  corresponds to the total number of rows, columns, or bands in a file on
 *  disk.  This value is set by an importer inside of
 *  Importer::getImportDescriptors().  If the data set for this row, column, or
 *  band was created by an object as a result of running an algorithm or
 *  otherwise not originally imported from a file, the on-disk number will be
 *  invalid.  Each row, column, and band object inside a
 *  RasterFileDescriptor should have a valid on-disk number.
 *
 *  <b>Active Number</b><br>
 *  The active number represents the row, column, or band number that is
 *  available for processing.  This value is set by the core application just
 *  before an importer is executed and after the user has specified an optional
 *  subset, or by an object that creates a data set as a result of running an
 *  algorithm.  If a subset of a data set has been imported, the active number
 *  will be valid only for the rows, columns, and bands that have been
 *  imported.  In this case, all rows, columns, and bands in the
 *  RasterDataDescriptor will have valid active numbers, and only those rows, 
 *  columns, and bands that were imported will have valid active numbers in the 
 *  RasterFileDescriptor.
 *
 *  @see   DataDescriptor, FileDescriptor
 */
class DimensionDescriptor
{
public:
   /**
    * Constructs a DimensionDescriptor that is invalid until one of the set methods
    * is called.
    */
   DimensionDescriptor() :
      mOriginalNumber(0),
      mOnDiskNumber(0),
      mActiveNumber(0),
      mOriginalValid(false),
      mOnDiskValid(false),
      mActiveValid(false)
   {
   }

   /**
    * Assignment operator for DimensionDescriptor.
    *
    * @param descriptor the right hand side of the assignment.
    *
    * @return this object.
    */
   BROKEN_INLINE_HINT DimensionDescriptor& operator =(const DimensionDescriptor& descriptor)
   {
      if (this != &descriptor)
      {
         mOriginalNumber = descriptor.mOriginalNumber;
         mOnDiskNumber = descriptor.mOnDiskNumber;
         mActiveNumber = descriptor.mActiveNumber;
         mOriginalValid = descriptor.mOriginalValid;
         mOnDiskValid = descriptor.mOnDiskValid;
         mActiveValid = descriptor.mActiveValid;
      }

      return *this;
   }

   /**
    * Inequality operator for DimensionDescriptor.
    *
    * @param dd the object being compared to.
    *
    * @return True, if any of the original, on-disk, or
    *         active numbers are not equal. False, otherwise.
    */
   bool operator!=(const DimensionDescriptor& dd) const
   {
      return !(*this == dd);
   }

   /**
    * Equality operator for DimensionDescriptor.
    *
    * @param dd the object being compared to.
    *
    * @return True, if all of the original, on-disk, or
    *         active numbers are equal. False, otherwise.
    */
   bool operator==(const DimensionDescriptor& dd) const
   {
      if (mOriginalValid != dd.mOriginalValid || mOnDiskValid != dd.mOnDiskValid || mActiveValid != dd.mActiveValid)
      {
         return false;
      }

      return ((!mOriginalValid || (mOriginalNumber == dd.mOriginalNumber)) && 
         (!mActiveValid || (mActiveNumber == dd.mActiveNumber)) &&
         (!mOnDiskValid || (mOnDiskNumber == dd.mOnDiskNumber)));
   }

   /**
    * Less than operator for DimensionDescriptor.
    * This method provides a guaranteed ordering of DimensionDescriptors.  This operator
    * should only be used to perform unique sorting of DimensionDescriptors, ie. the
    * kind required to use a DimensionDescriptor as a key of std::map.
    *
    * @param right
    *        the object being compared to.
    *
    * @return True, if this DimensionDescriptor is less than the right Dimension Descriptor.
    *         False, otherwise.
    */
   bool operator<(const DimensionDescriptor& right) const
   {
      if ((mOriginalValid == right.mOriginalValid) && (mOriginalNumber == right.mOriginalNumber))
      {
         if ((mOnDiskValid == right.mOnDiskValid) && (mOnDiskNumber == right.mOnDiskNumber))
         {
            if ((mActiveValid == right.mActiveValid) && (mActiveNumber == right.mActiveNumber))
            {
               return false;
            }
            else
            {
               if ((mActiveValid == true) && (right.mActiveValid == true))
               {
                  return mActiveNumber < right.mActiveNumber;
               }
               else
               {
                  return false;
               }
            }
         }
         else
         {
            if ((mOnDiskValid == true) && (right.mOnDiskValid == true))
            {
               return mOnDiskNumber < right.mOnDiskNumber;
            }
            else
            {
               return false;
            }
         }
      }
      else
      {
         if ((mOriginalValid == true) && (right.mOriginalValid == true))
         {
            return mOriginalNumber < right.mOriginalNumber;
         }
         else
         {
            return false;
         }
      }
   }

   /**
    * Greater than operator for DimensionDescriptor.
    * This method provides a guaranteed ordering of DimensionDescriptors.  This operator
    * should only be used to perform unique sorting of DimensionDescriptors, ie. the
    * kind required to use a DimensionDescriptor as a key of std::map.
    *
    * @param right
    *        the object being compared to.
    *
    * @return True, if this DimensionDescriptor is greater than the right Dimension Descriptor.
    *         False, otherwise.
    */
   bool operator>(const DimensionDescriptor& right) const
   {
      if ((mOriginalValid == right.mOriginalValid) && (mOriginalNumber == right.mOriginalNumber))
      {
         if ((mOnDiskValid == right.mOnDiskValid) && (mOnDiskNumber == right.mOnDiskNumber))
         {
            if ((mActiveValid == right.mActiveValid) && (mActiveNumber == right.mActiveNumber))
            {
               return false;
            }
            else
            {
               if ((mActiveValid == true) && (right.mActiveValid == true))
               {
                  return mActiveNumber > right.mActiveNumber;
               }
               else
               {
                  return false;
               }
            }
         }
         else
         {
            if ((mOnDiskValid == true) && (right.mOnDiskValid == true))
            {
               return mOnDiskNumber > right.mOnDiskNumber;
            }
            else
            {
               return false;
            }
         }
      }
      else
      {
         if ((mOriginalValid == true) && (right.mOriginalValid == true))
         {
            return mOriginalNumber > right.mOriginalNumber;
         }
         else
         {
            return false;
         }
      }
   }

   /**
    *  Sets the original number for the row, column, or band.
    *
    *  The original number indicates the initial index of the row, column, or
    *  band from the dataset generated by the sensor.
    *
    *  The original number is automatically made valid when this method is
    *  called, so there is no need to call setOriginalNumberValid().
    *
    *  @param   originalNumber
    *           The zero-based number of the row, column, or band in the
    *           original data set.
    */
   void setOriginalNumber(unsigned int originalNumber)
   {
      mOriginalNumber = originalNumber;
      mOriginalValid = true;
   }

   /**
    *  Returns the original number of the row, column, or band.
    *
    *  To ensure that the number returned is valid, call the
    *  isOriginalNumberValid() method before calling this method.
    *
    *  @return  The zero-based number of the row, column, or band in the
    *           original data set.
    */
   unsigned int getOriginalNumber() const
   {
      return mOriginalNumber;
   }

   /**
    *  Queries whether the original number of the row, column, or band is valid.
    *
    *  @return  Returns \b true if the original number has been set and is
    *           valid; otherwise returns \b false.
    */
   bool isOriginalNumberValid() const
   {
      return mOriginalValid;
   }

   /**
    *  Sets whether the original number of the row, column, or band is valid.
    *
    *  An invalid number is automatically made valid when setOriginalNumber()
    *  is called.  Therefore, this method is typically called to invalidate
    *  the original number.
    *
    *  @param   valid
    *           The new valid state of the original number.
    */
   void setOriginalNumberValid(bool valid)
   {
      mOriginalValid = valid;
   }

   /**
    *  Sets the on-disk number for the row, column, or band.
    *
    *  The on-disk number indicates the index of the row, column, or band as it
    *  is stored in the file on disk.  The current file may be reduced from the
    *  original sensor data file if a subset has been saved.
    *
    *  The on-disk number is automatically made valid when this method is
    *  called, so there is no need to call setOnDiskNumberValid().
    *
    *  @param   onDiskNumber
    *           The zero-based number of the row, column, or band in the file
    *           on disk.
    */
   void setOnDiskNumber(unsigned int onDiskNumber)
   {
      mOnDiskNumber = onDiskNumber;
      mOnDiskValid = true;
   }

    /**
    *  Returns the on-disk number of the row, column, or band.
    *
    *  To ensure that the number returned is valid, call the
    *  isOnDiskNumberValid() method before calling this method.
    *
    *  @return  The zero-based number of the row, column, or band as it is
    *           stored in the file on disk.
    */
   unsigned int getOnDiskNumber() const
   {
     return mOnDiskNumber;
   }

   /**
    *  Queries whether the on-disk number of the row, column, or band has
    *  been set.
    *
    *  @return  Returns \b true if the on-disk number has been set and is
    *           valid; otherwise returns \b false.
    */
   bool isOnDiskNumberValid() const
   {
      return mOnDiskValid;
   }

   /**
    *  Sets whether the on-disk number of the row, column, or band is valid.
    *
    *  An invalid number is automatically made valid when setOnDiskNumber()
    *  is called.  Therefore, this method is typically called to invalidate
    *  the on-disk number.
    *
    *  @param   valid
    *           The new valid state of the on-disk number.
    */
   void setOnDiskNumberValid(bool valid)
   {
      mOnDiskValid = valid;
   }

   /**
    *  Sets the active number for the row, column, or band.
    *
    *  The active number indicates the index of the row, column, or band as it
    *  has been imported.
    *
    *  The active number is automatically made valid when this method is
    *  called, so there is no need to call setActiveNumberValid().
    *
    *  @param   activeNumber
    *           The zero-based number of the row, column, or band as it has
    *           been imported.
    */
   void setActiveNumber(unsigned int activeNumber)
   {
      mActiveNumber = activeNumber;
      mActiveValid = true;
   }

    /**
    *  Returns the active number of the row, column, or band.
    *
    *  To ensure that the number returned is valid, call the
    *  isActiveNumberValid() method before calling this method.
    *
    *  @return  The zero-based number of the row, column, or band as it has
    *           been imported.
    */
   unsigned int getActiveNumber() const
   {
      return mActiveNumber;
   }

   /**
    *  Queries whether the active number of the row, column, or band has been
    *  set.
    *
    *  @return  Returns \b true if the active number has been set and is valid;
    *           otherwise returns \b false.
    */
   bool isActiveNumberValid() const
   {
      return mActiveValid;
   }

   /**
    *  Sets whether the active number of the row, column, or band is valid.
    *
    *  An invalid number is automatically made valid when setActiveNumber() is
    *  called.  Therefore, this method is typically called to invalidate the
    *  active number.
    *
    *  @param   valid
    *           The new valid state of the active number.
    */
   void setActiveNumberValid(bool valid)
   {
      mActiveValid = valid;
   }

   /**
    * Queries whether this DimensionDescriptor is valid or not.
    *
    * @return Returns \b true if the any number in the DimensionDescriptor is valid;
    *         otherwise returns \b false.
    */
   bool isValid() const
   {
      return mOriginalValid || mOnDiskValid || mActiveValid;
   }

private:
   unsigned int mOriginalNumber;
   unsigned int mOnDiskNumber;
   unsigned int mActiveNumber;

   bool mOriginalValid;
   bool mOnDiskValid;
   bool mActiveValid;
};

#endif
