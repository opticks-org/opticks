/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef VALUERANGE_H
#define VALUERANGE_H

template <class PassAreaType>
class ValueRange
{
public:
   ValueRange(double lower, double upper) : mPassArea(lower, upper) {}
   ValueRange(double limit) : mPassArea(limit) {}
   inline bool contains(double value) const { return mPassArea.contains(value); }
   PassAreaType getPassArea() const { return mPassArea; }
   bool operator==(const ValueRange<PassAreaType>& rhs) { return mPassArea == rhs.mPassArea; }

private:
   PassAreaType mPassArea;
};

namespace PassAreaType
{
   class Below
   {
   public:
      Below(double limit) : mLimit(limit) {}
      inline bool contains(double value) const
      { 
         return value <= mLimit; 
      }
      double getThreshold1() const { return mLimit; }
      bool operator==(const Below& rhs) { return mLimit == rhs.mLimit; }
   private:
      double mLimit;
   };

   class Above
   {
   public:
      Above(double limit) : mLimit(limit) {}
      inline bool contains(double value) const { return value >= mLimit; }
      double getThreshold1() const { return mLimit; }
      bool operator==(const Above& rhs) { return mLimit == rhs.mLimit; }
   private:
      double mLimit;
   };

   class Between
   {
   public:
      Between(double lower, double upper) : mLower(lower), mUpper(upper) {}
      inline bool contains(double value) const { return value >= mLower && value <= mUpper; }
      double getThreshold1() const { return mLower; }
      double getThreshold2() const { return mUpper; }
      bool operator==(const Between& rhs) { return mLower == rhs.mLower && mUpper == rhs.mUpper; }
   private:
      double mLower, mUpper;
   };

   class Outside
   {
   public:
      Outside(double lower, double upper) : mLower(lower), mUpper(upper) {}
      inline bool contains(double value) const { return value <= mLower || value >= mUpper; }
      double getThreshold1() const { return mLower; }
      double getThreshold2() const { return mUpper; }
      bool operator==(const Outside& rhs) { return mLower == rhs.mLower && mUpper == rhs.mUpper; }
   private:
      double mLower, mUpper;
   };
}

#endif

 
