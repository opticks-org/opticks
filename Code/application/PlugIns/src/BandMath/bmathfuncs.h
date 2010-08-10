/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BMATHFUNCS_H
#define BMATHFUNCS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <vector>

#include "Progress.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"

#define D_TO_R_MULT     0.017453292519943295
#define R_TO_D_MULT     57.295779513082321

#define MAX_OP_LENGTH   5   // set to length of the longest Operator string
#define SEP             '@'

class DivZero {};
class Undefined {};
class Complex {};
class NoData {};

bool ParseIsOp(char* ops, char* val);
int OpParams(char* ops, char* val);
int ParseExp(char* exp, int bands, char* DelimString, int cubes = 0);
char* ValRight(char* exp, int pos);
char* ValLeft(char* exp, int pos);
bool IsOp(char* ops, char* val);
int OpPres(char* ops, char* val);
inline double GRand();
inline double SingleRand();

inline double doubleFromEncoding(EncodingType encoding, void* data, int offset = 0);

class DataNode
{
public:

   DataNode(bool op, char* OS, bool deg)
   {
      degrees = deg; 
      isOperator = op;
      Left = NULL;
      Right = NULL;
      Opera = NULL;

      size_t numBytes = strlen(OS);
      if (numBytes > 0)
      {
         Opera = new char[numBytes + 1];
         strcpy(Opera, OS);
      }
   }

   ~DataNode()
   {
      delete Left;
      delete Right;
      if (Opera != NULL)
      {
         delete[] Opera;
      }
   }

   double eval(std::vector<void*>& data, int band, const std::vector<EncodingType>& types);

   bool degrees;
   bool isOperator;
   char* Opera;
   DataNode* Left;
   DataNode* Right;

private:
   DataNode(const DataNode& node) :
      degrees(node.degrees),
      isOperator(node.isOperator),
      Opera(node.Opera),
      Left(node.Left),
      Right(node.Right)
   {
   }
};


DataNode* BuildTreeFromInfix(char* ops, char* exp, int* offsetTable, int NumElems, bool degrees);

int eval(Progress* pProgress, std::vector<DataAccessor>& dataCubes,
         const std::vector<EncodingType>& types, int rows, int columns,
         int bands, char* exp, DataAccessor returnAccessor, bool degrees,
         char* error, bool cubeMath, bool interactive);

inline double GRand()
{
  return sqrt(-2 * log(SingleRand())) * cos(2.0 * acos(-1.0) * SingleRand());
}

inline double SingleRand()
{
   double value = static_cast<double>(rand() + 1) / RAND_MAX;
   while (value <= 0 || value >= 1)
   {
     value = static_cast<double>(rand() + 1) / RAND_MAX;
   }

   return value;
}

#endif
