/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <math.h>
#include <QtWidgets/QApplication>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

class AssertionCounter
{
private:
   static unsigned int smSuccesses;
   static unsigned int smFailures;
public:
   static void initialize();
   static void assertPassed() { smSuccesses++; }
   static void assertFailed() { smFailures++; }
   static unsigned int get() { return smSuccesses + smFailures; }
   static unsigned int getNumPassed() { return smSuccesses; }
   static unsigned int getNumFailed() { return smFailures; }
};

class AssertionFailureLog
{
public:
   static void append(std::string message)
   {
      smLog.push_back(message);
   }
   static void print()
   {
      for (unsigned int i=0; i<smLog.size(); ++i)
      {
         printf(smLog[i].c_str());
      }
   }

private:
   static std::vector<std::string> smLog;
};

static bool assertProc(bool success,
                       const char *expression,
                       const char *filename,
                       int line)
{
   if (!success)
   {
      char buffer[2048];
      sprintf(buffer, "\n***'%s' failed: %s @ #%d\n\n", expression, filename, line);
      printf (buffer);
      AssertionCounter::assertFailed();
      AssertionFailureLog::append(std::string(buffer));
   }
   else AssertionCounter::assertPassed();

   return success;
}

template<class T>
bool assertProcExt1(bool success,
                       const char *pComparitor,
                       const char *pValue1,
                       T value1,
                       const char *pValue2,
                       T value2,
                       const char *filename,
                       int line)
{
   if (!success)
   {
      std::stringstream ss;
      ss << "\n***assertion failed: x " << pComparitor << " y\n";
      ss << "      x: " << pValue1 << " (" << value1 << ")\n";
      ss << "      y: " << pValue2 << " (" << value2 << ")\n";
      ss << "      " << filename << " @ #" << line << std::endl;
      std::cout << ss.str() << "\n";
      AssertionCounter::assertFailed();
      AssertionFailureLog::append(ss.str());
   }
   else 
   {
      AssertionCounter::assertPassed();
   }

   return success;
}


static bool isWithin(double value, double target, double threshold)
{
   if (value == 0 || target == 0) return false;

   if (threshold == 0) return value == target; // no threshold, we're going for exact match here!

   return fabs((value - target) / (value + target)) < threshold;
}

#define tst_assert(x) assertProc(x,#x,__FILE__,__LINE__)

// issea = 'i'f ('s'uccess) 's'uccess and-'e'quals 'a'ssert
#define issea(x) success = success && assertProc(x,#x,__FILE__,__LINE__)

// issea = 'i'f ('s'uccess) 's'uccess and-'e'quals 'a'ssert; if (!success) continue
#define isseac(x) issea(x); if (!success) continue

// issea = 'i'f ('s'uccess) 's'uccess and-'e'quals 'a'ssert; if (!success) break
#define isseab(x) issea(x); if (!success) break

// issea = 'i'f ('s'uccess) 's'uccess and-'e'quals 'a'ssert; if (!success) 'r'eturn retVal;
#define issear(x, retVal) issea(x); if (!success) return retVal

// issea = 'i'f ('s'uccess) 's'uccess and-'e'quals 'a'ssert; 'r'eturn 'f'alse
#define issearf(x) issear(x, false)

class ApproxDouble
{
public:
   ApproxDouble (double value, double poa) : mValue(value), mPoa(poa) {}
   double distanceTo(double value)
   {
      if (mValue == value)
      {
         return 0.0;
      }
      double absLhs = fabs(mValue);
      double absRhs = fabs(value);
      double maxMag = std::max(absLhs, absRhs);
      double magDelta = fabs(mValue - value);
      return log10(maxMag / magDelta);
   }
   bool operator==(const ApproxDouble &rhs)
   {
      if (mValue == rhs.mValue)
      {
         return true;
      }
      double dist = distanceTo(rhs.mValue);
      double poa = std::max(mPoa, rhs.mPoa);
      if (dist < poa)
      {
         std::cout << "Actual POA: " << dist << std::endl;
         return false;
      }
      return true;
   }
   double value() const
   {
      return mValue;
   }
   double poa() const
   {
      return mPoa;
   }
private:
   double mValue;
   double mPoa;
};

inline std::ostream & operator<<(std::ostream &s, const ApproxDouble &ad)
{
   s << "value = " << ad.value() << ", places of accuracy = " << ad.poa();
   return s;
}

#define issea_ext1(x,c,y) success = success && assertProcExt1(x c y, #c, #x, x, #y, y, __FILE__, __LINE__)
#define issear_ext1(x,c,y,retVal) issea_ext1(x, c, y); if (!success) return retVal
#define issearf_ext1(x,c,y) issear(x, c, y, false)

#define issea_lt(x,y) issea_ext1(x, <, y)
#define issea_le(x,y) issea_ext1(x, <=, y)
#define issea_eq(x,y) issea_ext1(x, ==, y)
#define issea_gt(x,y) issea_ext1(x, >, y)
#define issea_ge(x,y) issea_ext1(x, >=, y)
#define issea_ae(poa,x,y) issea_ext1(ApproxDouble(x,poa), ==, ApproxDouble(y,poa))

#define issearf_lt(x,y) issearf_ext1(x, <, y)
#define issearf_le(x,y) issearf_ext1(x, <=, y)
#define issearf_eq(x,y) issearf_ext1(x, ==, y)
#define issearf_gt(x,y) issearf_ext1(x, >, y)
#define issearf_ge(x,y) issearf_ext1(x, >=, y)
#define issearf_ae(poa,x,y) issea_ext1(ApproxDouble(x,poa), ==, ApproxDouble(y,poa)); if (!success) return false

/*
#define assert(x) (x);\
   if (!(x)) \
   { \
      printf("\n'%s' failed: %s @ #%d\n\n", #x, __FILE__, __LINE__); \
   }
*/
#endif
