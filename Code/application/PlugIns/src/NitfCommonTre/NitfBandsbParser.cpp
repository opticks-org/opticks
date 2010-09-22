/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>

#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "DynamicObject.h"
#include "NitfBandsbParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "SpecialMetadata.h"

#include <set>
#include <sstream>

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;


REGISTER_PLUGIN(OpticksNitfCommonTre, BandsbParser, Nitf::BandsbParser());

/***************************************************************************
BANDSB Support Data Extension - Version 8.01 (Sept 2003)

Background: During an investigation of various MS data formats and the NITF
2.1 format, investigators nominated a list of orphaned band specific
parameters. The NITF 2.1 file format had no defined fields/extensions to
store this band level data. Included in this investigation were the
following MS formats; SEBASS, AVIRIS, PCI(PIX), and AISA.

Purpose: The BANDSB extension is designed to supplement information in the
NITF image subheader where additional parametric data are required. This
data extension is placed in each image subheader as required. Multiple
BANDSB extensions may be placed within one image subheader; for example,
when the amount of parametric data exceeds the maximum allowed length of a
single BANDSB extension or when implementers wish to segregate certain
parameters in their own extension.

NOTE:
-----
The following changes have been made to the field names from the standard:
1) Embedded blanks are replaced with "_"
2) The Auxiliary Band Parameter data has two numbers, the "auxiliary"
   counter and the "band" counter, that need to be separated in the field
   name to allow it to be parsed. Therefore, the numbers are separated with
   a "#"

***************************************************************************/

namespace
{
   bool bitTest(unsigned int bitMask, unsigned int bit)
   {
      return (bitMask >> bit & 0x01);
   }
}

