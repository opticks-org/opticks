/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "BandMath.h"
#include "mbox.h"
#include "RasterUtilities.h"

using namespace std;

int ParseExp(char* exp, int bands, char* DelimString, int cubes)
{
   //return if no string was passed in
   if (!exp || exp[0] == 0)
   {
      return -1;
   }

   //check for and/or fix any invalid chars
   int i;
   for (i = 0; exp[i]; i++)
   {
      //make any uppercase chars lower
      if (isalpha(exp[i]) && isupper(exp[i]))
      {
         exp[i] = tolower(exp[i]);
      }


      if ((exp[i] == '[') || (exp[i] == '{'))
      {
         exp[i] = '(';
      }

      if ((exp[i] == ']') || (exp[i] == '}'))
      {
         exp[i] = ')';
      }

      if (exp[i] == 246)  // normal divide symbol
      {
         exp[i] = '/';
      }

      //check string for any invalid chars
      if (exp[i])
      {
         if ((exp[i] != 32) &&
            ((exp[i] < 40) && (exp[i] > 43)) &&
            ((exp[i] < 45) && (exp[i] > 57)) &&
            ((exp[i] < 65) && (exp[i] > 90)) &&
            (exp[i] != 94) &&
            ((exp[i] < 97) && (exp[i] > 122)) )
         {
            sprintf(DelimString, "%c Is An Invalid Character", exp[i]);
            return -1;
         }
      }
   }

   char ops[] = "( 1 ) 1 + 2 - 2 * 2 / 2 ^ 2 sqrt 1 sin 1 cos 1 tan 1 log 1 log10 1 log2 1 exp 1 abs 1 asin 1 "
      "acos 1 atan 1 sinh 1 cosh 1 tanh 1 sec 1 csc 1 cot 1 asec 1 acsc 1 acot 1 sech 1 csch 1 coth 1 ";

   int strLength = strlen(exp); // set strLength to the Length of the input string

   int iOffsetPos = 0;                    // integer pointer into the offset array
   int* piOffset = new int[strLength*2];  // the Operand offset array

   for (i = 0; i < strLength*2; i++)
   {
      piOffset[i] = -1;
   }

   // fill piOffset with operator locations
   for (i = 0; i < strLength; i++)
   {
      char strBlock[MAX_OP_LENGTH+1];
      int read = MAX_OP_LENGTH;
      if ((i+MAX_OP_LENGTH) > strLength)
      {
         read = strLength - i;
      }

      strBlock[read] = 0;

      memcpy(strBlock, &exp[i], read);

      for (int j = MAX_OP_LENGTH; j; j--)
      {
         if ((i+j) <= strLength)
         {
            strBlock[j] = 0;
            if (ParseIsOp(ops, strBlock))
            {
               piOffset[iOffsetPos] = i;
               iOffsetPos++;
               piOffset[iOffsetPos] = j;
               iOffsetPos++;
               i += j-1;
               break;
            }
         }
      }
   }

   int iStringPos = 0;
   int op = 0;



   //Fill output array with delimited string
   for (i = 0; i < strLength; i++)
   {
      //    if(exp[i] == 32)
      if (isspace(exp[i]))
      {
         continue;
      }

      if (i == piOffset[op*2])
      {
         if (i && DelimString[iStringPos-1] != '@')
         {
            DelimString[iStringPos] = '@';
            iStringPos++;
         }
         memcpy(&DelimString[iStringPos], &exp[i], piOffset[op*2+1]);
         iStringPos += piOffset[op*2+1];
         DelimString[iStringPos] = '@';
         iStringPos++;

         i += piOffset[op*2+1]-1;
         op++;
      }
      else
      {
         DelimString[iStringPos] = exp[i];
         iStringPos++;
      }
   }

   DelimString[iStringPos] = '@';
   DelimString[iStringPos+1] = '?';
   DelimString[iStringPos+2] = 0;

   if (piOffset)
   {
      delete [] piOffset;
   }

   //check for formatting errors
   for (i = 0; DelimString[i] != '?'; i++)
   {
      if (DelimString[i] == '@')
      {
         DelimString[i] = 0;
      }
   }

   //check that all () are matched
   ////////////////////////////////////////////////////////////////////////
   int iCount = 0;
   iStringPos = 0;
   int parenCount = 0;
   while (DelimString[iStringPos] != '?')
   {

      char* pValToLeft = ValLeft(DelimString, iCount);
      char* pValToRight = ValRight(DelimString, iCount);

      if (DelimString[iStringPos] == '(')
      {
         parenCount++;

         //check that '()' contains a value
         if (pValToRight && *pValToRight == ')')
         {
            strcpy(DelimString, "Empty Parentheses Set In Expression");
            return -1;
         }
         if (pValToLeft && !ParseIsOp(ops, pValToLeft))
         {
            strcpy(DelimString, "Left Parentheses Must Have An Operator To The Left");
            return -1;
         }

      }
      else if (DelimString[iStringPos] == ')')
      {
         if (pValToRight && !ParseIsOp(ops, pValToRight))
         {
            strcpy(DelimString, "Right Parentheses Must Have An Operator To The Right");
            return -1;
         }

         parenCount--;
      }



      while (DelimString[iStringPos])
      {
         iStringPos++;
      }
      iStringPos++;
      iCount++;

   }

   if (parenCount > 0)
   {
      strcpy(DelimString, "Missing \")\" In Expression.");
      return -1;
   }
   else if (parenCount < 0)
   {
      strcpy(DelimString, "Missing \"(\" In Expression.");
      return -1;
   }


   ////////////////////////////////////////////////////////////////////////////



   iCount = 0;
   iStringPos = 0;
   while (DelimString[iStringPos] != '?')
   {


      if ((DelimString[iStringPos] != '(') && (DelimString[iStringPos] != ')'))
      {
         //check for unary and binary to have correct number of operands
         ////////////////////////////////////////////////////////////////////////

         if (ParseIsOp(ops, &DelimString[iStringPos]))
         {
            //check binary op
            if (OpParams(ops, &DelimString[iStringPos]) == 2)
            {

               char* pValToRight = ValRight(DelimString, iCount);
               char* pValToLeft = ValLeft(DelimString, iCount);


               //check to see if op is negation rather then subtraction
               if ((!pValToLeft) || ((pValToLeft) && (ParseIsOp(ops, pValToLeft)) && (*pValToLeft != ')')))
               {
                  DelimString[iStringPos+1] = '-';
                  DelimString[iStringPos] = 0;
                  iStringPos++;
                  continue;
               }

               if (!pValToLeft)
               {
                  char op[10];
                  strcpy(op, &DelimString[iStringPos]);
                  sprintf(DelimString, "Operator \"%s\" Without Left Operand", op);
                  return -1;
               }
               if (!pValToRight)
               {
                  char op[10];
                  strcpy(op, &DelimString[iStringPos]);
                  sprintf(DelimString, "Operator \"%s\" Without Right Operand", op);
                  return -1;
               }

               if ((*pValToLeft != ')') && (ParseIsOp(ops, pValToLeft)))
               {
                  char op1[10];
                  char op2[10];

                  strcpy(op1, &DelimString[iStringPos]);
                  strcpy(op2, ValLeft(DelimString, iCount));

                  sprintf(DelimString, "Operator \"%s\" Can Not Have \"%s\" As Left Operand", op1, op2);
                  return -1;
               }

               if (((ParseIsOp(ops, pValToRight)) && (OpParams(ops, pValToRight) != 1)) || *pValToRight == ')')
               {
                  char op1[10];
                  char op2[10];

                  strcpy(op1, &DelimString[iStringPos]);
                  strcpy(op2, ValRight(DelimString, iCount));

                  if (*op2 != '-')
                  {
                     sprintf(DelimString, "Operator \"%s\" Can Not Have \"%s\" As Right Operand", op1, op2);
                     return -1;
                  }
               }


            }
            //check unary op
            else
            {
               char* pValToRight = ValRight(DelimString, iCount);
               char* pValToLeft = ValLeft(DelimString, iCount);

               if ((!pValToRight) || (*pValToRight != '('))
               {
                  char op[10];
                  strcpy(op, &DelimString[iStringPos]);
                  sprintf(DelimString, "Operator \"%s\" Without Operand", op);
                  return -1;
               }

               if ((pValToLeft) && !ParseIsOp(ops, pValToLeft))
               {
                  char op1[10];
                  strcpy(op1, &DelimString[iStringPos]);
                  sprintf(DelimString, "Operator \"%s\" Can Only Have Operators To the Left", op1);
                  return -1;
               }


            }

         }

         //check for operands to be a number, cube  or a band
         ////////////////////////////////////////////////////////////////////////

         else
         {
            if ((DelimString[iStringPos] == 'b') || (DelimString[iStringPos] == 'B'))
            {
               for (int k = 1; DelimString[iStringPos+k]; k++)
               {
                  if ((DelimString[iStringPos+k] < 47) || (DelimString[iStringPos+k] > 57))
                  {
                     if (strlen(&DelimString[iStringPos]) < 60)
                     {
                        char op[80];
                        strcpy(op, &DelimString[iStringPos]);
                        sprintf(DelimString, "Operand \"%s\" Is Invalid", op);
                        return -1;
                     }
                     else
                     {
                        sprintf(DelimString, "Invalid Operand");
                        return -1;
                     }
                  }
               }


               if (atoi(&DelimString[iStringPos+1]) > bands)
               {
                  if (strlen(&DelimString[iStringPos]) < 60)
                  {
                     char op[80];
                     strcpy(op, &DelimString[iStringPos]);
                     sprintf(DelimString, "Operand \"%s\" Is Too Large, The Cube Only Has %d Bands", op, bands);
                     return -1;
                  }
                  else
                  {
                     sprintf(DelimString, "Invalid Operand");
                     return -1;
                  }
               }


            }

            else if ((DelimString[iStringPos] == 'c') || (DelimString[iStringPos] == 'C'))
            {
               for (int k = 1; DelimString[iStringPos+k]; k++)
               {
                  if ((DelimString[iStringPos+k] < 47) || (DelimString[iStringPos+k] > 57))
                  {
                     if (strlen(&DelimString[iStringPos]) < 60)
                     {
                        char op[80];
                        strcpy(op, &DelimString[iStringPos]);
                        sprintf(DelimString, "Operand \"%s\" Is Invalid", op);
                        return -1;
                     }
                     else
                     {
                        sprintf(DelimString, "Invalid Operand");
                        return -1;
                     }
                  }
               }


               if (atoi(&DelimString[iStringPos+1]) > cubes)
               {
                  if (strlen(&DelimString[iStringPos]) < 60)
                  {
                     char op[80];
                     strcpy(op, &DelimString[iStringPos]);
                     sprintf(DelimString, "Operand \"%s\" Is Too Large, There are only %d Cubes", op, cubes);
                     return -1;
                  }
                  else
                  {
                     sprintf(DelimString, "Invalid Operand");
                     return -1;
                  }
               }
            }

            else
            {
               bool hasDec = false;

               if (strcmp(&DelimString[iStringPos], "pi") && strcmp(&DelimString[iStringPos], "e"))
               {
                  for (int k = 0; DelimString[iStringPos+k]; k++)
                  {
                     if (k && DelimString[iStringPos+k] == '-')
                     {
                        char op[80];
                        strcpy(op, &DelimString[iStringPos]);
                        sprintf(DelimString, "Operand \"%s\" Is An Invalid Operand", op);
                        return -1;
                     }

                     if (DelimString[iStringPos+k] == '.')
                     {
                        if (!hasDec)
                        {
                           hasDec = true;
                        }
                        else
                        {
                           if (strlen(&DelimString[iStringPos]) < 60)
                           {
                              char op[80];
                              strcpy(op, &DelimString[iStringPos]);
                              sprintf(DelimString, "Operand \"%s\" Has More Then One Decimal Point", op);
                              return -1;
                           }
                           else
                           {
                              sprintf(DelimString, "Invalid Operand");
                              return -1;
                           }
                        }
                     }

                     if ((DelimString[iStringPos+k] < 45) || (DelimString[iStringPos+k] > 57))
                     {
                        if (strlen(&DelimString[iStringPos]) < 60)
                        {
                           char op[80];
                           strcpy(op, &DelimString[iStringPos]);
                           sprintf(DelimString, "Operand \"%s\" Is Invalid", op);
                           return -1;
                        }
                        else
                        {
                           sprintf(DelimString, "Invalid Operand");
                           return -1;
                        }
                     }
                  }
               }
            }
         }
      }

      while (DelimString[iStringPos])
      {
         iStringPos++;
      }
      while (!DelimString[iStringPos])
      {
         iStringPos++;
      }
      iCount++;

   }
   ////////////////////////////////////////////////////////////////////////////

   for (i = 0; DelimString[i] != '?'; i++)
   {
      if (DelimString[i] == 0)
      {
         DelimString[i] = '@';
      }
   }

   DelimString[i] = 0;
   if (DelimString[i-1] == '@')
   {
      DelimString[i-1] = 0;
      if (DelimString[i-2] == '@')
      {
         DelimString[i-2] = 0;
      }
   }


   return 0;
}


