/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DsiHeader.h"

#include <string>
#include <string.h>

DsiHeader::DsiHeader() :
   mValid(false),
   mSecClassCode(0),
   mDataEdition(0),
   mMatchMergeVer(0),
   mLatOrigin(0),
   mLongOrigin(0),
   mSWLatCorner(0),
   mSWLongCorner(0),
   mNWLatCorner(0),
   mNWLongCorner(0),
   mNELatCorner(0),
   mNELongCorner(0),
   mSELatCorner(0),
   mSELongCorner(0),
   mClockwiseOrientation(0),
   mLatInterval(0),
   mLongInterval(0),
   mLatCount(0),
   mLongCount(0),
   mPartCellIndicator(0)
{
   // wipe out the memory with null characters
   memset(&mDSI[0], 0, DSI_SIZE + 1);
   memset(&mSCRM[0], 0, DSI_SCRM_SIZE + 1);
   memset(&mSecHandleDesc[0], 0, DSI_SEC_DESC_SIZE + 1);
   memset(&mDesignator[0], 0, DSI_DESIGNATOR_SIZE + 1);
   memset(&mUniqueRef[0], 0, DSI_UNIQUE_REF_SIZE + 1);
   memset(&mMaintenanceDate[0], 0, DSI_MAINT_DATE_SIZE + 1);
   memset(&mMatchMergeDate[0], 0, DSI_MM_DATE_SIZE + 1);
   memset(&mMaintDesc[0], 0, DSI_MAINT_DESC_SIZE + 1);
   memset(&mProdCode[0], 0, DSI_PROD_CODE_SIZE + 1);
   memset(&mMILSpec[0], 0, DSI_MIL_SPEC_SIZE + 1);
   memset(&mProdSpec[0], 0, DSI_PROD_SPEC_SIZE + 1);
   memset(&mProdSpecDate[0], 0, DSI_PROD_SPEC_DATE_SIZE + 1);
   memset(&mMeanSeaLevel[0], 0, DSI_MSL_SIZE + 1);
   memset(&mHDC[0], 0, DSI_HDC_SIZE + 1);
   memset(&mDCS[0], 0, DSI_DCS_SIZE + 1);
   memset(&mCompilDate[0], 0, DSI_COMP_DATE_SIZE + 1);
}

bool DsiHeader::readHeader(FILE* pInputFile)
{
   if (pInputFile == NULL)
   {
      return false;
   }

   // DL06242003: throw an exception if something doesn't read right!
   size_t beforeRead = ftell(pInputFile);

   // put read statements here
   if (0 == fread(&mDSI, sizeof(char), DSI_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mSecClassCode, sizeof(char), 1, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mSCRM, sizeof(char), DSI_SCRM_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mSecHandleDesc, sizeof(char), DSI_SEC_DESC_SIZE, pInputFile))
   {
      return false;
   }

   // seek past reserved block of size 26
   if (0 != fseek(pInputFile, 26, SEEK_CUR))
   {
      return false;
   }

   if (0 == fread(&mDesignator, sizeof(char), DSI_DESIGNATOR_SIZE, pInputFile))
   {
      return false;
   }

   if (strncmp(mDesignator, "DTED", 4) != 0)
   {
      return false; // check if you're DTED or not
   }

   if (0 == fread(&mUniqueRef, sizeof(char), DSI_UNIQUE_REF_SIZE, pInputFile))
   {
      return false;
   }

   // seek past reserved block of size 8
   if (0 != fseek(pInputFile, 8, SEEK_CUR))
   {
      return false;
   }

   if (!readDataEdition(pInputFile, mDataEdition))
   {
      return false;
   }

   if (0 == fread(&mMatchMergeVer, sizeof(char), 1, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mMaintenanceDate, sizeof(char), DSI_MAINT_DATE_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mMatchMergeDate, sizeof(char), DSI_MM_DATE_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mMaintDesc, sizeof(char), DSI_MAINT_DESC_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mProdCode, sizeof(char), DSI_PROD_CODE_SIZE, pInputFile))
   {
      return false;
   }

   // seek past reserved block of size 16
   if (0 != fseek(pInputFile, 16, SEEK_CUR))
   {
      return false;
   }

   if (0 == fread(&mMILSpec, sizeof(char), DSI_MIL_SPEC_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mProdSpec, sizeof(char), DSI_PROD_SPEC_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mProdSpecDate, sizeof(char), DSI_PROD_SPEC_DATE_SIZE, pInputFile))
   {
      return false;
   }

   // DL06242003: I don't understand MSL yet, so I'll just read in the chars and deal with it later
   if (0 == fread(&mMeanSeaLevel, sizeof(char), DSI_MSL_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mHDC, sizeof(char), DSI_HDC_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mDCS, sizeof(char), DSI_DCS_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mCompilDate, sizeof(char), DSI_COMP_DATE_SIZE, pInputFile))
   {
      return false;
   }

   // seek past reserved block of size 22
   if (0 != fseek(pInputFile, 22, SEEK_CUR))
   {
      return false;
   }

   if (!readLatLongOrigin(pInputFile, mLatOrigin, true, true))
   {
      return false;
   }

   if (!readLatLongOrigin(pInputFile, mLongOrigin, false, true))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mSWLatCorner, true))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mSWLongCorner, false))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mNWLatCorner, true))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mNWLongCorner, false))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mNELatCorner, true))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mNELongCorner, false))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mSELatCorner, true))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mSELongCorner, false))
   {
      return false;
   }

   if (!readOrientation(pInputFile, mClockwiseOrientation))
   {
      return false;
   }

   if (!readLatLongInterval(pInputFile, mLatInterval))
   {
      return false;
   }

   if (!readLatLongInterval(pInputFile, mLongInterval))
   {
      return false;
   }

   if (!readLatLongCount(pInputFile, mLatCount))
   {
      return false;
   }

   if (!readLatLongCount(pInputFile, mLongCount))
   {
      return false;
   }

   if (!readPCI(pInputFile, mPartCellIndicator))
   {
      return false;
   }

   // seek past reserved blocks totalling 357
   if (0 != fseek(pInputFile, 357, SEEK_CUR))
   {
      return false;
   }

   size_t afterRead = ftell(pInputFile);

   if ((afterRead - beforeRead) != DSI_PROPER_TOTAL) // we've read the wrong # of bytes!
   {
      return false; // kick out
   }

   // what a relief, we're done and succeeded
   mValid = true;
   return true;
}

