/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "BandUtilities.h"
#include "AppVerify.h"

using namespace std;

template <class T>
int* deriveBadBands(const vector<T>& bandsToSave, const unsigned int totalBands,
                    unsigned int& numBad)
{
   int* plzBadBands = NULL;
   try
   {
      unsigned int i, j, k;

      numBad = 0;
      if ( totalBands > bandsToSave.size() )
      {
         numBad = totalBands - bandsToSave.size();
         plzBadBands = new int[numBad];
         for ( i = 0, j = 0, k = 0; i < totalBands; i++ )
         {
            if (j >= bandsToSave.size())
            {
               plzBadBands[k++] = i;
            }
            else if ( j < bandsToSave.size() && bandsToSave.at(j) > i )
            {
               plzBadBands[k++] = i;
            }
            else
            {
               j++;
            }
         }
      }
      return plzBadBands;
   }
   catch(...)
   {
      // TODO: If interactive mode, issue dialog? 
      LogVerificationError("plzBadBands == NULL", "Exception in deriveBadBands");
      delete plzBadBands;
      numBad = 0;
      return NULL;
   }
}