// Returns the Value to the right of pos
// the exp string must be NULL delimited
// with a '?' marking the end of the string
// returns NULL pos is the last param
char* ValRight(char* exp, int pos)
{

   // count NULL blocks one past pos
   int strPos = 0;
   for (int i = 0; i <= pos; strPos++)
   {
      //increment i if NULL fund
      if (exp[strPos] == 0)
      {
         i++;

         //skip extra NULL's
         while (exp[strPos+1] == 0)
            strPos++;
      }
   }

   //if value to the right is '?' the end of the string was reached
   if (exp[strPos] == '?')
   {
      return NULL;
   }

   return &exp[strPos];
}

// Returns the Value to the left of pos
// the exp string must be NULL delimited
// with a '?' marking the end of the string
// returns NULL pos is the first param
char* ValLeft(char* exp, int pos)
{

   // if first param return NULL
   if (!pos)
   {
      return NULL;
   }

   // count NULL blocks one short of pos
   int strPos = 0;
   for (int i = 1; i < pos; strPos++)
   {
      //increment i if NULL fund
      if (exp[strPos] == 0)
      {
         i++;

         //skip extra NULL's
         while (exp[strPos+1] == 0)
            strPos++;

      }
   }

   return &exp[strPos];
}

bool ParseIsOp(char* ops, char* val)
{
   //check special '-' case
   if ((strlen(val) == 1) && (*val == '-'))
   {
      return true;
   }

   //check to see if val is a number
   bool isNumber = true;
   int i;
   for (i = 0; val[i]; i++)
   {
      if (((val[i] < 45) || (val[i] > 57)) || (val[i] == 47))
      {
         isNumber = false;
      }
   }

   //return false if val is a number
   if (isNumber)
   {
      return false;
   }

   // use strtok to search for val
   bool found = false;
   char* token = strtok(ops, " ");
   while (token)
   {
      //set found to true if the op was located
      if (!strcmp(token, val))
      {
         found = true;
      }
      token = strtok(NULL, " ");
   }

   //return ops string to normal
   for (i = 0; ops[i] || ops[i+1]; i++)
   {
      if (!ops[i])
      {
         ops[i] = ' ';
      }
   }
   ops[i] = ' ';

   return found;
}