Nitf::BandsbParser::BandsbParser() :
   mWavelengthsInInverseCentimeters(false)
{
   setName("BANDSB");
   setDescriptorId("{9A2CC336-E28D-4284-9CEC-896FC92F14DD}");
   setSubtype(CreateOnExportSubtype());      // This call is needed for exportMetadata()
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::BandsbParser::runAllTests(Progress* pProgress, ostream& failure)
{
   // Data for test 1, also used for tests 2 and 3
   string test1_1 = "00007"                                     // COUNT
                   "RAW                     "                   // RADIOMETRIC QUANTITY
                   "S";                                         // RADIOMETRIC QUANTITY UNIT

   unsigned char test1_2[] = {0x3f, 0x80, 0x00, 0x00,           // SCALE FACTOR  0x3f800000 == (float)1.0
                    0x00, 0x00, 0x00, 0x00};                    // ADDITIVE FACTOR  0x00000000 == (float)0.0

   string test1_3 = "-------"                                   // ROW GSD
                   "M"                                          // ROW GSD UNIT
                   "-------"                                    // COL GSD
                   "M"                                          // COL GSD UNIT
                   "-------"                                    // SPT RESP ROW
                   "M"                                          // SPT RESP UNIT ROW
                   "-------"                                    // SPT RESP COL
                   "M"                                          // SPT RESP UNIT COL
                   "123456789012345678901234567890123456789012345678"; // DATA FLD 1

   unsigned char test1_4[] = {0x5e, 0xbb, 0xfc, 0x01};          // EXISTENCE MASK  0x5ebbfc01 bit mask

   string test1_5 =
      "0000.01"                                                 // DIAMETER
      "U"                                                       // WAVE LENGTH UNIT
      "Band 1 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      "970929123456.123"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 2 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01985.5"                                                 // LBOUND
      "01995.4"                                                 // UBOUND
      "970929123456.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.3"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 3 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01995.5"                                                 // LBOUND
      "02005.4"                                                 // UBOUND
      "9709291234--.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.4"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 4 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02005.5"                                                 // LBOUND
      "02015.4"                                                 // UBOUND
      "97092912----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.5"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 5 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02015.5"                                                 // LBOUND
      "02025.4"                                                 // UBOUND
      "970929------.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.6"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 6 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02025.5"                                                 // LBOUND
      "02035.4"                                                 // UBOUND
      "970929------.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.7"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "Band 7 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02035.5"                                                 // LBOUND
      "02045.4"                                                 // UBOUND
      "970929------.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.8"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "02"                                                      // NUM AUX B
      "00"                                                      // NUM AUX C
      "I"                                                       // BAPF
      "1234567"                                                 // UBAP
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "0000000000"                                              // APN
      "A"                                                       // BAPF
      "1234567"                                                 // UBAP
      "User Defined 1      "                                    // APA
      "User Defined 2      "                                    // APA
      "User Defined 3      "                                    // APA
      "User Defined 4      "                                    // APA
      "User Defined 5      "                                    // APA
      "User Defined 6      "                                    // APA
      "User Defined 7      "                                    // APA
      ;

   // Data for test 4
   string test4_1 =
      "00007"                                                   // COUNT
      "RAW                     "                                // RADIOMETRIC QUANTITY
      "S";                                                      // RADIOMETRIC QUANTITY UNIT

   string test4_1_ERROR =
      "00007"                                                   // COUNT
      "RAW\037                    "                             // RADIOMETRIC QUANTITY
      "S";                                                      // RADIOMETRIC QUANTITY UNIT

   unsigned char test4_2[] = {0x3f, 0x80, 0x00, 0x00,           // SCALE FACTOR  0x3f800000 == (float)1.0
                     0x00, 0x00, 0x00, 0x00};                   // ADDITIVE FACTOR  0x00000000 == (float)0.0

   string test4_3 =
      "-------"                                                 // ROW GSD
      "M"                                                       // ROW GSD UNIT
      "-------"                                                 // COL GSD
      "M"                                                       // COL GSD UNIT
      "-------"                                                 // SPT RESP ROW
      "M"                                                       // SPT RESP UNIT ROW
      "-------"                                                 // SPT RESP COL
      "M"                                                       // SPT RESP UNIT COL
      "123456789012345678901234567890123456789012345678"        // DATA FLD 1
      ;

   unsigned char test4_4[] = {0x5e, 0xbb, 0xf0, 0x00};          // EXISTENCE MASK  0x5ebbf000 bit mask

   string test4_5 =
      "0000.01"                                                 // DIAMETER
      "U"                                                       // WAVE LENGTH UNIT
      "Band 1 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      "970929------.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 2 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01985.5"                                                 // LBOUND
      "01995.4"                                                 // UBOUND
      "97092901----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.3"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 3 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01995.5"                                                 // LBOUND
      "02005.4"                                                 // UBOUND
      "97092902----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.4"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 4 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02005.5"                                                 // LBOUND
      "02015.4"                                                 // UBOUND
      "97092903----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.5"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 5 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02015.5"                                                 // LBOUND
      "02025.4"                                                 // UBOUND
      "97092904----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.6"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 6 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02025.5"                                                 // LBOUND
      "02035.4"                                                 // UBOUND
      "97092905----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.7"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 7 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02035.5"                                                 // LBOUND
      "02045.4"                                                 // UBOUND
      "97092906----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.8"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      ;

   string test4_5_ERROR =
      "0000.01"                                                 // DIAMETER
      "V"                                                       // WAVE_LENGTH_UNIT : ERROR == "V"
      "Band 1 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      "970929------.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 2 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01985.5"                                                 // LBOUND
      "01995.4"                                                 // UBOUND
      "97092901----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.3"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 3 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01995.5"                                                 // LBOUND
      "02005.4"                                                 // UBOUND
      "97092902----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.4"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 4 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02005.5"                                                 // LBOUND
      "02015.4"                                                 // UBOUND
      "97092903----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.5"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 5 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02015.5"                                                 // LBOUND
      "02025.4"                                                 // UBOUND
      "97092904----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.6"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 6 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02025.5"                                                 // LBOUND
      "02035.4"                                                 // UBOUND
      "97092905----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.7"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "Band 7 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // FWHM
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "02035.5"                                                 // LBOUND
      "02045.4"                                                 // UBOUND
      "97092906----.---"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.8"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      ;

   // Data for test 5 - The values in this test are not ment to be respresentitive. They are for
   // stressing the parser/validator
   string test5_1 =
      "00003"                                                   // COUNT
      "RAW                     "                                // RADIOMETRIC QUANTITY
      "S";                                                      // RADIOMETRIC QUANTITY UNIT

   unsigned char test5_2[] = {0x3f, 0x80, 0x00, 0x00,           // SCALE FACTOR     0x3f800000 == (float)1.0
                     0x47, 0xc3, 0x50, 0x00};                   // ADDITIVE FACTOR  0x47c35000 == (float)100000.0

   string test5_3 =
      "000.001"                                                 // ROW GSD
      "M"                                                       // ROW GSD UNIT
      "000.001"                                                 // COL GSD
      "R"                                                       // COL GSD UNIT
      "9999.99"                                                 // SPT RESP ROW
      "R"                                                       // SPT RESP UNIT ROW
      "9999.99"                                                 // SPT RESP COL
      "M"                                                       // SPT RESP UNIT COL
      "123456789012345678901234567890123456789012345678"        // DATA FLD 1
      ;

   unsigned char test5_4[] = {0xff, 0xff, 0xff, 0xc1};     // EXISTENCE MASK  0xffffffc1  == all bits set for full test

   string test5_5 =
      "WITHIN ATMOSPHERE       "                                // RADIOMETRIC_ADJUSTMENT_SURFACE
      ;

   unsigned char test5_6[] = {0xC7, 0xc3, 0x50, 0x00}; // ATMOSPHERIC_ADJUSTMENT_ALTITUDE 0xC7c35000==(float)-100000.0

   string test5_7 =
      "0000.01"                                                 // DIAMETER
      "12345678901234567890123456789012"                        // DATA_FLD_2
      "U"                                                       // WAVE LENGTH UNIT
               // The following fields repeat once per band
      "Band 1 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // CWAVE
      "0.00001"                                                 // FWHM
      "10000.0"                                                 // FWHM_UNC
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      ;

   unsigned char test5_8[] = {0x67, 0x53, 0xc2, 0x1c,           // SCALE FACTOR     0x6753c21c == (float)1.0e24
                     0x97, 0x9a, 0xbe, 0x15 };                  // ADDITIVE FACTOR  0x979abe15 == (float)-1.0e-24

   string test5_9 =
      "060720081530.123"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "1234567890123456"                                        // DATA_FLD_3
      "123456789012345678901234"                                // DATA_FLD_4
      "12345678901234567890123456789012"                        // DATA_FLD_5
      "123456789012345678901234567890123456789012345678"        // DATA_FLD_6
      ;


   string test5_10 =
      "Band 2 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // CWAVE
      "0.00001"                                                 // FWHM
      "10000.0"                                                 // FWHM_UNC
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      ;

   unsigned char test5_11[] = {0xe7, 0x53, 0xc2, 0x1c,          // SCALE FACTOR     0x6753c21c == (float)-1.0e24
                      0x17, 0x9a, 0xbe, 0x15 };                 // ADDITIVE FACTOR  0x979abe15 == (float)1.0e-24

   string test5_12 =
      "060720081530.123"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "1234567890123456"                                        // DATA_FLD_3
      "123456789012345678901234"                                // DATA_FLD_4
      "12345678901234567890123456789012"                        // DATA_FLD_5
      "123456789012345678901234567890123456789012345678"        // DATA_FLD_6
      ;

   string test5_13 =
      "Band 3 8901234567890123456789012345678901234567890"      // BANDID
      "1"                                                       // BAD BAND
      "0.1"                                                     // NIIRS
      "00001"                                                   // FOCAL LEN
      "0.00001"                                                 // CWAVE
      "0.00001"                                                 // FWHM
      "10000.0"                                                 // FWHM_UNC
      "0.00001"                                                 // NOM WAVE
      "0.00001"                                                 // NOM WAVEUNC
      "01975.5"                                                 // LBOUND
      "01985.4"                                                 // UBOUND
      ;

   unsigned char test5_14[] = {0x40, 0x49, 0x0f, 0xdb,          // SCALE FACTOR    0x40490fdb == pi == (float)3.1415927
                      0x40, 0x2d, 0xf8, 0x54 };                 // ADDITIVE FACTOR 0x402df854 ==  e == (float)2.7182817

   string test5_15 =
      "060720081530.123"                                        // START TIME
      "000010"                                                  // INT TIME
      "0001.2"                                                  // CALDRK
      "00.01"                                                   // CALIBRATION SENSITIVITY
      "0000.01"                                                 // FOW GSD
      "000.001"                                                 // ROW GSD UNC
      "M"                                                       // ROW GSD UNIT
      "0000.01"                                                 // COL GSD
      "000.001"                                                 // COL GSD UNC
      "M"                                                       // COL GSD UNIT
      "00.00"                                                   // BKNOISE
      "00.00"                                                   // SCNNOISE
      "000.001"                                                 // SPT RESP FUNCTION ROW
      "000.001"                                                 // SPT RESP UNC ROW
      "M"                                                       // SPT RESP UNIT ROW
      "000.001"                                                 // SPT RESP FUNCTION COL
      "000.001"                                                 // SPT RESP UNC COL
      "M"                                                       // SPT RESP UNIT COL
      "1234567890123456"                                        // DATA_FLD_3
      "123456789012345678901234"                                // DATA_FLD_4
      "12345678901234567890123456789012"                        // DATA_FLD_5
      "123456789012345678901234567890123456789012345678"        // DATA_FLD_6

            // End of repeating data for each band

      "03"                                                      // NUM AUX B
      "03"                                                      // NUM AUX C

      "I"                                                       // BAPF
      "1234567"                                                 // UBAP
      "0000000001"                                              // APN
      "0000000002"                                              // APN
      "0000000003"                                              // APN

      "A"                                                       // BAPF
      "1234567"                                                 // UBAP
      "User Defined 1      "                                    // APA
      "User Defined 2      "                                    // APA
      "User Defined 3      "                                    // APA

      "R"                                                       // BAPF
      "1234567"                                                 // UBAP
      ;

   unsigned char test5_16[] = {0xc0, 0x49, 0x0f, 0xdb,          // APR   0xc0490fdb == pi == (float)-3.1415927
                      0xc0, 0x2d, 0xf8, 0x54,                   // APR   0xc02df854 ==  e == (float)-2.7182817
                      0x42, 0x28, 0x00, 0x00                    // APR   0x42280000 == 42
                      };
   string test5_17 =
      "I"                                                       // CAPF
      "1234567"                                                 // UCAP
      "0000000001"                                              // APN

      "A"                                                       // CAPF
      "1234567"                                                 // UCAP
      "User Defined 1 - APA"                                    // APA

      "R"                                                       // CAPF
      "1234567"                                                 // UCAP
      ;

   unsigned char test5_18[] = {0x42, 0x28, 0x00, 0x00};                 // APR   0xc2280000 == -42


   string test5_17_Error =
      "I"                                                       // CAPF
      "1234567"                                                 // UCAP
      "0000000001"                                              // APN

      "A"                                                       // CAPF
      "1234567"                                                 // UCAP
      "User Defined 1 - APA"                                    // APA

      "D"                                                       // CAPF: ERROR: "D" is an invalid value
      "1234567"                                                 // UCAP
      ;

   unsigned char test9_4[] = {0x00, 0x00, 0x00, 0x01}; // EXISTENCE MASK  0x00000001 == minumim bits set for min test
                                                                // requires NUM_AUX_B and NUM_AUX_C only
   string test9_5 =
      "00"                                                      // NUM AUX B
      "00"                                                      // NUM AUX C
      ;


   unsigned char test10_4[] = {0x00, 0x00, 0x00, 0x02}; // EXISTENCE MASK  0x00000002  == only 2 bit set for min test
                                                                // requires no other values follow


   stringstream input;
   input << test1_1;
   input.write(reinterpret_cast<char*>(test1_2), sizeof(test1_2));
   input << test1_3;
   input.write(reinterpret_cast<char*>(test1_4), sizeof(test1_4));
   input << test1_5;

   size_t numBytes = input.str().size();

   FactoryResource<DynamicObject> treDO;
   string errorMessage;

   bool success = toDynamicObject(input, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   TreState status(INVALID);
   if (success)
   {
      status = isTreValid(*treDO.get(), failure);
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // Start of test 2 - Negative test: 1 extra byte in input stream
   errorMessage.clear();
   stringstream input2(input.str() + "1");
   numBytes = input2.str().size() + 1;
   success = toDynamicObject(input2, numBytes, *treDO.get(), errorMessage);
   if (success == true)     // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 extra byte failed\n";
      treDO->clear();
      return false;
   }

   // start of test 3 - Negative test: 1 byte short in input stream
   errorMessage.clear();
   string negdata3(input.str());        // data for test 3 not the 3rd data set
   negdata3.resize(negdata3.size()-1);
   stringstream input3(negdata3);
   numBytes = negdata3.size();
   success = toDynamicObject(input3, numBytes, *treDO.get(), errorMessage);
   if (success == true)  // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 byte short failed\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   // start of test 4 - alternate positive test 2
   errorMessage.clear();
   stringstream input4;
   input4 << test4_1;
   input4.write(reinterpret_cast<char*>(test4_2), sizeof(test4_2));
   input4 << test4_3;
   input4.write(reinterpret_cast<char*>(test4_4), sizeof(test4_4));
   input4 << test4_5;

   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   status = INVALID;
   if (success)
   {
      status = isTreValid(*treDO.get(), failure);
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // start of test 5 - alternate positive test 3
   errorMessage.clear();
   stringstream input5;
   input5 << test5_1;
   input5.write(reinterpret_cast<char*>(test5_2), sizeof(test5_2));
   input5 << test5_3;
   input5.write(reinterpret_cast<char*>(test5_4), sizeof(test5_4));
   input5 << test5_5;
   input5.write(reinterpret_cast<char*>(test5_6), sizeof(test5_6));
   input5 << test5_7;
   input5.write(reinterpret_cast<char*>(test5_8), sizeof(test5_8));
   input5 << test5_9;
   input5 << test5_10;
   input5.write(reinterpret_cast<char*>(test5_11), sizeof(test5_11));
   input5 << test5_12;
   input5 << test5_13;
   input5.write(reinterpret_cast<char*>(test5_14), sizeof(test5_14));
   input5 << test5_15;
   input5.write(reinterpret_cast<char*>(test5_16), sizeof(test5_16));
   input5 << test5_17;
   input5.write(reinterpret_cast<char*>(test5_18), sizeof(test5_18));

   string data5(input5.str());
   numBytes = data5.size();

   success = toDynamicObject(input5, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << endl;
      errorMessage.clear();
   }

   status = INVALID;
   if (success)
   {
      status = isTreValid(*treDO.get(), failure);
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // start of test 6 - Negative test Invalid CAPF: ERROR: "D"
   errorMessage.clear();
   stringstream input6;
   input6 << test5_1;
   input6.write(reinterpret_cast<char*>(test5_2), sizeof(test5_2));
   input6 << test5_3;
   input6.write(reinterpret_cast<char*>(test5_4), sizeof(test5_4));
   input6 << test5_5;
   input6.write(reinterpret_cast<char*>(test5_6), sizeof(test5_6));
   input6 << test5_7;
   input6.write(reinterpret_cast<char*>(test5_8), sizeof(test5_8));
   input6 << test5_9;
   input6 << test5_10;
   input6.write(reinterpret_cast<char*>(test5_11), sizeof(test5_11));
   input6 << test5_12;
   input6 << test5_13;
   input6.write(reinterpret_cast<char*>(test5_14), sizeof(test5_14));
   input6 << test5_15;
   input6.write(reinterpret_cast<char*>(test5_16), sizeof(test5_16));
   input6 << test5_17_Error;

   numBytes = input6.str().size();

   success = toDynamicObject(input6, numBytes, *treDO.get(), errorMessage);

   if (success == true)  // negative test so success must == false.
   {
      failure << "Error: Negative test of bad CAPF == \"D\" failed\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   // start of test 7 - Negative test: WAVE_LENGTH_UNIT == "V"
   errorMessage.clear();
   stringstream input7;
   input7 << test4_1;
   input7.write(reinterpret_cast<char*>(test4_2), sizeof(test4_2));
   input7 << test4_3;
   input7.write(reinterpret_cast<char*>(test4_4), sizeof(test4_4));
   input7 << test4_5_ERROR;

   numBytes = input7.str().size();

   success = toDynamicObject(input7, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success == false)
   {
      failure << "Error: Negative test of bad WAVE_LENGTH_UNIT == \"V\" failed in toDynamicObject\n";
      treDO->clear();
      return false;
   }
   else
   {
      stringstream tmpStream;
      status = isTreValid(*treDO.get(), tmpStream);
      if (status != SUSPECT)
      {
         failure << "Error: Negative test of bad WAVE_LENGTH_UNIT == \"V\" failed in isTreValid\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
   }

   stringstream tmpStream;
   status = isTreValid(*treDO.get(), tmpStream);
   if (status != SUSPECT)
   {
      failure << "Error: Negative test of bad WAVE_LENGTH_UNIT == \"V\" failed in isTreValid\n";
      failure << tmpStream.str();
      treDO->clear();
      return false;
   }

   treDO->clear();

   // start of test 8 - Negative test: Bad BCS-A byte in RADIOMETRIC_QUANTITY
   errorMessage.clear();
   stringstream input8;
   input8 << test4_1_ERROR;
   input8.write(reinterpret_cast<char*>(test4_2), sizeof(test4_2));
   input8 << test4_3;
   input8.write(reinterpret_cast<char*>(test4_4), sizeof(test4_4));
   input8 << test4_5;

   numBytes = input8.str().size();

   success = toDynamicObject(input8, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success)
   {
      stringstream tmpStream2;
      status = isTreValid(*treDO.get(), tmpStream2);
      if (status != INVALID)
      {
         failure << "Error: Negative test with bad BCS-A byte failed did not return INVALID\n";
         failure << tmpStream2.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // start of test 9 - positive test: EXISTANCE MASK minimum test 0x00000001 =>
   // only NUM_AUX_B and NUM_AUX_B supplied and they = zero
   errorMessage.clear();
   stringstream input9;
   input9 << test5_1;
   input9.write(reinterpret_cast<char*>(test5_2), sizeof(test5_2));
   input9 << test5_3;
   input9.write(reinterpret_cast<char*>(test9_4), sizeof(test9_4));
   input9 << test9_5;

   numBytes = input9.str().size();

   success = toDynamicObject(input9, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success)
   {
      stringstream tmpStream2;
      status = isTreValid(*treDO.get(), tmpStream2);
      if (status != VALID)
      {
         failure << "Error: Positive test with EXISTANCE max = 0x00000001 did not return VALID\n";
         failure << tmpStream2.str();
         treDO->clear();
         return false;
      }
   }

   treDO->clear();

   if (status == INVALID)
   {
      return false;
   }

   // start of test 10 - positive test: EXISTANCE MASK minimum test = 0x00000002 => no following data
   errorMessage.clear();
   stringstream input10;
   input10 << test5_1;
   input10.write(reinterpret_cast<char*>(test5_2), sizeof(test5_2));
   input10 << test5_3;
   input10.write(reinterpret_cast<char*>(test10_4), sizeof(test10_4));

   numBytes = input10.str().size();

   success = toDynamicObject(input10, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success)
   {
      stringstream tmpStream2;
      status = isTreValid(*treDO.get(), tmpStream2);
      if (status != VALID)
      {
         failure << "Error: Positive test with EXISTANCE max = 0x00000001 did not return VALID\n";
         failure << tmpStream2.str();
         treDO->clear();
         return false;
      }
   }

   treDO->clear();
   return (status != INVALID);
}

bool bandsbDtgParse(string &fDTG,
                             unsigned short &year,
                             unsigned short &month,
                             unsigned short &day,
                             unsigned short &hour,
                             unsigned short &min,
                             unsigned short &sec,
                             double         &frac)
{
   // BandsB DTG:  YYMMDDhhmmss.sss   (This is not the standard NITF 2.1 date format.)

   //                                              J   F   M   A   M   J   J   A   S   O   N   D
   static const unsigned short maxDayOfMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

   bool allFieldsValid = true;
   year = 0;
   month = 0;
   day = 0;
   hour = 0;
   min = 0;
   sec = 0;
   frac = 0.0;

   try
   {
      year = boost::lexical_cast<unsigned short>(fDTG.substr(0, 2));
      month = boost::lexical_cast<unsigned short>(fDTG.substr(2, 2));
      day = boost::lexical_cast<unsigned short>(fDTG.substr(4, 2));
      hour = boost::lexical_cast<unsigned short>(fDTG.substr(6, 2));
      min = boost::lexical_cast<unsigned short>(fDTG.substr(8, 2));
      sec = boost::lexical_cast<unsigned short>(fDTG.substr(10, 2));
      frac = boost::lexical_cast<double>(fDTG.substr(12, 4));
   }
   catch (const boost::bad_lexical_cast&)
   {
      allFieldsValid = false;
   }

   if (year < 48)
   {
      year += 2000;
   }
   else
   {
      year += 1900;
   }

   if (month < 1 || month > 12)
   {
      return false;
   }

   if (day < 1 || day > maxDayOfMonth[month-1])
   {
      return false;
   }

   if (hour > 23)
   {
      return false;
   }

   if (min > 59)
   {
      return false;
   }

   if (sec > 59)
   {
      return false;
   }

   return allFieldsValid;
}


bool Nitf::BandsbParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   // BANDSB NITF data is always Big Endian
   vector<char> buf;             // Input buffer
   vector<unsigned char> bufUC;  // buffer for array of unsigned char
   vector<unsigned int>  bufInt; // buffer for array of int
   unsigned int length;
   bool ok(true);
   bool success(true);

   vector<char> dashes7;         // Unknown numeric are 7 char of "-"; This vector is for "!=" testing
   dashes7.resize(8);
   memcpy(&dashes7[0], "-------\0", 8);    // trailing zero needed for comp with char data in vector
   vector<char> spaces7;         // Spaces aren't allowed for unknowns, but we need to parse some which have them
   spaces7.resize(8);
   memcpy(&spaces7[0], "       \0", 8);

   readField<unsigned int>(input, output, success, BANDSB::COUNT, 5, errorMessage, buf);
   unsigned int count = QString(&buf.front()).toUInt();      // COUNT == the number of bands in the cube

   readField<string>(input, output, success, BANDSB::RADIOMETRIC_QUANTITY, 24, errorMessage, buf, true);
   readField<string>(input, output, success, BANDSB::RADIOMETRIC_QUANTITY_UNIT, 1, errorMessage, buf, true);
   if (success)
   {
      success = readFromStream(input, buf, 4) &&
         output.setAttribute(BANDSB::SCALE_FACTOR, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
      if (!success)
      {
         errorMessage += "Read or setAttribute Error with convertBinary: " + BANDSB::SCALE_FACTOR + "\n";
      }
   }
   if (success)
   {
      success = readFromStream(input, buf, 4) &&
         output.setAttribute(BANDSB::ADDITIVE_FACTOR, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
      if (!success)
      {
         errorMessage += "Read or setAttribute Error with convertBinary: " + BANDSB::ADDITIVE_FACTOR + "\n";
      }
   }

   success = success && readFromStream(input, buf, 7);
   if (buf != dashes7 && buf != spaces7)
   {
      success = success && output.setAttribute(BANDSB::ROW_GSD, QString(&buf.front()).toFloat(&ok)) && ok;
   }

   readField<string>(input, output, success, BANDSB::ROW_GSD_UNIT, 1, errorMessage, buf, true);

   success = success && readFromStream(input, buf, 7);
   if (buf != dashes7 && buf != spaces7)
   {
      success = success && output.setAttribute(BANDSB::COL_GSD, QString(&buf.front()).toFloat(&ok)) && ok;
   }

   readField<string>(input, output, success, BANDSB::COL_GSD_UNITS, 1, errorMessage, buf, true);

   success = success && readFromStream(input, buf, 7);
   if (buf != dashes7 && buf != spaces7)
   {
      success = success && output.setAttribute(BANDSB::SPT_RESP_ROW, QString(&buf.front()).toFloat(&ok)) && ok;
   }

   readField<string>(input, output, success, BANDSB::SPT_RESP_UNIT_ROW, 1, errorMessage, buf, true);

   success = success && readFromStream(input, buf, 7);
   if (buf != dashes7 && buf != spaces7)
   {
      success = success && output.setAttribute(BANDSB::SPT_RESP_COL, QString(&buf.front()).toFloat(&ok)) && ok;
   }

   readField<string>(input, output, success, BANDSB::SPT_RESP_UNIT_COL, 1, errorMessage, buf, true);

   length = 48;      // binary array of 48 single byte unsigned int's
   success = success && readFromStream(input, buf, length);
   bufUC.resize(length);
   memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
   output.setAttribute(BANDSB::DATA_FLD_1, bufUC);

   // Bit-wise Existence Mask Field 32 flags represented by the bits of this 4
   // byte (32 bit) field. (b31, b30, b29, b28 ... b2, b1, b0)

   // A bit set to zero signals that a conditional field is not present in this
   // extension. A bit set to the value one indicates the inclusion of the conditional field.
   if (success)
   {
      success = readFromStream(input, buf, 4);
      if (!success)
      {
         errorMessage += "Read Error " + BANDSB::EXISTENCE_MASK + "\n";
      }
   }
   const unsigned int existmask = convertBinary<unsigned int>(&buf[0], BIG_ENDIAN_ORDER);
   if (success)
   {
      success = output.setAttribute(BANDSB::EXISTENCE_MASK, existmask);
      if (!success)
      {
         errorMessage += "setAttribute Error with convertBinary: " + BANDSB::EXISTENCE_MASK + "\n";
      }
   }

   // b31 signals the RADIOMETRIC ADJUSTMENT SURFACE and ATMOSPHERIC ADJUSTMENT ALTITUDE fields.
   if (success && bitTest(existmask, 31))
   {
      readField<string>(input, output, success, BANDSB::RADIOMETRIC_ADJUSTMENT_SURFACE, 24, errorMessage, buf);
      if (success)
      {
         success = readFromStream(input, buf, 4) &&
            output.setAttribute(BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
         if (!success)
         {
            errorMessage += "Read or setAttribute Error with convertBinary: " +
               BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE + "\n";
         }
      }
   }

   // b30 signals the DIAMETER field.
   if (success && bitTest(existmask, 30))
   {
      readField<double>(input, output, success, BANDSB::DIAMETER, 7, errorMessage, buf);
   }

   // b29 signals the DATA_FLD_2 field.
   if (success && bitTest(existmask, 29))
   {
      length = 32;      // binary array of 32 single byte unsigned int's
      success = success && readFromStream(input, buf, length);
      bufUC.resize(length);
      memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
      output.setAttribute(BANDSB::DATA_FLD_2, bufUC);
   }

   // The WAVE_LENGTH_UNIT field can be triggered by any of the following bits:
   // b24 signals the CWAVE and WAVE_LENGTH_UNIT fields.
   // b23 signals the FWHM and WAVE_LENGTH_UNIT fields.
   // b22 signals the FWHM_UNC and WAVE_LENGTH_UNIT fields.
   // b21 signals the NOM_WAVEn and WAVE_LENGTH_UNIT fields.
   // b20 signals the NOM_WAVE_UNCn field and WAVE_LENGTH_UNIT fields.
   // b19 signals the LBOUNDn, UBOUNDn and WAVE_LENGTH_UNIT fields.
   if (success && (bitTest(existmask, 24) || bitTest(existmask, 23) ||
      bitTest(existmask, 22) || bitTest(existmask, 21) || bitTest(existmask, 20) || bitTest(existmask, 19)))
   {
      readField<string>(input, output, success, BANDSB::WAVE_LENGTH_UNIT, 1, errorMessage, buf);
      if (buf[0] == 'W')
      {
         mWavelengthsInInverseCentimeters = true;
      }
   }

   // b24 signals the CWAVE field.
   if (success && bitTest(existmask, 24))
   {
      mCenterWavelengths.resize(count);
   }
   else
   {
      mCenterWavelengths.clear();
   }

   // b23 signals the FWHM field.
   if (success && bitTest(existmask, 23))
   {
      mFwhms.resize(count);
   }
   else
   {
      mFwhms.clear();
   }

   // b19 signals the LBOUNDn, UBOUNDn field.
   if (success && bitTest(existmask, 19))
   {
      mStartWavelengths.resize(count);
      mEndWavelengths.resize(count);
   }
   else
   {
      mStartWavelengths.clear();
      mEndWavelengths.clear();
   }

   for (unsigned int bandNum = 0; bandNum < count; ++bandNum)
   {
      stringstream bandStreamStr;
      bandStreamStr << "#" << bandNum;
      string bandNumStr(bandStreamStr.str());

      string fieldName;

      // b28 flags the BANDIDn field.
      if (success && bitTest(existmask, 28))
      {
         fieldName = BANDSB::BANDID + bandNumStr;
         readField<string>(input, output, success, fieldName, 50, errorMessage, buf);
      }

      // b27 signals the BAD_BANDn field.
      if (success && bitTest(existmask, 27))
      {
         fieldName = BANDSB::BAD_BAND + bandNumStr;
         readField<int>(input, output, success, fieldName, 1, errorMessage, buf);
      }

      // b26 signals the NIIRSn field.
      if (success && bitTest(existmask, 26))
      {
         fieldName = BANDSB::NIIRS + bandNumStr;
         readField<double>(input, output, success, fieldName, 3, errorMessage, buf);
      }

      // b25 signals the FOCAL_LENn field.
      if (success && bitTest(existmask, 25))
      {
         fieldName = BANDSB::FOCAL_LEN + bandNumStr;
         readField<int>(input, output, success, fieldName, 5, errorMessage, buf);
      }

      // b24 signals the CWAVE field.
      if (success && bitTest(existmask, 24))
      {
         fieldName = BANDSB::CWAVE + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
         mCenterWavelengths[bandNum] = boost::lexical_cast<double>(&buf[0]);
      }

      // b23 signals the FWHM field.
      if (success && bitTest(existmask, 23))
      {
         fieldName = BANDSB::FWHM + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
         mFwhms[bandNum] = boost::lexical_cast<double>(&buf[0]);
      }

      // b22 signals the FWHM_UNC field.
      if (success && bitTest(existmask, 22))
      {
         fieldName = BANDSB::FWHM_UNC + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b21 signals the NOM_WAVEn field.
      if (success && bitTest(existmask, 21))
      {
         fieldName = BANDSB::NOM_WAVE + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b20 signals the NOM_WAVE_UNCn field.
      if (success && bitTest(existmask, 20))
      {
         fieldName = BANDSB::NOM_WAVE_UNC + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b19 signals the LBOUNDn, UBOUNDn field.
      if (success && bitTest(existmask, 19))
      {
         fieldName = BANDSB::LBOUND + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
         mStartWavelengths[bandNum] = boost::lexical_cast<double>(&buf[0]);

         fieldName = BANDSB::UBOUND + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
         mEndWavelengths[bandNum] = boost::lexical_cast<double>(&buf[0]);
      }

      // b18 signals the SCALE FACTORn, and ADDITIVE FACTORn fields.
      if (success && bitTest(existmask, 18))
      {
         fieldName = BANDSB::SCALE_FACTOR + bandNumStr;
         if (success)
         {
            success = readFromStream(input, buf, 4) &&
               output.setAttribute(fieldName, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
            if (!success)
            {
               errorMessage += "Read or setAttribute Error with convertBinary: " + fieldName + "\n";
            }
         }

         fieldName = BANDSB::ADDITIVE_FACTOR + bandNumStr;
         if (success)
         {
            success = readFromStream(input, buf, 4) &&
               output.setAttribute(fieldName, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
            if (!success)
            {
               errorMessage += "Read or setAttribute Error with convertBinary: " + fieldName + "\n";
            }
         }
      }

      // b17 signals the START_TIMEn.
      if (success && bitTest(existmask, 17))
      {
         success = success && readFromStream(input, buf, 16);  // YYMMDDhhmmss.sss
         if (!success)
         {
            errorMessage += "reading " + BANDSB::START_TIME + bandNumStr + " failed\n";
         }

         string dtg;
         dtg.resize(16);
         memcpy(&dtg[0], &buf[0], 16);

         unsigned short year(0);
         unsigned short month(0);
         unsigned short day(0);
         unsigned short hour(0);
         unsigned short min(0);
         unsigned short sec(0);
         double frac(0.0);

         // Purposely ignore the return value of bandsbDtgParse since not all values may be set.
         bandsbDtgParse(dtg, year, month, day, hour, min, sec, frac);
         FactoryResource<DateTime> appDTG;
         if (appDTG->set(year, month, day, hour, min, sec) == false)
         {
            errorMessage += "Unable to set " + BANDSB::START_TIME + bandNumStr;
         }
         else
         {
            fieldName = BANDSB::START_TIME + bandNumStr;
            success = success && output.setAttribute(fieldName, *appDTG.get());
            fieldName = BANDSB::START_TIME_FRAC + bandNumStr;
            success = success && output.setAttribute(fieldName, frac);
            if (!success)
            {
               errorMessage += "setAttribute " + BANDSB::START_TIME + bandNumStr + " failed\n";
            }
         }
      }

      // b16 signals the INT_TIMEn field.
      if (success && bitTest(existmask, 16))
      {
         fieldName = BANDSB::INT_TIME + bandNumStr;
         readField<int>(input, output, success, fieldName, 6, errorMessage, buf);
      }

      // b15 signals the CALDRK and CALIBRATION SENSITIVITYn fields.
      if (success && bitTest(existmask, 15))
      {
         fieldName = BANDSB::CALDRK + bandNumStr;
         readField<double>(input, output, success, fieldName, 6, errorMessage, buf);

         fieldName = BANDSB::CALIBRATION_SENSITIVITY + bandNumStr;
         readField<double>(input, output, success, fieldName, 5, errorMessage, buf);
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (success && bitTest(existmask, 14))
      {
         fieldName = BANDSB::ROW_GSD + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
      if (success && bitTest(existmask, 13))
      {
         fieldName = BANDSB::ROW_GSD_UNC + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (success && bitTest(existmask, 14))
      {
         fieldName = BANDSB::ROW_GSD_UNIT + bandNumStr;
         readField<string>(input, output, success, fieldName, 1, errorMessage, buf);

         fieldName = BANDSB::COL_GSD + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
      if (success && bitTest(existmask, 13))
      {
         fieldName = BANDSB::COL_GSD_UNC + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (success && bitTest(existmask, 14))
      {
         fieldName = BANDSB::COL_GSD_UNIT + bandNumStr;
         readField<string>(input, output, success, fieldName, 1, errorMessage, buf);
      }

      // b12 signals the BKNOISEn and SCNNOISEn fields.
      if (success && bitTest(existmask, 12))
      {
         fieldName = BANDSB::BKNOISE + bandNumStr;
         readField<double>(input, output, success, fieldName, 5, errorMessage, buf);

         fieldName = BANDSB::SCNNOISE + bandNumStr;
         readField<double>(input, output, success, fieldName, 5, errorMessage, buf);
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (success && bitTest(existmask, 11))
      {
         fieldName = BANDSB::SPT_RESP_FUNCTION_ROW + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
      if (success && bitTest(existmask, 10))
      {
         fieldName = BANDSB::SPT_RESPUNC_ROW + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (success && bitTest(existmask, 11))
      {
         fieldName = BANDSB::SPT_RESP_UNIT_ROW + bandNumStr;
         readField<string>(input, output, success, fieldName, 1, errorMessage, buf);

         fieldName = BANDSB::SPT_RESP_FUNCTION_COL + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
      if (success && bitTest(existmask, 10))
      {
         fieldName = BANDSB::SPT_RESPUNC_COL + bandNumStr;
         readField<double>(input, output, success, fieldName, 7, errorMessage, buf);
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (success && bitTest(existmask, 11))
      {
         fieldName = BANDSB::SPT_RESP_UNIT_COL + bandNumStr;
         readField<string>(input, output, success, fieldName, 1, errorMessage, buf);
      }

      // b9 signals the DATA_FLD_3n field.
      if (success && bitTest(existmask, 9))
      {
         fieldName = BANDSB::DATA_FLD_3 + bandNumStr;
         length = 16;      // binary array of 16 single byte unsigned int's
         success = success && readFromStream(input, buf, length);
         bufUC.resize(length);
         memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
         output.setAttribute(fieldName, bufUC);
      }

      // b8 signals the DATA_FLD_4n field.
      if (success && bitTest(existmask, 8))
      {
         fieldName = BANDSB::DATA_FLD_4 + bandNumStr;
         length = 24;      // binary array of 24 single byte unsigned int's
         success = success && readFromStream(input, buf, length);
         bufUC.resize(length);
         memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
         output.setAttribute(fieldName, bufUC);
      }

      // b7 signals the DATA_FLD_5n field.
      if (success && bitTest(existmask, 7))
      {
         fieldName = BANDSB::DATA_FLD_5 + bandNumStr;
         length = 32;      // binary array of 32 single byte unsigned int's
         success = success && readFromStream(input, buf, length);
         bufUC.resize(length);
         memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
         output.setAttribute(fieldName, bufUC);
      }

      // b6 signals the DATA_FLD_6n field.
      if (success && bitTest(existmask, 6))
      {
         fieldName = BANDSB::DATA_FLD_6 + bandNumStr;
         length = 48;      // binary array of 48 single byte unsigned int's
         success = success && readFromStream(input, buf, length);
         bufUC.resize(length);
         memcpy(&bufUC[0], &buf[0], length);            // put into vector<unsigned char> for storage in Dynamic Object
         output.setAttribute(fieldName, bufUC);
      }
   }

   // bits: b5 = b4 = b3 = b2 = b1 = 0 (Not used, but present)

   unsigned int num_aux_b = 0;
   unsigned int num_aux_c = 0;

   // b0 = signals the NUM_AUX_B and NUM_AUX_C fields.
   if (success && bitTest(existmask, 0))
   {
      readField<unsigned int>(input, output, success, BANDSB::NUM_AUX_B, 2, errorMessage, buf);
      num_aux_b = QString(&buf.front()).toUInt();

      readField<unsigned int>(input, output, success, BANDSB::NUM_AUX_C, 2, errorMessage, buf);
      num_aux_c = QString(&buf.front()).toUInt();
   }

   for (unsigned int aux_b = 0; aux_b < num_aux_b; ++aux_b)
   {
      stringstream auxbStreamStr;
      auxbStreamStr << "#" << aux_b;
      string auxbStr(auxbStreamStr.str());

      string fieldName;

      fieldName = BANDSB::BAPF + auxbStr;
      readField<string>(input, output, success, fieldName, 1, errorMessage, buf);
      char bapf = buf[0];

      fieldName = BANDSB::UBAP + auxbStr;
      readField<string>(input, output, success, fieldName, 7, errorMessage, buf);
      string ubap(&buf.front());

      for (unsigned int bandNum = 0; bandNum < count; ++bandNum)
      {
         stringstream bandStreamStr;
         bandStreamStr << "#" << bandNum;
         string bandNumStr(bandStreamStr.str());

         switch (bapf)
         {
            case 'I':
            {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
               fieldName = BANDSB::APN + auxbStr + bandNumStr;
               readField<string>(input, output, success, fieldName, 10, errorMessage, buf);
               break;
            }
            case 'R':
            {
               fieldName = BANDSB::APR + auxbStr + bandNumStr;
               if (success)
               {
                  success = readFromStream(input, buf, 4) &&
                     output.setAttribute(fieldName, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
                  if (!success)
                  {
                     errorMessage += "Read or setAttribute Error with convertBinary: " + fieldName + "\n";
                  }
               }
               break;
            }
            case 'A':
            {
               fieldName = BANDSB::APA + auxbStr + bandNumStr;
               readField<string>(input, output, success, fieldName, 20, errorMessage, buf);
               break;
            }
            default:
            {
               // This is an error and should never happen with a proper BANDSB TAG
               success = false;
               errorMessage += "Parsing error in bapf: aux_b " + BANDSB::BAPF +
                  auxbStr + ":" + bandNumStr + ":" + bapf;
               break;
            }
         }
      }
   }

   for (unsigned int aux_c = 0; aux_c < num_aux_c; ++aux_c)
   {
      stringstream auxcStreamStr;
      auxcStreamStr << "#" << aux_c;
      string auxcStr(auxcStreamStr.str());

      string fieldName;

      fieldName = BANDSB::CAPF + auxcStr;
      readField<string>(input, output, success, fieldName, 1, errorMessage, buf);
      char capf = buf[0];

      fieldName = BANDSB::UCAP + auxcStr;
      readField<string>(input, output, success, fieldName, 7, errorMessage, buf);
      string ubap(&buf.front());

      switch (capf)
      {
         case 'I':
         {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
            fieldName = BANDSB::APN + auxcStr;
            readField<string>(input, output, success, fieldName, 10, errorMessage, buf);
            break;
         }
         case 'R':
         {
            fieldName = BANDSB::APR + auxcStr;
            if (success)
            {
               success = readFromStream(input, buf, 4) &&
                  output.setAttribute(fieldName, convertBinary<float>(&buf[0], BIG_ENDIAN_ORDER));
               if (!success)
               {
                  errorMessage += "Read or setAttribute Error with convertBinary: " + fieldName + "\n";
               }
            }
            break;
         }
         case 'A':
         {
            fieldName = BANDSB::APA + auxcStr;
            readField<string>(input, output, success, fieldName, 20, errorMessage, buf);
            break;
         }
         default:
         {
            // This is an error and should never happen with a proper BANDSB TAG
            success = false;
            errorMessage += "Parsing error in capf: aux_c " + BANDSB::CAPF + auxcStr + ":" + capf;
            break;
         }
      }
   }

   int numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

Nitf::TreState Nitf::BandsbParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{

   // Temp variables of different types to hold return values
   string                  testString;

   vector<char>            buf;             // Input buffer
   vector<unsigned char>   bufUC;  // buffer for array of unsigned char
   vector<unsigned int>    bufInt; // buffer for array of int
   set<string>             testSet;

   TreState         status(VALID);
   unsigned int numFields(0);

   unsigned int count(0);
   try
   {
      count = dv_cast<unsigned int>(tre.getAttribute(BANDSB::COUNT));
      numFields++;
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << "COUNT" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   status = MaxState(status, testTagValueRange(reporter, count, 1U, 9999U));

   testSet.clear();
   testSet.insert("REFLECTANCE");
   testSet.insert("EMITTANCE");
   testSet.insert("EMISSIVITY");
   testSet.insert("RADIANCE");
   testSet.insert("IRRADIANCE");
   testSet.insert("KINETIC TEMPERATURE");
   testSet.insert("RADIANT TEMPERATURE");
   testSet.insert("THERMAL INERTIA");
   testSet.insert("APPARENT THERMAL INERTIA");
   testSet.insert("RADIANT FLUX");
   testSet.insert("UNCALIBRATED");
   testSet.insert("RAW");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RADIOMETRIC_QUANTITY", testSet, true, true, true));

   testSet.clear();
   testSet.insert("F");
   testSet.insert("L");
   testSet.insert("M");
   testSet.insert("S");
   testSet.insert("P");
   testSet.insert("K");
   testSet.insert("I");
   testSet.insert("V");
   testSet.insert("N");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RADIOMETRIC_QUANTITY_UNIT", testSet, true, true, true));

   // Just make sure these fields exist no checking needed
   // These fields comes in as binary float, unlimited range
   if (tre.getAttribute(BANDSB::SCALE_FACTOR).getPointerToValue<float>() == NULL)
   {
      reporter << "Field \"" << "SCALE_FACTOR" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   if (tre.getAttribute(BANDSB::ADDITIVE_FACTOR).getPointerToValue<float>() == NULL)
   {
      reporter << "Field \"" << "ADDITIVE_FACTOR" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   // This field does not have to exist but if it does then it must be in range.
   try
   {
      float rowGsd = dv_cast<float>(tre.getAttribute(BANDSB::ROW_GSD));
      status = MaxState(status, testTagValueRange(reporter, rowGsd, 0.001F, 9999.99F));
      ++numFields;
   }
   catch (const bad_cast&)
   {
      // Do nothing
   }


   testSet.clear();
   testSet.insert("M");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "ROW_GSD_UNIT", testSet, true, false, true));

   // This field does not have to exist but if it does then it must be in range.
   try
   {
      float colGsd = dv_cast<float>(tre.getAttribute(BANDSB::COL_GSD));
      status = MaxState(status, testTagValueRange(reporter, colGsd, 0.001F, 9999.99F));
      ++numFields;
   }
   catch (const bad_cast&)
   {
      // Do nothing
   }

   testSet.clear();
   testSet.insert("M");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "COL_GSD_UNITS", testSet, true, false, true));

   // This field does not have to exist but if it does then it must be in range.
   try
   {
      float sptRespRow = dv_cast<float>(tre.getAttribute(BANDSB::SPT_RESP_ROW));
      status = MaxState(status, testTagValueRange(reporter, sptRespRow, 0.001F, 9999.99F));
      ++numFields;
   }
   catch (const bad_cast&)
   {
      // Do nothing
   }

   testSet.clear();
   testSet.insert("M");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "SPT_RESP_UNIT_ROW", testSet, true, false, true));

   // This field does not have to exist but if it does then it must be in range.
   try
   {
      float sptRespCol = dv_cast<float>(tre.getAttribute(BANDSB::SPT_RESP_COL));
      status = MaxState(status, testTagValueRange(reporter, sptRespCol, 0.001F, 9999.99F));
      ++numFields;
   }
   catch (const bad_cast&)
   {
      // Do nothing
   }

   testSet.clear();
   testSet.insert("M");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "SPT_RESP_UNIT_COL", testSet, true, false, true));

   // binary array of 48 single byte unsigned int's
   // Just test that it exists. We don't need the values.
   if (tre.getAttribute(BANDSB::DATA_FLD_1).getPointerToValue<vector<unsigned char> >() == NULL)
   {
      reporter << "Field \"" << "DATA_FLD_1" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   else
   {
      ++numFields;
   }

   // Bit-wise Existence Mask Field 32 flags represented by the bits of this 4
   // byte (32 bit) field. (b31, b30, b29, b28 ... b2, b1, b0)

   // A bit set to zero signals that a conditional field is not present in this
   // extension. A bit set to the value one indicates the inclusion of the conditional field.
   unsigned int existmask = 0;
   try
   {
      existmask = dv_cast<unsigned int>(tre.getAttribute(BANDSB::EXISTENCE_MASK));
      ++numFields;
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << "EXISTENCE_MASK" << "\" missing from the Dynamic Object";
      status = INVALID;
   }
   status = MaxState(status, testTagValueRange(reporter, existmask, 0x00000001U, 0xFFFFFFC1U));


   // b31 signals the RADIOMETRIC ADJUSTMENT SURFACE and ATMOSPHERIC ADJUSTMENT ALTITUDE fields.
   if (status != INVALID && bitTest(existmask, 31))
   {
      testSet.clear();
      testSet.insert("DETECTOR");
      testSet.insert("APERTURE FOCAL PLANE");
      testSet.insert("TOP OF ATMOSPHERE");
      testSet.insert("WITHIN ATMOSPHERE");
      testSet.insert("EARTH SURFACE");
      status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields,
         "RADIOMETRIC_ADJUSTMENT_SURFACE", testSet, false, true, true));

      // comes in as binary float, unlimited range
      if (tre.getAttribute(BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE).getPointerToValue<float>() == NULL)
      {
         reporter << "Field \"" << "ATMOSPHERIC_ADJUSTMENT_ALTITUDE" << "\" missing from the Dynamic Object";
         status = INVALID;
      }
      else
      {
         ++numFields;
      }
   }

   // b30 signals the DIAMETER field.
   if (status != INVALID && bitTest(existmask, 30))
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "DIAMETER", 0.01F, 8999.99F));
   }

   // b29 signals the DATA_FLD_2 field.
   if (status != INVALID && bitTest(existmask, 29))
   {
      // binary array of 32 single byte unsigned int's
      // Just test that it exists. We don't need the values.
      if (tre.getAttribute(BANDSB::DATA_FLD_2).getPointerToValue<vector<unsigned char> >() == NULL)
      {
         reporter << "Field \"" << "DATA_FLD_2" << "\" missing from the Dynamic Object";
         status = INVALID;
      }
      ++numFields;
   }

   // The WAVE_LENGTH_UNIT field can be triggered by any of the following bits:
   // b24 signals the CWAVE and WAVE_LENGTH_UNIT fields.
   // b23 signals the FWHM and WAVE_LENGTH_UNIT fields.
   // b22 signals the FWHM_UNC and WAVE_LENGTH_UNIT fields.
   // b21 signals the NOM_WAVEn and WAVE_LENGTH_UNIT fields.
   // b20 signals the NOM_WAVE_UNCn and WAVE_LENGTH_UNIT fields.
   // b19 signals the LBOUNDn, UBOUNDn and WAVE_LENGTH_UNIT fields.
   if (status != INVALID && (bitTest(existmask, 24) || bitTest(existmask, 23) || bitTest(existmask, 22) ||
      bitTest(existmask, 21) || bitTest(existmask, 20) || bitTest(existmask, 19)))
   {
      testSet.clear();
      testSet.insert("U");
      testSet.insert("W");
                                    // all blank and extra strings in set not allowed
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "WAVE_LENGTH_UNIT", testSet, false, false));
   }

   // b24 signals the CWAVE field.
   if (status != INVALID)
   {
      if (bitTest(existmask, 24))
      {
         if (mCenterWavelengths.size() != count)
         {
            status = INVALID;
         }
      }
      else if (mCenterWavelengths.empty() == false)
      {
         status = INVALID;
      }
   }

   // b23 signals the FWHM field.
   if (status != INVALID)
   {
      if (bitTest(existmask, 23))
      {
         if (mFwhms.size() != count)
         {
            status = INVALID;
         }
      }
      else if (mFwhms.empty() == false)
      {
         status = INVALID;
      }
   }

   // b19 signals the LBOUNDn, UBOUNDn field.
   if (status != INVALID)
   {
      if (bitTest(existmask, 19))
      {
         if (mStartWavelengths.size() != count || mEndWavelengths.size() != count)
         {
            status = INVALID;
         }
      }
      else if (mStartWavelengths.empty() == false || mEndWavelengths.empty() == false)
      {
         status = INVALID;
      }
   }

   for (unsigned int bandNum = 0; (bandNum < count) && status != INVALID; ++bandNum)
   {
      stringstream bandStreamStr;
      bandStreamStr << "#" << bandNum;
      string bandNumStr(bandStreamStr.str());

      string fieldName;

      // b28 flags the BANDIDn field.
      if (status != INVALID && bitTest(existmask, 28))
      {
         fieldName = BANDSB::BANDID + bandNumStr;
         testSet.clear();              // no items in list, field is user defined
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, false, true, false));
      }

      // b27 signals the BAD_BANDn field.
      if (status != INVALID && bitTest(existmask, 27))
      {
         fieldName = BANDSB::BAD_BAND + bandNumStr;
         status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, fieldName, 0, 1));
      }

      // b26 signals the NIIRSn field.
      if (status != INVALID && bitTest(existmask, 26))
      {
         fieldName = BANDSB::NIIRS + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.1, 9.9));
      }

      // b25 signals the FOCAL_LENn field.
      if (status != INVALID && bitTest(existmask, 25))
      {
         fieldName = BANDSB::FOCAL_LEN + bandNumStr;
         status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, fieldName, 1, 99999));
      }

      // b24 signals the CWAVE field.
      if (status != INVALID && bitTest(existmask, 24))
      {
         fieldName = BANDSB::CWAVE + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b23 signals the FWHM field.
      if (status != INVALID && bitTest(existmask, 23))
      {
         fieldName = BANDSB::FWHM + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b22 signals the FWHM_UNC field.
      if (status != INVALID && bitTest(existmask, 22))
      {
         fieldName = BANDSB::FWHM_UNC + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b21 signals the NOM_WAVEn field.
      if (status != INVALID && bitTest(existmask, 21))
      {
         fieldName = BANDSB::NOM_WAVE + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b20 signals the NOM_WAVE_UNCn field.
      if (status != INVALID && bitTest(existmask, 20))
      {
         fieldName = BANDSB::NOM_WAVE_UNC + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b19 signals the LBOUNDn, UBOUNDn field.
      if (status != INVALID && bitTest(existmask, 19))
      {
         fieldName = BANDSB::LBOUND + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));

         fieldName = BANDSB::UBOUND + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.00001, 10000.0));
      }

      // b18 signals the SCALE FACTORn, and ADDITIVE FACTORn fields.
      if (status != INVALID && bitTest(existmask, 18))
      {
         fieldName = BANDSB::SCALE_FACTOR + bandNumStr;
         if (tre.getAttribute(fieldName).getPointerToValue<float>() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }

         fieldName = BANDSB::ADDITIVE_FACTOR + bandNumStr;
         if (status != INVALID)
         {
            if (tre.getAttribute(fieldName).getPointerToValue<float>() == NULL)
            {
               reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
               status = INVALID;
            }
            else
            {
               ++numFields;
            }
         }
      }

      // b17 signals the START_TIMEn.
      if (status != INVALID && bitTest(existmask, 17))
      {
         // Just check that the field exists. Assume the DateTime class will contain a valid date/time
         fieldName = BANDSB::START_TIME + bandNumStr;

         if (tre.getAttribute(fieldName).getPointerToValue<DateTime>() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }

         fieldName = BANDSB::START_TIME_FRAC + bandNumStr;

         const double* frac = NULL;

         DataVariant tempDV = tre.getAttribute(fieldName);

         if (tempDV.isValid())
         {
            frac = dv_cast<double>(&tempDV);
            ++numFields;
         }
         else
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
      }

       // b16 signals the INT_TIMEn field.
      if (status != INVALID && bitTest(existmask, 16))
      {
         fieldName = BANDSB::INT_TIME + bandNumStr;
         status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, fieldName, 1, 999999));
      }

      // b15 signals the CALDRK and CALIBRATION SENSITIVITYn fields.
      if (status != INVALID && bitTest(existmask, 15))
      {
         fieldName = BANDSB::CALDRK + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.0, 9999.9));

         fieldName = BANDSB::CALIBRATION_SENSITIVITY + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.01, 100.0));
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (status != INVALID && bitTest(existmask, 14))
      {
         fieldName = BANDSB::ROW_GSD + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.0, 9999.99));
      }

      // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
      if (status != INVALID && bitTest(existmask, 13))
      {
         fieldName = BANDSB::ROW_GSD_UNC + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.001, 9999.99));
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (status != INVALID && bitTest(existmask, 14))
      {
         testSet.clear();
         testSet.insert("M");
         testSet.insert("R");
                                    // all blank and extra strings in set not allowed
         fieldName = BANDSB::ROW_GSD_UNIT + bandNumStr;
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, false, false, true));

         fieldName = BANDSB::COL_GSD + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.01, 9999.99));
      }

      // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
      if (status != INVALID && bitTest(existmask, 13))
      {
         fieldName = BANDSB::COL_GSD_UNC + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.001, 9999.99));
      }

      // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
      if (status != INVALID && bitTest(existmask, 14))
      {
         testSet.clear();
         testSet.insert("M");
         testSet.insert("R");
                                    // all blank and extra strings in set not allowed
         fieldName = BANDSB::COL_GSD_UNIT + bandNumStr;
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, false, false, true));
      }

      // b12 signals the BKNOISEn and SCNNOISEn fields.
      if (status != INVALID && bitTest(existmask, 12))
      {
         fieldName = BANDSB::BKNOISE + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.0, 99.99));

         fieldName = BANDSB::SCNNOISE + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.0, 99.99));
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (status != INVALID && bitTest(existmask, 11))
      {
         fieldName = BANDSB::SPT_RESP_FUNCTION_ROW + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.001, 9999.99));
      }

      // b10 signals the SPT_RESP UNC_ROWn and SPT_RESP UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
      if (status != INVALID && bitTest(existmask, 10))
      {
         fieldName = BANDSB::SPT_RESPUNC_ROW + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, fieldName, 0.001, 9999.99));
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (status != INVALID && bitTest(existmask, 11))
      {
         testSet.clear();
         testSet.insert("M");
         testSet.insert("R");
                                 // all blank and extra strings in set not allowed
         fieldName = BANDSB::SPT_RESP_UNIT_ROW + bandNumStr;
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, false, false, true));

         fieldName = BANDSB::SPT_RESP_FUNCTION_COL + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.001, 9999.99));
      }

      // b10 signals the SPT_RESP UNC_ROWn and SPT_RESP UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
      if (status != INVALID && bitTest(existmask, 10))
      {
         fieldName = BANDSB::SPT_RESPUNC_COL + bandNumStr;
         status = MaxState(status, testTagValueRange<double>(tre, reporter,
            &numFields, fieldName, 0.001, 9999.99));
      }

      // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn, SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
      if (status != INVALID && bitTest(existmask, 11))
      {
         testSet.clear();
         testSet.insert("M");
         testSet.insert("R");
                                    // all blank and extra strings in set not allowed
         fieldName = BANDSB::SPT_RESP_UNIT_COL + bandNumStr;
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, false, false, true));
      }

      // b9 signals the DATA_FLD_3n field.
      if (status != INVALID && bitTest(existmask, 9))
      {
         // binary array of 16 single byte unsigned int's
         // Just test that it exists. We don't need the values.
         fieldName = BANDSB::DATA_FLD_3 + bandNumStr;
         if (tre.getAttribute(fieldName).getPointerToValue<vector<unsigned char> >() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }
      }

      // b8 signals the DATA_FLD_4n field.
      if (status != INVALID && bitTest(existmask, 8))
      {
         // binary array of 24 single byte unsigned int's
         // Just test that it exists. We don't need the values.
         fieldName = BANDSB::DATA_FLD_4 + bandNumStr;
         if (tre.getAttribute(fieldName).getPointerToValue<vector<unsigned char> >() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }
      }

      // b7 signals the DATA_FLD_5n field.
      if (status != INVALID && bitTest(existmask, 7))
      {
         // binary array of 32 single byte unsigned int's
         // Just test that it exists. We don't need the values.
         fieldName = BANDSB::DATA_FLD_5 + bandNumStr;
         if (tre.getAttribute(fieldName).getPointerToValue<vector<unsigned char> >() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }
      }

      // b6 signals the DATA_FLD_6n field.
      if (status != INVALID && bitTest(existmask, 6))
      {
         // binary array of 48 single byte unsigned int's
         // Just test that it exists. We don't need the values.
         fieldName = BANDSB::DATA_FLD_6 + bandNumStr;
         if (tre.getAttribute(fieldName).getPointerToValue<vector<unsigned char> >() == NULL)
         {
            reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
            status = INVALID;
         }
         else
         {
            ++numFields;
         }
      }
   }

   // bits: b5 = b4 = b3 = b2 = b1 = 0 (Not used, but present)

   unsigned int num_aux_b = 0;
   unsigned int num_aux_c = 0;

   // b0 = signals the NUM_AUX_B and NUM_AUX_C fields.
   if (status != INVALID && bitTest(existmask, 0))
   {
      try
      {
         num_aux_b = dv_cast<unsigned int>(tre.getAttribute(BANDSB::NUM_AUX_B));
         ++numFields;
      }
      catch (const bad_cast&)
      {
         reporter << "Field \"" << "NUM_AUX_B" << "\" missing from the Dynamic Object";
         status = INVALID;
      }

      if (status != INVALID)
      {
         status = MaxState(status, testTagValueRange<unsigned int>(reporter, num_aux_b, 0U, 99U));
      }

      if (status != INVALID)
      {
         try
         {
            num_aux_c = dv_cast<unsigned int>(tre.getAttribute(BANDSB::NUM_AUX_C));
            ++numFields;
         }
         catch (const bad_cast&)
         {
            reporter << "Field \"" << "NUM_AUX_C" << "\" missing from the Dynamic Object";
            status = INVALID;
         }
      }
      if (status != INVALID)
      {
         status = MaxState(status, testTagValueRange<unsigned int>(reporter, num_aux_c, 0U, 99U));
      }
   }

   for (unsigned int aux_b = 0; (aux_b < num_aux_b) && status != INVALID; ++aux_b)
   {
      stringstream auxbStreamStr;
      auxbStreamStr << "#" << aux_b;
      string auxbStr(auxbStreamStr.str());

      string fieldName;

      fieldName = BANDSB::BAPF + auxbStr;
      testSet.clear();
      testSet.insert("I");
      testSet.insert("R");
      testSet.insert("A");
                                    // all blank and extra strings in set not allowed
      string bapf;
      try
      {
         bapf = dv_cast<string>(tre.getAttribute(fieldName));
         ++numFields;
      }
      catch (const bad_cast&)
      {
         reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
         status = INVALID;
      }

      status = MaxState(status, testTagValidBcsASet(bapf, reporter, testSet, false, false, true));

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Test against DIGEST Edition 2.1, Part 3-7 (lbeck)")
      fieldName = BANDSB::UBAP + auxbStr;
      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, fieldName, testSet, true, true, false));

      for (unsigned int bandNum = 0; (bandNum < count) && status != INVALID; ++bandNum)
      {
         stringstream bandStreamStr;
         bandStreamStr << "#" << bandNum;
         string bandNumStr(bandStreamStr.str());

         switch (*(bapf.c_str()))
         {
            case 'I':
            {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
               fieldName = BANDSB::APN + auxbStr + bandNumStr;
               testSet.clear();
               // Don't need to test range because it is min to max int.
               status = MaxState(status, testTagValidBcsASet(tre, reporter,
                  &numFields, fieldName, testSet, true, true, false));
               break;
            }
            case 'R':
            {
               fieldName = BANDSB::APR + auxbStr + bandNumStr;
               if (tre.getAttribute(fieldName).getPointerToValue<float>() == NULL)
               {
                  reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
                  status = INVALID;
               }
               else
               {
                  ++numFields;
               }

               break;
            }
            case 'A':
            {
               fieldName = BANDSB::APA + auxbStr + bandNumStr;
               testSet.clear();
               status = MaxState(status, testTagValidBcsASet(tre, reporter,
                  &numFields, fieldName, testSet, true, true, false));
               break;
            }
            default:
            {
               // This is an error and should never happen with a proper BANDSB TAG
               status = INVALID;
               break;
            }
         }
      }
   }

   for (unsigned int aux_c = 0; (aux_c < num_aux_c) && status != INVALID; ++aux_c)
   {
      stringstream auxcStreamStr;
      auxcStreamStr << "#" << aux_c;
      string auxcStr(auxcStreamStr.str());

      string fieldName;

      fieldName = BANDSB::CAPF + auxcStr;
      testSet.clear();
      testSet.insert("I");
      testSet.insert("R");
      testSet.insert("A");
                                    // all blank and extra strings in set not allowed
      string capf;
      try
      {
         capf = dv_cast<string>(tre.getAttribute(fieldName));
         ++numFields;
      }
      catch (const bad_cast&)
      {
         reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
         status = INVALID;
      }
      status = MaxState(status, testTagValidBcsASet(capf, reporter, testSet, false, false, true));

      if (status != INVALID)
      {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Test against DIGEST Edition 2.1, Part 3-7 (lbeck)")
         fieldName = BANDSB::UCAP + auxcStr;
         testSet.clear();
         status = MaxState(status, testTagValidBcsASet(tre, reporter,
            &numFields, fieldName, testSet, true, true, false));
      }

      switch (*(capf.c_str()))
      {
         case 'I':
         {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
            fieldName = BANDSB::APN + auxcStr;
            testSet.clear();
            // Don't need to test range because it is min to max int.
            status = MaxState(status, testTagValidBcsASet(tre, reporter,
               &numFields, fieldName, testSet, true, true, false));
            break;
         }
         case 'R':
         {
            fieldName = BANDSB::APR + auxcStr;
            if (tre.getAttribute(fieldName).getPointerToValue<float>() == NULL)
            {
               reporter << "Field \"" << fieldName << "\" missing from the Dynamic Object";
               status = INVALID;
            }
            else
            {
               ++numFields;
            }
            break;
         }
         case 'A':
         {
            fieldName = BANDSB::APA + auxcStr;
            testSet.clear();
            status = MaxState(status, testTagValidBcsASet(tre, reporter,
               &numFields, fieldName, testSet, true, true, false));
            break;
         }
         default:
         {
            status = INVALID;
            break;
         }
      }
   }

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the BANDSB TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the BANDSB TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::BandsbParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      // BANDSB NITF data is always Big Endian

      vector<char> buf;             // Input buffer
      vector<unsigned char> bufUC;  // buffer for array of unsigned char
      vector<unsigned int>  bufInt; // buffer for array of int
      unsigned int length;
      float tempFloat;
      bool ok(true);

      string dashes7("-------");         // Unknown numeric are 7 char of "-"
      unsigned int count =
         dv_cast<unsigned int>(input.getAttribute(BANDSB::COUNT));      // COUNT == the number of bands in the cube
      output << toString(count, 5);

      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::RADIOMETRIC_QUANTITY)), 24);
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::RADIOMETRIC_QUANTITY_UNIT)), 1);

      tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(BANDSB::SCALE_FACTOR)), BIG_ENDIAN_ORDER);
      output.write(reinterpret_cast<char*>(&tempFloat), 4);

      tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(BANDSB::ADDITIVE_FACTOR)), BIG_ENDIAN_ORDER);
      output.write(reinterpret_cast<char*>(&tempFloat), 4);

      DataVariant tempDV = input.getAttribute(BANDSB::ROW_GSD);
      if (tempDV.isValid())
      {
         output << toString(dv_cast<float>(tempDV), 7, 3);
      }
      else
      {
         output << dashes7;
      }
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::ROW_GSD_UNIT)), 1);

      tempDV = input.getAttribute(BANDSB::COL_GSD);
      if (tempDV.isValid())
      {
         output << toString(dv_cast<float>(tempDV), 7, 3);
      }
      else
      {
         output << dashes7;
      }
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::COL_GSD_UNITS)), 1);

      tempDV = input.getAttribute(BANDSB::SPT_RESP_ROW);
      if (tempDV.isValid())
      {
         output << toString(dv_cast<float>(tempDV), 7, 3);
      }
      else
      {
         output << dashes7;
      }
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::SPT_RESP_UNIT_ROW)), 1);

      tempDV = input.getAttribute(BANDSB::SPT_RESP_COL);
      if (tempDV.isValid())
      {
         output << toString(dv_cast<float>(tempDV), 7, 3);
      }
      else
      {
         output << dashes7;
      }
      output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::SPT_RESP_UNIT_COL)), 1);


      length = 48;      // binary array of 48 single byte unsigned int's
      bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(BANDSB::DATA_FLD_1));
      if (bufUC.size() != length)
      {
         cerr << " Error writing " << BANDSB::DATA_FLD_1 << 
            " size == " << buf.size() << " should have been " << length << endl;
      }
      buf.resize(length);
      memcpy(&buf[0], &bufUC[0], length);

      output.write(&buf[0], length);

      // Bit-wise Existence Mask Field 32 flags represented by the bits of this 4
      // byte (32 bit) field. (b31, b30, b29, b28 ... b2, b1, b0)

      // A bit set to zero signals that a conditional field is not present in this
      // extension. A bit set to the value one indicates the inclusion of the conditional field.
      unsigned int existmask = dv_cast<unsigned int>(input.getAttribute(BANDSB::EXISTENCE_MASK));
      unsigned int tempUInt = convertBinary<unsigned int>(&existmask, BIG_ENDIAN_ORDER);
      output.write(reinterpret_cast<char*>(&tempUInt), 4);

      // b31 signals the RADIOMETRIC ADJUSTMENT SURFACE and ATMOSPHERIC ADJUSTMENT ALTITUDE fields.
      if (bitTest(existmask, 31))
      {
         output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::RADIOMETRIC_ADJUSTMENT_SURFACE)), 24);

         tempFloat = convertBinary<float>(&dv_cast<float>
            (input.getAttribute(BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE)), BIG_ENDIAN_ORDER);
         output.write(reinterpret_cast<char*>(&tempFloat), 4);
      }

      // b30 signals the DIAMETER field.
      if (bitTest(existmask, 30))
      {
         output << toString(dv_cast<double>(input.getAttribute(BANDSB::DIAMETER)), 7, 2);
      }

      // b29 signals the DATA_FLD_2 field.
      if (bitTest(existmask, 29))
      {
         length = 32;      // binary array of 32 single byte unsigned int's
         bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(BANDSB::DATA_FLD_2));
         if (bufUC.size() != length)
         {
            cerr << " Error writing " << BANDSB::DATA_FLD_2 <<
               " size == " << buf.size() << " should have been " << length << endl;
         }
         buf.resize(length);
         memcpy(&buf[0], &bufUC[0], length);

         output.write(&buf[0], length);
      }

      // The WAVE_LENGTH_UNIT field can be triggered by any of the following bits:
      // b24 signals the CWAVE and WAVE_LENGTH_UNIT fields.
      // b23 signals the FWHM and WAVE_LENGTH_UNIT fields.
      // b22 signals the FWHM_UNC and WAVE_LENGTH_UNIT fields.
      // b21 signals the NOM_WAVEn and WAVE_LENGTH_UNIT fields.
      // b20 signals the NOM_WAVE_UNCn and WAVE_LENGTH_UNIT fields.
      // b19 signals the LBOUNDn, UBOUNDn and WAVE_LENGTH_UNIT fields.
      if (bitTest(existmask, 24) || bitTest(existmask, 23) || bitTest(existmask, 22) ||
         bitTest(existmask, 21) || bitTest(existmask, 20) || bitTest(existmask, 19))
      {
         output << sizeString(dv_cast<string>(input.getAttribute(BANDSB::WAVE_LENGTH_UNIT)), 1);
      }

      for (unsigned int bandNum = 0; bandNum < count; ++bandNum)
      {
         stringstream bandStreamStr;
         bandStreamStr << "#" << bandNum;
         string bandNumStr(bandStreamStr.str());

         string fieldName;

         // b28 flags the BANDIDn field.
         if (bitTest(existmask, 28))
         {
            fieldName = BANDSB::BANDID + bandNumStr;
            output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 50);
         }

         // b27 signals the BAD_BANDn field.
         if (bitTest(existmask, 27))
         {
            fieldName = BANDSB::BAD_BAND + bandNumStr;
            output << toString(dv_cast<int>(input.getAttribute(fieldName)), 1);
         }

         // b26 signals the NIIRSn field.
         if (bitTest(existmask, 26))
         {
            fieldName = BANDSB::NIIRS + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 3);
         }

         // b25 signals the FOCAL_LENn field.
         if (bitTest(existmask, 25))
         {
            fieldName = BANDSB::FOCAL_LEN + bandNumStr;
            output << toString(dv_cast<int>(input.getAttribute(fieldName)), 5);
         }

         // b24 signals the CWAVE field.
         if (bitTest(existmask, 24))
         {
            fieldName = BANDSB::CWAVE + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b23 signals the FWHM field.
         if (bitTest(existmask, 23))
         {
            fieldName = BANDSB::FWHM + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b22 signals the FWHM_UNC field.
         if (bitTest(existmask, 22))
         {
            fieldName = BANDSB::FWHM_UNC + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b21 signals the NOM_WAVEn field.
         if (bitTest(existmask, 21))
         {
            fieldName = BANDSB::NOM_WAVE + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b20 signals the NOM_WAVE_UNCn field.
         if (bitTest(existmask, 20))
         {
            fieldName = BANDSB::NOM_WAVE_UNC + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b19 signals the LBOUNDn, UBOUNDn field.
         if (bitTest(existmask, 19))
         {
            fieldName = BANDSB::LBOUND + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);

            fieldName = BANDSB::UBOUND + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7);
         }

         // b18 signals the SCALE FACTORn, and ADDITIVE FACTORn fields.
         if (bitTest(existmask, 18))
         {
            fieldName = BANDSB::SCALE_FACTOR + bandNumStr;
            tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(fieldName)), BIG_ENDIAN_ORDER);
            output.write(reinterpret_cast<char*>(&tempFloat), 4);

            fieldName = BANDSB::ADDITIVE_FACTOR + bandNumStr;
            tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(fieldName)), BIG_ENDIAN_ORDER);
            output.write(reinterpret_cast<char*>(&tempFloat), 4);
         }


         // b17 signals the START_TIMEn.
         if (bitTest(existmask, 17))
         {
            string YYMMDDhhmmssSSS("------------.---");
            fieldName = BANDSB::START_TIME + bandNumStr;
            const DateTime* pStartDtg = dv_cast<DateTime>(&input.getAttribute(fieldName));
            if (pStartDtg != NULL)
            {
               fieldName = BANDSB::START_TIME_FRAC + bandNumStr;
               const double* pFrac = dv_cast<double>(&input.getAttribute(fieldName));
               if (pFrac != NULL)
               {
                  // put date in form YYMMDDhhmmss.sss for this TAG    see: strftime() for format info
                  string fracStr = toString<double>(*pFrac, 5);
                  fracStr = fracStr.substr(1);                    // trim off leading zero

                  YYMMDDhhmmssSSS = pStartDtg->getFormattedUtc("%y%m%d%H%M%S") + fracStr;
               }
            }

            output << sizeString(YYMMDDhhmmssSSS, 16);
         }

         // b16 signals the INT_TIMEn field.
         if (bitTest(existmask, 16))
         {
            fieldName = BANDSB::INT_TIME + bandNumStr;
            output << toString(dv_cast<int>(input.getAttribute(fieldName)), 6);
         }

         // b15 signals the CALDRK and CALIBRATION SENSITIVITYn fields.
         if (bitTest(existmask, 15))
         {
            fieldName = BANDSB::CALDRK + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 6, 1);

            fieldName = BANDSB::CALIBRATION_SENSITIVITY + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2);
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existmask, 14))
         {
            fieldName = BANDSB::ROW_GSD + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 2);
         }

         // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
         if (bitTest(existmask, 13))
         {
            fieldName = BANDSB::ROW_GSD_UNC + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existmask, 14))
         {
            fieldName = BANDSB::ROW_GSD_UNIT + bandNumStr;
            output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 1);

            fieldName = BANDSB::COL_GSD + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 2);
         }

         // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
         if (bitTest(existmask, 13))
         {
            fieldName = BANDSB::COL_GSD_UNC + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existmask, 14))
         {
            fieldName = BANDSB::COL_GSD_UNIT + bandNumStr;
            output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 1);
         }

         // b12 signals the BKNOISEn and SCNNOISEn fields.
         if (bitTest(existmask, 12))
         {
            fieldName = BANDSB::BKNOISE + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2);

            fieldName = BANDSB::SCNNOISE + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 5, 2);
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existmask, 11))
         {
            fieldName = BANDSB::SPT_RESP_FUNCTION_ROW + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
         if (bitTest(existmask, 10))
         {
            fieldName = BANDSB::SPT_RESPUNC_ROW + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existmask, 11))
         {
            fieldName = BANDSB::SPT_RESP_UNIT_ROW + bandNumStr;
            output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 1);

            fieldName = BANDSB::SPT_RESP_FUNCTION_COL + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
         if (bitTest(existmask, 10))
         {
            fieldName = BANDSB::SPT_RESPUNC_COL + bandNumStr;
            output << toString(dv_cast<double>(input.getAttribute(fieldName)), 7, 3);
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existmask, 11))
         {
            fieldName = BANDSB::SPT_RESP_UNIT_COL + bandNumStr;
            output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 1);
         }

         // b9 signals the DATA_FLD_3n field.
         if (bitTest(existmask, 9))
         {
            fieldName = BANDSB::DATA_FLD_3 + bandNumStr;
            length = 16;      // binary array of 16 single byte unsigned int's

            bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(fieldName));
            if (bufUC.size() != length)
            {
               cerr << " Error writing " << fieldName <<
                  " size == " << buf.size() << " should have been " << length << endl;
            }
            buf.resize(length);
            memcpy(&buf[0], &bufUC[0], length);

            output.write(&buf[0], length);
         }

         // b8 signals the DATA_FLD_4n field.
         if (bitTest(existmask, 8))
         {
            fieldName = BANDSB::DATA_FLD_4 + bandNumStr;
            length = 24;      // binary array of 24 single byte unsigned int's

            bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(fieldName));
            if (bufUC.size() != length)
            {
               cerr << " Error writing " << fieldName <<
                  " size == " << buf.size() << " should have been " << length << endl;
            }
            buf.resize(length);
            memcpy(&buf[0], &bufUC[0], length);

            output.write(&buf[0], length);
         }

         // b7 signals the DATA_FLD_5n field.
         if (bitTest(existmask, 7))
         {
            fieldName = BANDSB::DATA_FLD_5 + bandNumStr;
            length = 32;      // binary array of 32 single byte unsigned int's

            bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(fieldName));
            if (bufUC.size() != length)
            {
               cerr << " Error writing " << fieldName <<
                  " size == " << buf.size() << " should have been " << length << endl;
            }
            buf.resize(length);
            memcpy(&buf[0], &bufUC[0], length);

            output.write(&buf[0], length);
         }

         // b6 signals the DATA_FLD_6n field.
         if (bitTest(existmask, 6))
         {
            fieldName = BANDSB::DATA_FLD_6 + bandNumStr;
            length = 48;      // binary array of 48 single byte unsigned int's

            bufUC = dv_cast<vector<unsigned char> >(input.getAttribute(fieldName));
            if (bufUC.size() != length)
            {
               cerr << " Error writing " << fieldName <<
                  " size == " << buf.size() << " should have been " << length << endl;
            }
            buf.resize(length);
            memcpy(&buf[0], &bufUC[0], length);

            output.write(&buf[0], length);
         }
      }

      // bits: b5 = b4 = b3 = b2 = b1 = 0 (Not used, but present)

      unsigned int num_aux_b = 0;
      unsigned int num_aux_c = 0;

      // b0 = signals the NUM_AUX_B and NUM_AUX_C fields.
      if (bitTest(existmask, 0))
      {
         num_aux_b = dv_cast<unsigned int>(input.getAttribute(BANDSB::NUM_AUX_B));
         output << toString(num_aux_b, 2);
         num_aux_c = dv_cast<unsigned int>(input.getAttribute(BANDSB::NUM_AUX_C));
         output << toString(num_aux_c, 2);
      }

      for (unsigned int aux_b = 0; aux_b < num_aux_b; ++aux_b)
      {
         stringstream auxbStreamStr;
         auxbStreamStr << "#" << aux_b;
         string auxbStr(auxbStreamStr.str());

         string fieldName;

         fieldName = BANDSB::BAPF + auxbStr;
         string tempString = dv_cast<string>(input.getAttribute(fieldName));
         char bapf = tempString.c_str()[0];
         output << sizeString(tempString, 1);

         fieldName = BANDSB::UBAP + auxbStr;
         string ubap = dv_cast<string>(input.getAttribute(fieldName));
         output << sizeString(ubap, 7);

         for (unsigned int bandNum = 0; bandNum < count; ++bandNum)
         {
            stringstream bandStreamStr;
            bandStreamStr << "#" << bandNum;
            string bandNumStr(bandStreamStr.str());

            switch (bapf)
            {
               case 'I':
               {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
                  fieldName = BANDSB::APN + auxbStr + bandNumStr;
                  output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 10);
                  break;
               }
               case 'R':
               {
                  fieldName = BANDSB::APR + auxbStr + bandNumStr;
                  tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(fieldName)), BIG_ENDIAN_ORDER);
                  output.write(reinterpret_cast<char*>(&tempFloat), 4);
                  break;
               }
               case 'A':
               {
                  fieldName = BANDSB::APA + auxbStr + bandNumStr;
                  output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 20);
                  break;
               }
               default:
               {
                  // This is an error and should never happen with a proper BANDSB TAG
                  break;
               }
            }
         }
      }

      for (unsigned int aux_c = 0; aux_c < num_aux_c; ++aux_c)
      {
         stringstream auxcStreamStr;
         auxcStreamStr << "#" << aux_c;
         string auxcStr(auxcStreamStr.str());

         string fieldName;

         fieldName = BANDSB::CAPF + auxcStr;
         output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 1);
         char capf = buf[0];

         fieldName = BANDSB::UCAP + auxcStr;
         output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 7);
         string ubap(&buf.front());

         switch (capf)
         {
            case 'I':
            {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : BANDSB::APN Needs to be an int64 (lbeck)")
               fieldName = BANDSB::APN + auxcStr;
               output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 10);
               break;
            }
            case 'R':
            {
               fieldName = BANDSB::APR + auxcStr;
               tempFloat = convertBinary<float>(&dv_cast<float>(input.getAttribute(fieldName)), BIG_ENDIAN_ORDER);
               output.write(reinterpret_cast<char*>(&tempFloat), 4);
               break;
            }
            case 'A':
            {
               fieldName = BANDSB::APA + auxcStr;
               output << sizeString(dv_cast<string>(input.getAttribute(fieldName)), 20);
               break;
            }
            default:
            {
               // This is an error and should never happen with a proper BANDSB TAG
               break;
            }
         }
      }

   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}


