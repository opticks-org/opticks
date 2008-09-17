/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include "DtedShared.h"

#include <memory>
#include <string.h>

bool readLatLongInterval(FILE* pInputFile, float &lli)
{
   // pre: NNNN
   // post: NNN.N
   char buf[5] = {'\0'};
   if (0 == fread(&buf, sizeof(char), 4, pInputFile)) return false; // fail out

   lli = atoi(buf) / float(10); // make it DDD.D or SSS.S
   return true;
}

bool readLatLongCount(FILE* pInputFile, int &llc)
{
   char buf[5] = {'\0'};
   if (0 == fread(&buf, sizeof(char), 4, pInputFile)) return false;
   llc = atoi(buf);
   return true;
}

int hemValue(char c)
{
   switch(c)
   {
      case 'N': return 1;
      case 'E': return 1;
      case 'S': return -1;
      case 'W': return -1;
      default: return 0;
   }
}

bool readLatLong(FILE* pInputFile, float &latLong, bool lat)
{
   // pre: DDDMMSSH; NE => +, SW => -
   // post: Lat/Long value in decimal format
   latLong = 0;
   bool bSuccess = true;

   unsigned char cDegSize = 0;
   if (lat)
      cDegSize = 2;
   else
      cDegSize = 3;

   char deg[4];
   char min[3];
   char sec[3];
   char hem = 0;

   memset(deg, 0, 4);
   memset(&min[0], 0, 3);
   memset(&sec[0], 0, 3);

   if (bSuccess) bSuccess = (0 != fread(&deg, sizeof(char), cDegSize, pInputFile));
   if (bSuccess) bSuccess = (0 != fread(&min, sizeof(char), 2, pInputFile));
   if (bSuccess) bSuccess = (0 != fread(&sec, sizeof(char), 2, pInputFile));
   if (bSuccess) bSuccess = (0 != fread(&hem, sizeof(char), 1, pInputFile));

   if (bSuccess)
   {
      latLong = atoi(deg) + ( atoi(min)/float(60) ) + ( atoi(sec) / float(60 * 60) );
      latLong *= hemValue(hem);
   }

   return bSuccess;
}

 