DataNode* BuildTreeFromInfix(char* ops, char* exp, int* offsetTable, int NumElems,bool degrees)
{
   DataNode* retVal = NULL;

   int i;
   for (i = 0; i < NumElems; i++)
   {

      if (exp[offsetTable[i]] == '(')
      {

         DataNode* cur = retVal;

         if (!retVal)
         {
            cur = new DataNode(true, &exp[offsetTable[i]], degrees);
            retVal = cur;
         }
         else
         {
            while (cur->Right)
            {
               cur = cur->Right;
            }
            cur->Right = new DataNode(true, &exp[offsetTable[i]], degrees);
            cur = cur->Right;
         }

         i++;
         int start = i;

         // find closing paren
         for (int iPCount = 1; iPCount; i++)
         {
            if (exp[offsetTable[i]] == '(')
            {
               iPCount++;
            }
            else if (exp[offsetTable[i]] == ')')
            {
               iPCount--;
            }
         }
         i--;

         int stop = i;

         int littleElems = stop-start;
         int* pLittleTable = new int[littleElems];

         for (int j = 0; j < littleElems; j++)
         {
            pLittleTable[j] = offsetTable[j+start];
         }

         cur->Right = BuildTreeFromInfix(ops, exp, pLittleTable, littleElems, degrees);

         delete [] pLittleTable;

      }

      else
      {
         //current value is an operand
         if (!IsOp(ops, &exp[offsetTable[i]]))
         {
            if (!retVal)
            {
               retVal = new DataNode(false, &exp[offsetTable[i]], degrees);
            }
            else if (!retVal->Right)
            {
               retVal->Right = new DataNode(false, &exp[offsetTable[i]], degrees);
            }
            else
            {
               DataNode* cur = retVal;
               while (cur->Right)
               {
                  cur = cur->Right;
               }

               cur->Right = new DataNode(false, &exp[offsetTable[i]], degrees);
            }
         }
         //current value is an operator
         else
         {
            //operator is unary
            if (OpParams(ops, &exp[offsetTable[i]]) == 1)
            {

               DataNode* cur = retVal;

               if (!retVal)
               {
                  cur = new DataNode(true, &exp[offsetTable[i]], degrees);
                  retVal = cur;
               }
               else
               {
                  while (cur->Right)
                  {
                     cur = cur->Right;
                  }

                  cur->Right = new DataNode(true, &exp[offsetTable[i]], degrees);

                  cur = cur->Right;
               }


               i = i + 2;
               int start = i;

               // find closing paren
               for (int iPCount = 1; iPCount; i++)
               {
                  if (exp[offsetTable[i]] == '(')
                  {
                     iPCount++;
                  }
                  else if (exp[offsetTable[i]] == ')')
                  {
                     iPCount--;
                  }
               }
               i--;

               int stop = i;

               int littleElems = stop-start;
               int* pLittleTable = new int[littleElems];

               for (int j = 0; j < littleElems; j++)
               {
                  pLittleTable[j] = offsetTable[j+start];
               }

               cur->Right = BuildTreeFromInfix(ops, exp, pLittleTable, littleElems, degrees);

               delete [] pLittleTable;

            }
            //head is not an operator
            else if (!retVal->isOperator)
            {
               DataNode* newnode = new DataNode(true, &exp[offsetTable[i]], degrees);
               newnode->Left = retVal;
               retVal = newnode;
            }
            //head is an operator
            else if (retVal->isOperator)
            {
               DataNode* cur = retVal;

               // head node has higher order then new node
               if (OpPres(ops, &exp[offsetTable[i]]) <= OpPres(ops, cur->Opera))
               {
                  DataNode* newnode = new DataNode(true, &exp[offsetTable[i]], degrees);
                  newnode->Left = retVal;
                  retVal = newnode;
               }
               else
               {
                  // find point where node to the right is either highr order or is a operand
                  while ((cur->Right->isOperator) &&
                     (OpPres(ops, &exp[offsetTable[i]]) > OpPres(ops, cur->Right->Opera)))
                  {
                     cur = cur->Right;
                  }

                  DataNode* newnode = new DataNode(true, &exp[offsetTable[i]], degrees);
                  newnode->Left = cur->Right;
                  cur->Right = newnode;
               }
            }
         }
      }
   }
   return retVal;
}

