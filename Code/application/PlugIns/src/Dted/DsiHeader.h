/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DSIHEADER_H
#define DSIHEADER_H

#include "DtedShared.h"

const int DSI_SIZE = 3;
const int DSI_SCRM_SIZE = 2;
const int DSI_SEC_DESC_SIZE = 27;
const int DSI_DESIGNATOR_SIZE = 5;
const int DSI_UNIQUE_REF_SIZE = 15;
const int DSI_ED_NO = 2;
const int DSI_MAINT_DATE_SIZE = 4;
const int DSI_MM_DATE_SIZE = 4;
const int DSI_MAINT_DESC_SIZE = 4;
const int DSI_PROD_CODE_SIZE = 8;
const int DSI_MIL_SPEC_SIZE = 9;
const int DSI_PROD_SPEC_SIZE = 2;
const int DSI_PROD_SPEC_DATE_SIZE = 4;
const int DSI_MSL_SIZE = 3;
const int DSI_HDC_SIZE = 5;
const int DSI_DCS_SIZE = 10;
const int DSI_COMP_DATE_SIZE = 4;
const int DSI_LAT_OSIZE = 9;
const int DSI_LONG_OSIZE = 10;
const int DSI_LAT_CSIZE = 7;
const int DSI_LONG_CSIZE = 8;
const int DSI_RO_SIZE = 9;
const int DSI_PCI_SIZE = 2;
const int DSI_PROPER_TOTAL = 648;

class DsiHeader
{
public:
   /*
    * Constructor for a DSI Header does very little except init all values to 0
    * does not init arrays
    */
   DsiHeader();

   /*
    * Constructor for a DSI Header to be loaded by way of a FILE pointer for DTED data
    * does not init arrays
    * 
    * @param   pInputFile
    *          Pointer to the file that the DSI Header will be loaded from; uses a pointer
    *          to avoid having to open and reopen files when loading all 3 headers needed
    *          for DTED.
    */
   bool readHeader(FILE* pInputFile);

   bool isValid() const;

   float getNELatCorner() const;
   float getNELongCorner() const;

   float getSELatCorner() const;
   float getSELongCorner() const;

   float getNWLatCorner() const;
   float getNWLongCorner() const;

   float getSWLatCorner() const;
   float getSWLongCorner() const;

   char getSecurityCode() const;

private:
   bool mValid;
   char mDSI[DSI_SIZE + 1];
   char mSecClassCode;
   char mSCRM[DSI_SCRM_SIZE + 1]; // Security Control and Release Markings
   char mSecHandleDesc[DSI_SEC_DESC_SIZE + 1];
   char mDesignator[DSI_DESIGNATOR_SIZE + 1];
   char mUniqueRef[DSI_UNIQUE_REF_SIZE + 1];
   short mDataEdition;
   char mMatchMergeVer;
   char mMaintenanceDate[DSI_MAINT_DATE_SIZE + 1];
   char mMatchMergeDate[DSI_MM_DATE_SIZE + 1];
   char mMaintDesc[DSI_MAINT_DESC_SIZE + 1];
   char mProdCode[DSI_PROD_CODE_SIZE + 1];
   char mMILSpec[DSI_MIL_SPEC_SIZE + 1];
   char mProdSpec[DSI_PROD_SPEC_SIZE + 1];
   char mProdSpecDate[DSI_PROD_SPEC_DATE_SIZE + 1];

   // DL06242003: I don't understand MSL yet, so I'll just read in the chars and deal with it later
   char mMeanSeaLevel[DSI_MSL_SIZE + 1];

   char mHDC[DSI_HDC_SIZE + 1]; // Horizontal Datum Code
   char mDCS[DSI_DCS_SIZE + 1]; // Digitizing Collection System, free text
   char mCompilDate[DSI_COMP_DATE_SIZE + 1];
   float mLatOrigin;
   float mLongOrigin;
   float mSWLatCorner;
   float mSWLongCorner;
   float mNWLatCorner;
   float mNWLongCorner;
   float mNELatCorner;
   float mNELongCorner;
   float mSELatCorner;
   float mSELongCorner;
   float mClockwiseOrientation;
   float mLatInterval;
   float mLongInterval;
   int mLatCount;
   int mLongCount;
   float mPartCellIndicator;

   // private functions to read various DTED-specific values
   bool readDataEdition(FILE* pInputFile, short& dataEd);

   bool readLatLongOrigin(FILE* pInputFile, float& latLongOrigin, bool lat, bool hasHem);

   bool readOrientation(FILE* pInputFile, float& orientation);
   bool readPCI(FILE* pInputFile, float& pci);
};

#endif
