/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#if defined(WIN_API)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "MipMappedTextures.h"

void GenerateMipMappedTextures(unsigned int *pTexData1, int sourceWidth, int sourceHeight)
{
   glTexImage2D(GL_TEXTURE_2D, 0, 4, sourceWidth, sourceHeight, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, pTexData1);

   unsigned int *pTexData2 = new unsigned int[sourceWidth*sourceHeight/4];
   unsigned int *pDest, *pSource;

   pDest = pTexData2;
   pSource = pTexData1;

   int j, k, l, m, n;
   for (k=1; sourceWidth>1 && sourceHeight>1; k++)
   {
      for (j=0; j<sourceHeight; j+=2)
      {
         for (l=0; l<sourceWidth; l+=2)
         {
            unsigned int red=0, green=0, blue=0, alpha=0;
            for(m=0; m<2; m++)
            {
               for(n=0; n<2; n++)
               {
                  unsigned int value = pSource[sourceWidth*(j+m)+l+n];
                  red += value&0xff;
                  green += (value & 0xff00)>>8;
                  blue += (value & 0xff0000)>>16;
                  alpha += (value & 0xff000000)>>24;
               }
            }
            red /= 4;
            green /= 4;
            blue /= 4;
            alpha /= 4;

            pDest[(j*sourceWidth/4 + l/2)] = red+(green<<8)+(blue<<16)+(alpha<<24);
         }
      }
      glTexImage2D(GL_TEXTURE_2D, k, 4, sourceWidth/2, sourceHeight/2, 0,
         GL_RGBA, GL_UNSIGNED_BYTE, pDest);

      sourceWidth /= 2;
      sourceHeight /= 2;

      // swap arrays
      unsigned int *pTempPtr = pSource;
      pSource = pDest;
      pDest = pTempPtr;
   }

   delete pTexData2;
}