bool IsOp(char* ops, char* val)
{
   //check for (
   if (*val == '(')
   {
      return true;
   }

   //check special '-' case
   if ((strlen(val) == 1) && (*val == '-'))
   {
      return true;
   }

   //check to see if val is a number
   bool isNumber = true;
   int i;
   for (i = 0; val[i]; i++)
   {
      if (((val[i] < 45) || (val[i] > 57)) || (val[i] == 47))
      {
         isNumber = false;
      }
   }

   if (isNumber)
   {
      return false;
   }

   bool found = false;

   char* token = strtok(ops, " ");
   while (token)
   {
      if (!strcmp(token, val))
      {
         found = true;
      }
      token = strtok(NULL, " ");
   }

   //return ops string to normal
   for (i = 0; ops[i] || ops[i+1]; i++)
   {
      if (!ops[i])
      {
         ops[i] = ' ';
      }
   }
   ops[i] = ' ';

   return found;
}

int OpPres(char* ops, char* val)
{
   if (*val == '(')
   {
      return 1000;
   }

   int retval = 0;

   char* token = strtok(ops, " ");
   while (token)
   {
      if (!strcmp(token, val))
      {
         token = strtok(NULL, " ");
         token = strtok(NULL, " ");
         retval = atoi(token);
      }
      token = strtok(NULL, " ");
   }

   //return ops string to normal
   int i;
   for (i = 0; ops[i] || ops[i+1]; i++)
   {
      if (!ops[i])
      {
         ops[i] = ' ';
      }
   }
   ops[i] = ' ';

   return retval;
}