bool DsiHeader::isValid() const
{
   return mValid;
}

float DsiHeader::getNELatCorner() const
{
   return mNELatCorner;
}

float DsiHeader::getNELongCorner() const
{
   return mNELongCorner;
}

float DsiHeader::getSELatCorner() const
{
   return mSELatCorner;
}

float DsiHeader::getSELongCorner() const
{
   return mSELongCorner;
}

float DsiHeader::getNWLatCorner() const
{
   return mNWLatCorner;
}

float DsiHeader::getNWLongCorner() const
{
   return mNWLongCorner;
}

float DsiHeader::getSWLatCorner() const
{
   return mSWLatCorner;
}

float DsiHeader::getSWLongCorner() const
{
   return mSWLongCorner;
}

char DsiHeader::getSecurityCode() const
{
   return mSecClassCode;
}

// private functions for readHeader

bool DsiHeader::readDataEdition(FILE* pInputFile, short& dataEd)
{
   char buf[DSI_ED_NO + 1];
   memset(&buf[0], 0, DSI_ED_NO + 1);
   if (0 == fread(&buf, sizeof(char), DSI_ED_NO, pInputFile))
   {
      return false;
   }

   dataEd = atoi(buf);
   return true;
}

bool DsiHeader::readLatLongOrigin(FILE* pInputFile, float& latLongOrigin, bool lat, bool hasHem)
{
   latLongOrigin = 0;

   unsigned char ucReadSize = 0;
   if (lat) // latitude value being read -> DD
   {
      ucReadSize = 2;
   }
   else // longitude value being read -> DDD
   {
      ucReadSize = 3;
   }

   char deg[4] = { '\0' }; // DD or DDD, depending on whether a LAT or a LONG
   char min[3] = { '\0' }; // "MM"
   char sec[5] = { '\0' }; // "SS.S"
   char hem = 0;

   if (0 == fread(&deg, sizeof(char), ucReadSize, pInputFile))
   {
      return false; // DD or DDD
   }

   if (0 == fread(&min, sizeof(char), 2, pInputFile))
   {
      return false; // MM
   }

   if (0 == fread(&sec, sizeof(char), 4, pInputFile))
   {
      return false; // SS.S
   }

   if (hasHem) // if there's a hemisphere value that needs to be read, take care of it
   {
      if (0 == fread(&hem, sizeof(char), 1, pInputFile))
      {
         return false; // H - hemisphere code
      }
   }

   float fsec = static_cast<float>(atof(sec)); // we shouldn't lose any precision since it's going to be small...

   latLongOrigin = (atoi(deg) + (atoi(min) / static_cast<float>(60)) + (fsec / static_cast<float>(60 * 60)));

   if (hasHem)
   {
      latLongOrigin *= hemValue(hem); // need to take into account the hemisphere value
   }

   return true;
}

bool DsiHeader::readOrientation(FILE* pInputFile, float& orientation)
{
   return readLatLongOrigin(pInputFile, orientation, false, false); // in DDDMMSS.S format, no hemisphere value read
}

bool DsiHeader::readPCI(FILE* pInputFile, float& pci)
{
   // we don't care about the PCI, so we'll skip it; it would normally need a method like this,
   // but since we don't care, we'll just fseek char buf[DSI_PCI_SIZE];
   pci = 0;
   if (0 != fseek(pInputFile, DSI_PCI_SIZE, SEEK_CUR))
   {
      return false;
   }

   return true;
}
