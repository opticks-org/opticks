/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VECTOR_H
#define VECTOR_H

#include "FusionException.h"

#include <math.h>

#include <vector>

template<class T>
class Vector : public std::vector<T>
{
   public:
      Vector() : std::vector<T>() {}
      Vector(size_t size) : std::vector<T>(size) {}
      Vector(const Vector& srcVector) : std::vector<T>(srcVector) {}
      ~Vector() {}

      T magnitude() const
      {
         Vector<T>::const_iterator iter;
         T returnValue = (T)0;

         for (iter = begin(); iter != end(); iter++)
         {
            returnValue += ((*iter) * (*iter));
         }
         return (T) sqrt(static_cast<double>(returnValue));
      }

      Vector<T> normalize() const
      {
         T magn;
         Vector<T>::const_iterator iter;
         Vector<T>::iterator returnIter;
         Vector<T> returnVector(size());

         magn = magnitude();

         if (magn > (T)0)
         {
            for (returnIter = returnVector.begin(), iter = begin(); 
                 returnIter != returnVector.end(); 
                 returnIter++, iter++)
            {
               *returnIter = *iter/magn;
            }
         }
         else
         {
            magn = (T)0;
            for (returnIter = returnVector.begin(); returnIter != returnVector.end(); returnIter++)
            {
               *returnIter = magn;
            }
         }

         return returnVector;
      }

      inline const T& operator[](const size_t element) const
      {
         if (element < size())
         {
            return *(begin() + element);
         }
         else
         {
            throw FusionException(std::string("Out of bounds"), __LINE__, __FILE__);
         }
      }

      inline T& operator[](const size_t element)
      {
         if (element < size())
         {
            return *(begin() + element);
         }
         else
         {
            throw FusionException(std::string("Out of bounds"), __LINE__, __FILE__);
         }
      }

      Vector<T> operator+(const Vector<T>& srcVector) const
      {
         Vector<T> returnVector(*this);
         returnVector += srcVector;
         return returnVector;
      }

      Vector<T> operator-(const Vector<T>& srcVector) const
      {
         Vector<T> returnVector(*this);
         returnVector += (srcVector * -1);
         return returnVector;
      }

      Vector<T> operator*(const double scalar) const
      {
         Vector<T> returnVector(*this);
         returnVector *= scalar;
         return returnVector;
      }

      Vector<T>& operator+=(const Vector<T>& srcVector)
      {
         Vector<T>::iterator iter;
         Vector<T>::const_iterator srcIter;

         // size mismatch error
         if (srcVector.size() != size())
         {
            throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
         }

         for (iter = begin(), srcIter = srcVector.begin();
            iter != end() && srcIter != srcVector.end();
            iter++, srcIter++)
         {
            *iter += *srcIter;
         }

         return *this;
      }

      Vector<T>& operator-=(const Vector<T>& vect)
      {
         (*this) += (vect * -1);
         return *this;
      }

      Vector<T>& operator*=(const T scalar)
      {
         Vector<T>::iterator iter;

         //std::cout<<"Vec Size: "<<(int)size( )<<std::endl;
         for (iter = begin(); iter != end( ); iter++)
         {
            *iter *= scalar;
         }

         return *this;
      }

      inline bool operator==(const Vector<T>& rhs)
      {
         return !(*this != rhs);
      }

      bool operator!=(const Vector<T>& rhs)
      {
         Vector<T>::const_iterator iter;
         Vector<T>::const_iterator rhsIter;

         if (size() != rhs.size())
         {
            throw FusionException(std::string("Size mismatch"), __LINE__, __FILE__);
         }

         for (iter = begin(), rhsIter = rhs.begin();
              iter != end() && rhsIter != rhs.end();
              iter++, rhsIter++)
         {
            if (*iter != *rhsIter)
            {
               return true;
            }
         }

         return false;
      }
};

#endif