int OpParams(char* ops, char* val)
{
   if (*val == '(')
   {
      return 1;
   }

   int retval = 0;

   char* token = strtok(ops, " ");
   while (token)
   {
      if (!strcmp(token, val))
      {
         token = strtok(NULL, " ");
         retval = atoi(token);
      }
      token = strtok(NULL, " ");
   }

   //return ops string to normal
   int i;
   for (i = 0; ops[i] || ops[i+1]; i++)
   {
      if (!ops[i])
      {
         ops[i] = ' ';
      }
   }
   ops[i] = ' ';

   return retval;
}

int eval(Progress* pProgress, vector<DataAccessor>& dataCubes, const vector<EncodingType>& types,
         int rows, int columns, int bands, char* exp, DataAccessor returnAccessor, bool degrees, char* error,
         bool cubeMath, bool interactive)
{
   int stringSize = strlen(exp)*2;
   if (stringSize < 80)
   {
      stringSize = 80;
   }

   char* pString = new char[stringSize];

   int iError = ParseExp(exp, bands, pString, dataCubes.size());
   if (iError)
   {
      strcpy(error, pString);
      return -1;
   }

   bool lastCharSep = false;
   int itemsCount = 1;
   int i;
   for (i = 0; pString[i]; i++)
   {
      if (pString[i] == SEP)
      {
         if (lastCharSep == false)
         {
            itemsCount++;
         }

         lastCharSep = true;
      }
      else
      {
         lastCharSep = false;
      }
   }

   int* pItems = new int[itemsCount];
   pItems[0] = 0;
   int pos = 1;

   for (i = 0; pString[i]; i++)
   {
      if (pString[i] == SEP)
      {
         //Skip multiple SEP's
         while (pString[i+1] == SEP)
         {
            pString[i] = 0;
            i++;
         }

         pItems[pos] = i+1;
         pos++;
         pString[i] = 0;
      }
   }

   char ops[] = "+ 2 1 - 2 1 * 2 2 / 2 2 ^ 2 3 sqrt 1 9 sin 1 9 cos 1 9 tan 1 9 log 1 9 log10 1 9 "
      "log2 1 9 exp 1 9 abs 1 9 asin 1 9 acos 1 9 atan 1 9 sinh 1 9 cosh 1 9 tanh 1 9 sec 1 9 csc 1 9 "
      "cot 1 9 asec 1 9 acsc 1 9 acot 1 9 sech 1 9 csch 1 9 coth 1 9 rand 1 9 ";
   DataNode* pTree = BuildTreeFromInfix(ops, pString, pItems, itemsCount, degrees);
   delete[] pItems;
   delete[] pString;
   pItems = NULL;
   pString = NULL;

   bool dispDZMes = true;
   bool dispUDMes = true;
   bool dispCMMes = true;

   int j;

   srand(time(NULL));

   int bandCount = 1;
   if (cubeMath)
   {
      bandCount = bands;
   }

   float* pReturnValue = NULL;
   void* pCubeValue = NULL;
   vector<void*> cubeValues(dataCubes.size());

   for (i = 0; i < rows; i++)
   {
      for (j = 0; j < columns; j++)
      {
         pReturnValue = reinterpret_cast<float*>(returnAccessor->getColumn());
         for (unsigned int cubeNum = 0; cubeNum < dataCubes.size(); ++cubeNum)
         {
            cubeValues[cubeNum] = dataCubes[cubeNum]->getColumn();
         }

         for (int bandNum = 0; bandNum < bandCount; ++bandNum)
         {
            try
            {
               float newValue = static_cast<float>(pTree->eval(cubeValues, bandNum, types));
               if (RasterUtilities::isBad(newValue))
               {
                  throw out_of_range("The value is out of range");
               }

               pReturnValue[bandNum] = newValue;
            }
            catch (DivZero)
            {
               if (interactive == true)
               {
                  if (dispDZMes)
                  {
                     MBox mb("Warning", "Warning bandmathfuncs003: Divide By Zero\nSelect 'OK' to continue, \n"
                        "all bad values will be set to 0.  \nOr 'Cancel' to cancel the operation.",
                        MB_OK_CANCEL_ALWAYS, NULL);

                     if (mb.exec() == QDialog::Rejected)
                     {
                        return -2;
                     }
                     else if (mb.cbAlways->isChecked())
                     {
                        dispDZMes = false;
                     }
                  }
               }
               else
               {
                  if (dispDZMes)
                  {
                     if (pProgress != NULL)
                     {
                        pProgress->updateProgress("The band math operation attempted to divide by zero. "
                                                  "Operation will contine and bad values will be set to 0.",
                                                  100 * i / rows, WARNING);
                     }
                     dispDZMes = false;
                  }
               }

               memset(pReturnValue, 0, bandCount * sizeof(float)); // clear the point
            }
            catch (Undefined)
            {
               if (interactive == true)
               {
                  if (dispUDMes)
                  {
                     MBox mb("Warning", "Warning bandmathfuncs001: Undefined Value\n"
                        "Select 'OK' to continue, \nall bad values will be set to 0.  \n"
                        "Or 'Cancel' to cancel the operation.",
                        MB_OK_CANCEL_ALWAYS, NULL);

                     if (mb.exec() == QDialog::Rejected)
                     {
                        return -2;
                     }
                     else if (mb.cbAlways->isChecked())
                     {
                        dispUDMes = false;
                     }
                  }
               }
               else
               {
                  strcpy(error, "The band math operation encountered an undefined value.");
                  return -1;
               }

               memset(pReturnValue, 0, bandCount * sizeof(float)); // clear the point
            }
            catch (Complex)
            {
               if (interactive == true)
               {
                  if (dispCMMes)
                  {
                     MBox mb("Warning", "Warning bandmathfuncs002: Math Operation Resulted in a Complex Number\n"
                        "Select 'OK' to continue, \nall bad values will be set to 0.\n"
                        "Or 'Cancel' to cancel the operation.",
                        MB_OK_CANCEL_ALWAYS, NULL);

                     if (mb.exec() == QDialog::Rejected)
                     {
                        return -2;
                     }
                     else if (mb.cbAlways->isChecked())
                     {
                        dispCMMes = false;
                     }
                  }
               }
               else
               {
                  strcpy(error, "The band math operation resulted in an invalid complex number.");
                  return -1;
               }

               memset(pReturnValue, 0, bandCount * sizeof(float)); // clear the point
            }

            catch (NoData)
            {
               strcpy(error, "The band math operation could not be perfomed because the data is not available.");
               return -1;
            }

            catch (...)
            {
               strcpy(error, "The band math operation resulted in a floating point error.");
               return -1;
            }
         }
         returnAccessor->nextColumn();
         for (unsigned int cubeNum = 0; cubeNum < dataCubes.size(); ++cubeNum)
         {
            dataCubes[cubeNum]->nextColumn();
         }
      }
      returnAccessor->nextRow();
      for (unsigned int cubeNum = 0; cubeNum < dataCubes.size(); ++cubeNum)
      {
         dataCubes[cubeNum]->nextRow();
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Band Math", 100 * i / rows, NORMAL);
      }
   }

   return 0;
}

