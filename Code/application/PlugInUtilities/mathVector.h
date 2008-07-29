/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include <vector>
#include <algorithm>
#include <functional>
#include <ostream>

template<class T>
class mathVector : public std::vector<T>
{
public:
   mathVector(int size) : std::vector<T>(size) {}
   mathVector() : std::vector<T>() {}

   int min(int size1, int size2) { return size1<size2?size1:size2; }

   template<class S>
   mathVector& operator+=(const std::vector<S>& rhs)
   {
      int count = min(size(), rhs.size());
      std::transform(begin(), begin()+count, rhs.begin(), begin(), std::plus<T>());
      return *this;
   }
   template<class S>
   mathVector& operator-=(const std::vector<S>& rhs)
   {
      int count = min(size(), rhs.size());
      std::transform(begin(), begin()+count, rhs.begin(), begin(), std::minus<T>());
      return *this;
   }
   template<class S>
   mathVector& operator*=(const std::vector<S>& rhs)
   {
      int count = min(size(), rhs.size());
      std::transform(begin(), begin()+count, rhs.begin(), begin(), std::multiplies<T>());
      return *this;
   }
   template<class S>
   mathVector& operator/=(const std::vector<S>& rhs)
   {
      int count = min(size(), rhs.size());
      std::transform(begin(), begin()+count, rhs.begin(), begin(), std::divides<T>());
      return *this;
   }

   mathVector& operator+=(const T& rhs)
   {
      std::transform(begin(), end(), begin(), std::bind2nd(std::plus<T>(), rhs));
      return *this;
   }
   mathVector& operator-=(const T& rhs)
   {
      std::transform(begin(), end(), begin(), std::bind2nd(std::minus<T>(), rhs));
      return *this;
   }
   mathVector& operator*=(const T& rhs)
   {
      std::transform(begin(), end(), begin(), std::bind2nd(std::multiplies<T>(), rhs));
      return *this;
   }
   mathVector& operator/=(const T& rhs)
   {
      std::transform(begin(), end(), begin(), std::bind2nd(std::divides<T>(), rhs));
      return *this;
   }
};

template<class T>
std::ostream &operator<<(std::ostream& stream, mathVector<T> &vec)
{
   std::vector<T>::const_iterator it;
   for(it=vec.begin(); it!=vec.end();++it)
   {
      if(it!=vec.begin())
      {
         stream<< string(", ");
      }
      stream << *it;
   }

   return stream;
}

template<class T>
double absSquared(const mathVector<T>& vec)
{
   T result = 0;
   std::vector<T>::iterator it;
   for(it=vec.begin(); it!= vec.end(); ++it)
   {
      double value = *it;
      result += value * value;
   }

   return result;
}

template<class T>
double abs(const mathVector<T>& vec)
{
   return sqrt(absSquared(vec));
}

#endif   // MATH_VECTOR_H

 