TreExportStatus Nitf::BandsbParser::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, DynamicObject &tre, 
   unsigned int & ownerIndex, string & tagType, string &errorMessage) const
{
   const vector<DimensionDescriptor>& bands = exportDescriptor.getBands();

   // Find out if we are exporting a subset of the original bands. If so then delete the
   // band info for the excluded bands.

   const DynamicObject* pMetadata = descriptor.getMetadata();
   VERIFYRV(pMetadata != NULL, REMOVE);
   try
   {
      const DataVariant& nitfMetadata = pMetadata->getAttribute(Nitf::NITF_METADATA);
      const DynamicObject* pExistingBandsb = NULL;
      if (nitfMetadata.isValid() == true)
      {
         pExistingBandsb = getTagHandle(dv_cast<DynamicObject>(nitfMetadata), "BANDSB", FindFirst());
      }

      const vector<DimensionDescriptor>& exportBands = exportDescriptor.getBands();
      VERIFYRV(!exportBands.empty(), REMOVE);
      unsigned int newCount = exportBands.size();

      const string centerWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
         CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
      const string startWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
         START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};
      const string endWavelengthsPath[] = {SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
         END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME};

      vector<double> centerWavelengths;
      const DataVariant& dvCenterWavelengths = pMetadata->getAttributeByPath(centerWavelengthsPath);
      dvCenterWavelengths.getValue(centerWavelengths);

      vector<double> startWavelengths;
      const DataVariant& dvStartWavelengths = pMetadata->getAttributeByPath(startWavelengthsPath);
      dvStartWavelengths.getValue(startWavelengths);

      vector<double> endWavelengths;
      const DataVariant& dvEndWavelengths = pMetadata->getAttributeByPath(endWavelengthsPath);
      dvEndWavelengths.getValue(endWavelengths);

      unsigned int existMask = 0;
      if (centerWavelengths.size() >= newCount)
      {
         existMask |= (0x01 << 24);
      }

      vector<double> fwhms;
      if (startWavelengths.size() >= newCount && startWavelengths.size() == endWavelengths.size())
      {
         for (vector<double>::const_iterator startIter = startWavelengths.begin(), endIter = endWavelengths.begin();
            startIter != startWavelengths.end() && endIter != endWavelengths.end();
            ++startIter, ++endIter)
         {
            fwhms.push_back(*endIter - *startIter);
         }

         existMask |= ((0x01 << 19) | (0x01 << 23));
      }

      if (pExistingBandsb == NULL)
      {
         if (existMask == 0)
         {
            return REMOVE;
         }

         tre.setAttribute(BANDSB::RADIOMETRIC_QUANTITY, string());
         tre.setAttribute(BANDSB::RADIOMETRIC_QUANTITY_UNIT, string());
         tre.setAttribute(BANDSB::SCALE_FACTOR, static_cast<float>(1));
         tre.setAttribute(BANDSB::ADDITIVE_FACTOR, static_cast<float>(0));
         tre.setAttribute(BANDSB::ROW_GSD_UNIT, string());
         tre.setAttribute(BANDSB::COL_GSD_UNITS, string());
         tre.setAttribute(BANDSB::SPT_RESP_UNIT_ROW, string());
         tre.setAttribute(BANDSB::SPT_RESP_UNIT_COL, string());
         tre.setAttribute(BANDSB::DATA_FLD_1, vector<unsigned char>(48, 0));
         tre.setAttribute(BANDSB::EXISTENCE_MASK, existMask);
         tre.setAttribute(BANDSB::WAVE_LENGTH_UNIT, string("U"));
         pExistingBandsb = &tre;
      }

      tre.setAttribute(BANDSB::COUNT, newCount);

      tre.setAttribute(BANDSB::RADIOMETRIC_QUANTITY,
         pExistingBandsb->getAttribute(BANDSB::RADIOMETRIC_QUANTITY));

      tre.setAttribute(BANDSB::RADIOMETRIC_QUANTITY_UNIT,
         pExistingBandsb->getAttribute(BANDSB::RADIOMETRIC_QUANTITY_UNIT));

      tre.setAttribute(BANDSB::SCALE_FACTOR,
         pExistingBandsb->getAttribute(BANDSB::SCALE_FACTOR));

      tre.setAttribute(BANDSB::ADDITIVE_FACTOR,
         pExistingBandsb->getAttribute(BANDSB::ADDITIVE_FACTOR));

      DataVariant tempDV = pExistingBandsb->getAttribute(BANDSB::ROW_GSD);
      if (tempDV.isValid())
      {
         tre.setAttribute(BANDSB::ROW_GSD, tempDV);
      }

      tre.setAttribute(BANDSB::ROW_GSD_UNIT, pExistingBandsb->getAttribute(BANDSB::ROW_GSD_UNIT));

      tempDV = pExistingBandsb->getAttribute(BANDSB::COL_GSD);
      if (tempDV.isValid())
      {
         tre.setAttribute(BANDSB::COL_GSD, tempDV);
      }

      tre.setAttribute(BANDSB::COL_GSD_UNITS, pExistingBandsb->getAttribute(BANDSB::COL_GSD_UNITS));

      tempDV = pExistingBandsb->getAttribute(BANDSB::SPT_RESP_ROW);
      if (tempDV.isValid())
      {
         tre.setAttribute(BANDSB::SPT_RESP_ROW, tempDV);
      }

      tre.setAttribute(BANDSB::SPT_RESP_UNIT_ROW, pExistingBandsb->getAttribute(BANDSB::SPT_RESP_UNIT_ROW));

      tempDV = pExistingBandsb->getAttribute(BANDSB::SPT_RESP_COL);
      if (tempDV.isValid())
      {
         tre.setAttribute(BANDSB::SPT_RESP_COL, tempDV);
      }

      tre.setAttribute(BANDSB::SPT_RESP_UNIT_COL, pExistingBandsb->getAttribute(BANDSB::SPT_RESP_UNIT_COL));

      tre.setAttribute(BANDSB::DATA_FLD_1, pExistingBandsb->getAttribute(BANDSB::DATA_FLD_1));

      // Bit-wise Existence Mask Field 32 flags represented by the bits of this 4
      // byte (32 bit) field. (b31, b30, b29, b28 ... b2, b1, b0)

      // A bit set to zero signals that a conditional field is not present in this
      // extension. A bit set to the value one indicates the inclusion of the conditional field.
      existMask |= dv_cast<unsigned int>(pExistingBandsb->getAttribute(BANDSB::EXISTENCE_MASK));
      if (fwhms.empty() == false)
      {
         // If the FWHMs were calculated, clear the FWHM UNCERTAINTY value since it is unknown.
         existMask &= ~(0x01 << 22);
      }

      tre.setAttribute(BANDSB::EXISTENCE_MASK, existMask);

      // b31 signals the RADIOMETRIC ADJUSTMENT SURFACE and ATMOSPHERIC ADJUSTMENT ALTITUDE fields.
      if (bitTest(existMask, 31))
      {
         tre.setAttribute(BANDSB::RADIOMETRIC_ADJUSTMENT_SURFACE,
            pExistingBandsb->getAttribute(BANDSB::RADIOMETRIC_ADJUSTMENT_SURFACE));
         tre.setAttribute(BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE,
            pExistingBandsb->getAttribute(BANDSB::ATMOSPHERIC_ADJUSTMENT_ALTITUDE));
      }

      // b30 signals the DIAMETER field.
      if (bitTest(existMask, 30))
      {
         tre.setAttribute(BANDSB::DIAMETER, pExistingBandsb->getAttribute(BANDSB::DIAMETER));
      }

      // b29 signals the DATA_FLD_2 field.
      if (bitTest(existMask, 29))
      {
         tre.setAttribute(BANDSB::DATA_FLD_2, pExistingBandsb->getAttribute(BANDSB::DATA_FLD_2));
      }

      // The WAVE_LENGTH_UNIT field can be triggered by any of the following bits:
      // b24 signals the CWAVE and WAVE_LENGTH_UNIT fields.
      // b23 signals the FWHM and WAVE_LENGTH_UNIT fields.
      // b22 signals the FWHM_UNC and WAVE_LENGTH_UNIT fields.
      // b21 signals the NOM_WAVEn and WAVE_LENGTH_UNIT fields.
      // b20 signals the NOM_WAVE_UNCn and WAVE_LENGTH_UNIT fields.
      // b19 signals the LBOUNDn, UBOUNDn and WAVE_LENGTH_UNIT fields.
      if (bitTest(existMask, 24) || bitTest(existMask, 23) || bitTest(existMask, 22)
         || bitTest(existMask, 21) || bitTest(existMask, 20) || bitTest(existMask, 19))
      {
         tre.setAttribute(BANDSB::WAVE_LENGTH_UNIT, pExistingBandsb->getAttribute(BANDSB::WAVE_LENGTH_UNIT));
      }

      unsigned int bandcount(0);

      for (vector<DimensionDescriptor>::const_iterator iter = exportBands.begin(); iter != exportBands.end(); ++iter)
      {

         // Use the original band number to find the associated band data in the original TRE
         LOG_IF(!iter->isOriginalNumberValid(), continue);
         unsigned int origBandNum = iter->getOriginalNumber();

         stringstream bandStreamStr;
         bandStreamStr << "#" << bandcount;
         string bandStr(bandStreamStr.str());

         stringstream origBandStreamStr;
         origBandStreamStr << "#" << origBandNum;
         string origBandStr(origBandStreamStr.str());

         ++bandcount;

         string fieldName;
         string origFieldName;

         // b28 flags the BANDIDn field.
         if (bitTest(existMask, 28))
         {
            fieldName = BANDSB::BANDID + bandStr;
            origFieldName = BANDSB::BANDID + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b27 signals the BAD_BANDn field.
         if (bitTest(existMask, 27))
         {
            fieldName = BANDSB::BAD_BAND + bandStr;
            origFieldName = BANDSB::BAD_BAND + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b26 signals the NIIRSn field.
         if (bitTest(existMask, 26))
         {
            fieldName = BANDSB::NIIRS + bandStr;
            origFieldName = BANDSB::NIIRS + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b25 signals the FOCAL_LENn field.
         if (bitTest(existMask, 25))
         {
            fieldName = BANDSB::FOCAL_LEN + bandStr;
            origFieldName = BANDSB::FOCAL_LEN + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b24 signals the CWAVE field.
         if (bitTest(existMask, 24))
         {
            fieldName = BANDSB::CWAVE + bandStr;
            if (centerWavelengths.size() > origBandNum)
            {
               tre.setAttribute(fieldName, centerWavelengths[origBandNum]);
            }
            else
            {
               origFieldName = BANDSB::CWAVE + origBandStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
            }
         }

         // b23 signals the FWHM field.
         if (bitTest(existMask, 23))
         {
            fieldName = BANDSB::FWHM + bandStr;
            if (fwhms.size() > origBandNum)
            {
               tre.setAttribute(fieldName, fwhms[origBandNum]);
            }
            else
            {
               origFieldName = BANDSB::FWHM + origBandStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
            }
         }

         // b22 signals the FWHM_UNC field.
         if (bitTest(existMask, 22))
         {
            fieldName = BANDSB::FWHM_UNC + bandStr;
            origFieldName = BANDSB::FWHM_UNC + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b21 signals the NOM_WAVEn field.
         if (bitTest(existMask, 21))
         {
            fieldName = BANDSB::NOM_WAVE + bandStr;
            origFieldName = BANDSB::NOM_WAVE + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b20 signals the NOM_WAVE_UNCn field.
         if (bitTest(existMask, 20))
         {
            fieldName = BANDSB::NOM_WAVE_UNC + bandStr;
            origFieldName = BANDSB::NOM_WAVE_UNC + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b19 signals the LBOUNDn, UBOUNDn field.
         if (bitTest(existMask, 19))
         {
            fieldName = BANDSB::LBOUND + bandStr;
            if (startWavelengths.size() > origBandNum)
            {
               tre.setAttribute(fieldName, startWavelengths[origBandNum]);
            }
            else
            {
               origFieldName = BANDSB::LBOUND + origBandStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
            }

            fieldName = BANDSB::UBOUND + bandStr;
            if (endWavelengths.size() > origBandNum)
            {
               tre.setAttribute(fieldName, endWavelengths[origBandNum]);
            }
            else
            {
               origFieldName = BANDSB::UBOUND + origBandStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
            }
         }

         // b18 signals the SCALE FACTORn, and ADDITIVE FACTORn fields.
         if (bitTest(existMask, 18))
         {
            fieldName = BANDSB::SCALE_FACTOR + bandStr;
            origFieldName = BANDSB::SCALE_FACTOR + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::ADDITIVE_FACTOR + bandStr;
            origFieldName = BANDSB::ADDITIVE_FACTOR + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b17 signals the START_TIMEn.
         if (bitTest(existMask, 17))
         {
            fieldName = BANDSB::START_TIME_FRAC + bandStr;
            origFieldName = BANDSB::START_TIME_FRAC + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::START_TIME + bandStr;
            origFieldName = BANDSB::START_TIME + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b16 signals the INT_TIMEn field.
         if (bitTest(existMask, 16))
         {
            fieldName = BANDSB::INT_TIME + bandStr;
            origFieldName = BANDSB::INT_TIME + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b15 signals the CALDRK and CALIBRATION SENSITIVITYn fields.
         if (bitTest(existMask, 15))
         {
            fieldName = BANDSB::CALDRK + bandStr;
            origFieldName = BANDSB::CALDRK + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::CALIBRATION_SENSITIVITY + bandStr;
            origFieldName = BANDSB::CALIBRATION_SENSITIVITY + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existMask, 14))
         {
            fieldName = BANDSB::ROW_GSD + bandStr;
            origFieldName = BANDSB::ROW_GSD + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
         if (bitTest(existMask, 13))
         {
            fieldName = BANDSB::ROW_GSD_UNC + bandStr;
            origFieldName = BANDSB::ROW_GSD_UNC + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existMask, 14))
         {
            fieldName = BANDSB::ROW_GSD_UNIT + bandStr;
            origFieldName = BANDSB::ROW_GSD_UNIT + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::COL_GSD + bandStr;
            origFieldName = BANDSB::COL_GSD + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b13 signals the ROW_GSD_UNCn and COL_GSD_UNCn fields. (If b13 is set to 1 then b14 must be set.)
         if (bitTest(existMask, 13))
         {
            fieldName = BANDSB::COL_GSD_UNC + bandStr;
            origFieldName = BANDSB::COL_GSD_UNC + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b14 signals the ROW_GSDn and ROW_GSD_UNITSn, COL_GSDn, COL_GSD_UNITSn fields.
         if (bitTest(existMask, 14))
         {
            fieldName = BANDSB::COL_GSD_UNIT + bandStr;
            origFieldName = BANDSB::COL_GSD_UNIT + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b12 signals the BKNOISEn and SCNNOISEn fields.
         if (bitTest(existMask, 12))
         {
            fieldName = BANDSB::BKNOISE + bandStr;
            origFieldName = BANDSB::BKNOISE + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::SCNNOISE + bandStr;
            origFieldName = BANDSB::SCNNOISE + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existMask, 11))
         {
            fieldName = BANDSB::SPT_RESP_FUNCTION_ROW + bandStr;
            origFieldName = BANDSB::SPT_RESP_FUNCTION_ROW + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
         if (bitTest(existMask, 10))
         {
            fieldName = BANDSB::SPT_RESPUNC_ROW + bandStr;
            origFieldName = BANDSB::SPT_RESPUNC_ROW + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existMask, 11))
         {
            fieldName = BANDSB::SPT_RESP_UNIT_ROW + bandStr;
            origFieldName = BANDSB::SPT_RESP_UNIT_ROW + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));

            fieldName = BANDSB::SPT_RESP_FUNCTION_COL + bandStr;
            origFieldName = BANDSB::SPT_RESP_FUNCTION_COL + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b10 signals the SPT_RESP_UNC_ROWn and SPT_RESP_UNC_COLn fields. (If b10 is set to 1 then b11 must be set.)
         if (bitTest(existMask, 10))
         {
            fieldName = BANDSB::SPT_RESPUNC_COL + bandStr;
            origFieldName = BANDSB::SPT_RESPUNC_COL + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b11 signals the SPT_RESP_FUNCTION_ROWn, SPT_RESP_UNIT_ROWn,
         // SPT_RESP_FUNCTION_COLn, SPT_RESP_UNIT_COLn fields.
         if (bitTest(existMask, 11))
         {
            fieldName = BANDSB::SPT_RESP_UNIT_COL + bandStr;
            origFieldName = BANDSB::SPT_RESP_UNIT_COL + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b9 signals the DATA_FLD_3n field.
         if (bitTest(existMask, 9))
         {
            fieldName = BANDSB::DATA_FLD_3 + bandStr;
            origFieldName = BANDSB::DATA_FLD_3 + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b8 signals the DATA_FLD_4n field.
         if (bitTest(existMask, 8))
         {
            fieldName = BANDSB::DATA_FLD_4 + bandStr;
            origFieldName = BANDSB::DATA_FLD_4 + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b7 signals the DATA_FLD_5n field.
         if (bitTest(existMask, 7))
         {
            fieldName = BANDSB::DATA_FLD_5 + bandStr;
            origFieldName = BANDSB::DATA_FLD_5 + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }

         // b6 signals the DATA_FLD_6n field.
         if (bitTest(existMask, 6))
         {
            fieldName = BANDSB::DATA_FLD_6 + bandStr;
            origFieldName = BANDSB::DATA_FLD_6 + origBandStr;
            tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
         }
      }

      // bits: b5 = b4 = b3 = b2 = b1 = 0 (Not used, but present)

      unsigned int num_aux_b = 0;
      unsigned int num_aux_c = 0;

      // b0 = signals the NUM_AUX_B and NUM_AUX_C fields.
      if (bitTest(existMask, 0))
      {
         num_aux_b = dv_cast<unsigned int>(pExistingBandsb->getAttribute(BANDSB::NUM_AUX_B));
         tre.setAttribute(BANDSB::NUM_AUX_B, num_aux_b);

         num_aux_c = dv_cast<unsigned int>(pExistingBandsb->getAttribute(BANDSB::NUM_AUX_C));
         tre.setAttribute(BANDSB::NUM_AUX_C, num_aux_c);
      }

      for (unsigned int aux_b = 0; aux_b < num_aux_b; ++aux_b)
      {
         stringstream auxbStreamStr;
         auxbStreamStr << "#" << aux_b;
         string auxbStr(auxbStreamStr.str());

         string fieldName;

         fieldName = BANDSB::BAPF + auxbStr;
         string tempString = dv_cast<string>(pExistingBandsb->getAttribute(fieldName));
         tre.setAttribute(fieldName, tempString);
         char bapf = tempString.c_str()[0];

         fieldName = BANDSB::UBAP + auxbStr;
         string ubap = dv_cast<string>(pExistingBandsb->getAttribute(fieldName));
         tre.setAttribute(fieldName, ubap);

         unsigned int bandcount(0);

         for (vector<DimensionDescriptor>::const_iterator iter = exportBands.begin();
            iter != exportBands.end(); ++iter)
         {
            // Use the original band number to find the associated band data in the original TRE
            LOG_IF(!iter->isOriginalNumberValid(), continue);
            unsigned int origBandNum = iter->getOriginalNumber();

            stringstream bandStreamStr;
            bandStreamStr << "#" << bandcount;
            string bandNumStr(bandStreamStr.str());

            stringstream origBandStreamStr;
            origBandStreamStr << "#" << origBandNum;
            string origBandNumStr(origBandStreamStr.str());

            ++bandcount;

            string fieldName;
            string origFieldName;

            switch (bapf)
            {
               case 'I':
               {
                  fieldName = BANDSB::APN + auxbStr + bandNumStr;
                  origFieldName = BANDSB::APN + auxbStr + origBandNumStr;
                  tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
                  break;
               }
               case 'R':
               {
                  fieldName = BANDSB::APR + auxbStr + bandNumStr;
                  origFieldName = BANDSB::APR + auxbStr + origBandNumStr;
                  tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
                  break;
               }
               case 'A':
               {
                  fieldName = BANDSB::APA + auxbStr + bandNumStr;
                  origFieldName = BANDSB::APA + auxbStr + origBandNumStr;
                  tre.setAttribute(fieldName, pExistingBandsb->getAttribute(origFieldName));
                  break;
               }
               default:
               {
                  // This is an error and should never happen with a proper BANDSB TAG
                  break;
               }
            }
         }
      }

      for (unsigned int aux_c = 0; aux_c < num_aux_c; ++aux_c)
      {
         stringstream auxcStreamStr;
         auxcStreamStr << "#" << aux_c;
         string auxcStr(auxcStreamStr.str());

         string fieldName;

         fieldName = BANDSB::CAPF + auxcStr;
         string capfStr = dv_cast<string>(pExistingBandsb->getAttribute(fieldName));
         tre.setAttribute(fieldName, capfStr);
         char capf = capfStr[0];

         fieldName = BANDSB::UCAP + auxcStr;
         tre.setAttribute(fieldName, pExistingBandsb->getAttribute(fieldName));

         switch (capf)
         {
            case 'I':
            {
               fieldName = BANDSB::APN + auxcStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(fieldName));
               break;
            }
            case 'R':
            {
               fieldName = BANDSB::APR + auxcStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(fieldName));
               break;
            }
            case 'A':
            {
               fieldName = BANDSB::APA + auxcStr;
               tre.setAttribute(fieldName, pExistingBandsb->getAttribute(fieldName));
               break;
            }
            default:
            {
               // This is an error and should never happen with a proper BANDSB TAG
               break;
            }
         }
      }
   }
   catch (const bad_cast&)
   {
      return REMOVE;
   }
   catch (const string& message)
   {
      errorMessage = message;
      return REMOVE;
   }

   return REPLACE;
}

bool Nitf::BandsbParser::importMetadata(const DynamicObject& tre,
   RasterDataDescriptor& descriptor, string& errorMessage) const
{
   return updateSpecialMetadata(descriptor.getMetadata(), mCenterWavelengths,
      mStartWavelengths, mEndWavelengths, mFwhms, mWavelengthsInInverseCentimeters);
}