double doubleFromEncoding(EncodingType encoding, void* data, int offset)
{
   if (data == NULL)
   {
      return 0.0;
   }

   switch (encoding)
   {
   case INT1SBYTE:
      return reinterpret_cast<signed char*>(data)[offset];
   case INT1UBYTE:
      return reinterpret_cast<unsigned char*>(data)[offset];
   case INT2SBYTES:
      return reinterpret_cast<signed short*>(data)[offset];
   case INT2UBYTES:
      return reinterpret_cast<unsigned short*>(data)[offset];
   case INT4SBYTES:
      return reinterpret_cast<signed int*>(data)[offset];
   case INT4UBYTES:
      return reinterpret_cast<unsigned int*>(data)[offset];
   case FLT4BYTES:
      return reinterpret_cast<float*>(data)[offset];
   case FLT8BYTES:
      return reinterpret_cast<double*>(data)[offset];
   default:
      break;
   }

   return 0.0;
}

double DataNode::eval(vector<void*>& data, int band, const vector<EncodingType>& types)
{
   if (data.empty() || data.size() != types.size() || Opera == NULL)
   {
      return 0.0;
   }

   if (!isOperator)
   {
      if (!strcmp(Opera, "pi") || !strcmp(Opera, "PI") || !strcmp(Opera, "Pi"))
      {
         return PI;
      }

      if (!strcmp(Opera, "e") || !strcmp(Opera, "E"))
      {
         return exp(1.0);
      }

      if ((Opera[0] == 'b') || (Opera[0] == 'B'))
      {
         int bandOperation = atoi(&Opera[1]) - 1;
         return doubleFromEncoding(types[0], data[0], bandOperation);
      }

      if ((Opera[0] == 'c') || (Opera[0] == 'C'))
      {
         int cube = atoi(&Opera[1]) - 1;

         if (data.size() > static_cast<unsigned int>(cube))
         {
            return doubleFromEncoding(types[cube], data[cube], band);
         }
         return 0.0;
      }
      return atof(Opera);
   }
   if (!strcmp(Opera, "("))
   {
      return Right->eval(data, band, types);
   }
   if (!strcmp(Opera, "+"))
   {
      return Left->eval(data, band, types) + Right->eval(data, band, types);
   }
   if (!strcmp(Opera, "-"))
   {
      return Left->eval(data, band, types) - Right->eval(data, band, types);
   }
   if (!strcmp(Opera, "*"))
   {
      return Left->eval(data, band, types) * Right->eval(data, band, types);
   }
   if (!strcmp(Opera, "/"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight == 0)
      {
         throw DivZero();
      }

      return Left->eval(data, band, types) / resRight;
   }
   if (!strcmp(Opera, "^"))
   {
      double resLeft = Left->eval(data, band, types);
      double resRight = Right->eval(data, band, types);

      if (resLeft == 0 && resRight <= 0)
      {
         throw DivZero();
      }
      if (resLeft < 0)
      {
         double inter;
         double frac = modf(resRight, &inter);
         if (frac)
         {
            throw Complex();
         }
      }
      return pow(resLeft, resRight);
   }
   if (!strcmp(Opera, "sqrt"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight <= 0)
      {
         throw Complex();
      }
      return sqrt(resRight);
   }
   if (!strcmp(Opera, "sin"))
   {
      if (degrees)
      {
         return sin(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return sin(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "cos"))
   {
      if (degrees)
      {
         return cos(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return cos(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "tan"))
   {
      if (degrees)
      {
         return tan(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return tan(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "log"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight <= 0)
      {
         throw Undefined();
      }
      return log(resRight);
   }
   if (!strcmp(Opera, "log10"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight <= 0)
      {
         throw Undefined();
      }
      return log10(resRight);
   }
   if (!strcmp(Opera, "log2"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight <= 0)
      {
         throw Undefined();
      }
      return log(resRight)/log(2.0);
   }
   if (!strcmp(Opera, "exp"))
   {
      return exp(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "abs"))
   {
      return fabs(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "asin"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight < -1 || resRight > 1)
      {
         throw Complex();
      }

      if (degrees)
      {
         return R_TO_D_MULT * asin(resRight);
      }

      return asin(resRight);
   }
   if (!strcmp(Opera, "acos"))
   {
      double resRight = Right->eval(data, band, types);
      if (resRight < -1 || resRight > 1)
      {
         throw Complex();
      }

      if (degrees)
      {
         return R_TO_D_MULT * acos(resRight);
      }

      return acos(resRight);
   }
   if (!strcmp(Opera, "atan"))
   {
      if (degrees)
      {
         return R_TO_D_MULT * atan(Right->eval(data, band, types));
      }

      return atan(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "sinh"))
   {
      if (degrees)
      {
         return sinh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return sinh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "cosh"))
   {
      if (degrees)
      {
         return cosh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return cosh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "tanh"))
   {
      if (degrees)
      {
         return tanh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return tanh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "csc"))
   {
      if (degrees)
      {
         return 1/sin(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/sin(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "sec"))
   {
      if (degrees)
      {
         return 1/cos(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/cos(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "cot"))
   {
      if (degrees)
      {
         return 1/tan(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/tan(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "acsc"))
   {
      double resRight = 1 / Right->eval(data, band, types);
      if (resRight < -1 || resRight > 1)
      {
         throw Complex();
      }

      if (degrees)
      {
         return R_TO_D_MULT * asin(resRight);
      }

      return asin(resRight);
   }
   if (!strcmp(Opera, "asec"))
   {
      double resRight = 1 / Right->eval(data, band, types);
      if (resRight < -1 || resRight > 1)
      {
         throw Complex();
      }

      if (degrees)
      {
         return R_TO_D_MULT * acos(resRight);
      }

      return acos(resRight);
   }
   if (!strcmp(Opera, "acot"))
   {
      if (degrees)
      {
         return R_TO_D_MULT * atan(1/Right->eval(data, band, types));
      }

      return atan(1/Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "csch"))
   {
      if (degrees)
      {
         return 1/sinh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/sinh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "sech"))
   {
      if (degrees)
      {
         return 1/cosh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/cosh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "coth"))
   {
      if (degrees)
      {
         return 1/tanh(D_TO_R_MULT*Right->eval(data, band, types));
      }

      return 1/tanh(Right->eval(data, band, types));
   }
   if (!strcmp(Opera, "rand"))
   {
      return GRand()*Right->eval(data, band, types);
   }

   return 0.0;
}
