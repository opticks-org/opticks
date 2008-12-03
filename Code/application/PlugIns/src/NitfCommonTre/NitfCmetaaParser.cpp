/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "NitfCmetaaParser.h"
#include "NitfConstants.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"

#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;
using namespace Nitf;
using namespace Nitf::TRE;

Nitf::CmetaaParser::CmetaaParser()
{
   setName("CMETAA");
   setDescriptorId("{FED70E25-4022-4c29-82FC-EB5F321E63DC}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::CmetaaParser::runAllTests(Progress* pProgress, ostream& failure)
{
   static const string data(
      "00"                              // NUM_RELATED_TRES
      // RELATED_TRES
      "                                                                                                               "
      "         "
      "            "                    // RD_RPC_NO
      "PF  "                            // IF_PROCESS
      "X   "                            // RD_CEN_FREQ
      "1FG  "                           // RD_MODE
      "0001"                            // RD_PATCH_NO
      "M1P2 "                           // CMPLX_DOMAIN
      "LLM "                            // CMPLX_MAG_REMAP_TYPE
      "1.94369"                         // CMPLX_LIN_SCALE
      "0000000"                         // CMPLX_AVG_POWER
      "00117"                           // CMPLX_LINLOG_TP
      "UQ1"                             // CMPLX_PHASE_QUANT_FLAG
      "00"                              // CMPLX_PHASE_QUANT_BIT_DEPTH
      "08"                              // CMPLX_SIZE_1
      "NC"                              // CMPLX_IC_1
      "08"                              // CMPLX_SIZE_2
      "NC"                              // CMPLX_IC_2
      "00000"                           // CMPLX_IC_BPP
      "TAY"                             // CMPLX_WEIGHT
      "40"                              // CMPLX_AZ_SLL
      "40"                              // CMPLX_RNG_SLL
      "12"                              // CMPLX_AZ_TAY_NBAR
      "12"                              // CMPLX_RNG_TAY_NBAR
      "   "                             // CMPLX_WEIGHT_NORM
      "G"                               // CMPLX_SIGNAL_PLANE
      "000000"                          // IF_DC_SF_ROW
      "000000"                          // IF_DC_SF_COL
      "000000"                          // IF_PATCH_1_ROW
      "000000"                          // IF_PATCH_1_COL
      "000000"                          // IF_PATCH_2_ROW
      "000000"                          // IF_PATCH_2_COL
      "000000"                          // IF_PATCH_3_ROW
      "000000"                          // IF_PATCH_3_COL
      "000000"                          // IF_PATCH_4_ROW
      "000000"                          // IF_PATCH_4_COL
      "00000000"                        // IF_DC_IS_ROW
      "00000000"                        // IF_DC_IS_COL
      "00000000"                        // IF_IMG_ROW_DC
      "00000000"                        // IF_IMG_COL_DC
      "000000"                          // IF_TILE_1_ROW
      "000000"                          // IF_TILE_1_COL
      "000000"                          // IF_TILE_2_ROW
      "000000"                          // IF_TILE_2_COL
      "000000"                          // IF_TILE_3_ROW
      "000000"                          // IF_TILE_3_COL
      "000000"                          // IF_TILE_4_ROW
      "000000"                          // IF_TILE_4_COL
      "N"                               // IF_RD
      "O"                               // IF_RGWLK
      "N"                               // IF_KEYSTN
      "O"                               // IF_LINSFT
      "O"                               // IF_SUBPATCH
      "N"                               // IF_GEODIST
      "N"                               // IF_RGFO
      "N"                               // IF_BEAM_COMP
      "0009.999"                        // IF_RGRES
      "0009.999"                        // IF_AZRES
      "09.99999"                        // IF_RSS
      "09.99999"                        // IF_AZSS
      "09.99999"                        // IF_RSR
      "09.99999"                        // IF_AZSR
      "0000256"                         // IF_RFFT_SAMP
      "0000256"                         // IF_AZFFT_SAMP
      "0000400"                         // IF_RFFT_TOT
      "0000400"                         // IF_AZFFT_TOT
      "000000"                          // IF_SUBP_ROW
      "000000"                          // IF_SUBP_COL
      "0000"                            // IF_SUB_RG
      "0000"                            // IF_SUB_AZ
      "+"                               // IF_RFFTS
      "+"                               // IF_AFFTS
      "ROW_INC"                         // IF_RANGE_DATA
      "+"                               // IF_INCPH
      "        "                        // IF_SR_NAME1
      "01.00000"                        // IF_SR_AMOUNT1
      "        "                        // IF_SR_NAME2
      "01.00000"                        // IF_SR_AMOUNT2
      "        "                        // IF_SR_NAME3
      "01.00000"                        // IF_SR_AMOUNT
      "N    "                           // AF_TYPE1
      "N    "                           // AF_TYPE2
      "N    "                           // AF_TYPE3
      "V"                               // POL_TR
      "V"                               // POL_RE
      "                                        " // POL_REFERENCE
      "P"                               // POL
      "Y"                               // POL_REG
      "00.00"                           // POL_ISO_1
      "A"                               // POL_BAL
      "0.000000"                        // POL_BAL_MAG
      "+0.00000"                        // POL_BAL_PHS
      " "                               // POL_HCOMP
      "          "                      // P_HCOMP_BASIS
      "000000000"                       // POL_HCOMP_COEF_1
      "000000000"                       // POL_HCOMP_COEF_2
      "000000000"                       // POL_HCOMP_COEF_3
      "A"                               // POL_AFCOMP
      "               "                 // POL_SPARE_A
      "000000000"                       // POL_SPARE_N
      "1999OCT31"                       // T_UTC_YYYYMMMDD
      "003045"                          // T_HHMMSSUTC
      "123045"                          // T_HHMMSSLOCAL
      "09999999.99"                     // CG_SRAC
      "0000.00"                         // CG_SLANT_CONFIDENCE
      "00000000.00"                     // CG_CROSS
      "0000.00"                         // CG_CROSS_CONFIDENCE
      "+010.0000"                       // CG_CAAC
      "0.0000"                          // CG_CONE_CONFIDENCE
      "+00.0000"                        // CG_GPSAC
      "0.0000"                          // CG_GPSAC_CONFIDENCE
      "+09.9999"                        // CG_SQUINT
      "09.9999"                         // CG_GAAC
      "0.0000"                          // CG_GAAC_CONFIDENCE
      "09.9999"                         // CG_INCIDENT
      "0000000"                         // CG_SLOPE
      "+09.9999"                        // CG_TILT
      "R"                               // CG_LD
      "009.9999"                        // CG_NORTH
      "0.0000"                          // CG_NORTH_CONFIDENCE
      "000.0000"                        // CG_EAST
      "000.0000"                        // CG_RLOS
      "0.0000"                          // CG_LOS_CONFIDENCE
      "009.9999"                        // CG_LAYOVER
      "000.0000"                        // CG_SHADOW
      "000.000"                         // CG_OPM
      "ECEF "                           // CG_MODEL
      "+00000000.000"                   // CG_AMPT_X
      "+00000000.000"                   // CG_AMPT_Y
      "+00000000.000"                   // CG_AMPT_Z
      "000000"                          // CG_AP_CONF_XY
      "000000"                          // CG_AP_CONF_Z
      "+00999999.999"                   // CG_APCEN_X
      "+00999999.999"                   // CG_APCEN_Y
      "+00999999.999"                   // CG_APCEN_Z
      "000.00"                          // CG_APER_CONF_XY
      "000.00"                          // CG_APER_CONF_Z
      "-0.999999"                       // CG_FPNUV_X
      "+0.999999"                       // CG_FPNUV_Y
      "+0.999999"                       // CG_FPNUV_Z
      "-0.999999"                       // CG_IDPNUVX
      "+0.999999"                       // CG_IDPNUVY
      "+0.999999"                       // CG_IDPNUVZ
      "-00999999.999"                   // CG_SCECN_X
      "+09999999.999"                   // CG_SCECN_Y
      "+09999999.999"                   // CG_SCECN_Z
      "000.00"                          // CG_SC_CONF_XY
      "000.00"                          // CG_SC_CONF_Z
      "00000.00"                        // CG_SWWD
      "-09999.999"                      // CG_SNVEL_X
      "-09999.999"                      // CG_SNVEL_Y
      "+09999.999"                      // CG_SNVEL_Z
      "+00.000000"                      // CG_SNACC_X
      "+00.000000"                      // CG_SNACC_Y
      "+00.000000"                      // CG_SNACC_Z
      "+000.000"                        // CG_SNATT_ROLL
      "+000.000"                        // CG_SNATT_PITCH
      "+000.000"                        // CG_SNATT_YAW
      "+0.000000"                       // CG_GTP_X
      "+0.000000"                       // CG_GTP_Y
      "+0.000000"                       // CG_GTP_Z

      "NA  "                            // CG_MAP_TYPE

      // start conditional:  CG_MAP_TYPE == "NA"
      "                                                                                                               "
      "                      "       // CG_MAP_TYPE_BLANK
      // end conditional:  CG_MAP_TYPE == "NA"
      "                                                                                                               "
      "                                 "   // CG_SPARE_A
      "0000000"                         // CA_CALPA
      "000000000000.0"                  // WF_SRTFR
      "000000000000.0"                  // WF_ENDFR
      "+00.000000"                      // WF_CHRPRT
      "0.0000000"                       // WF_WIDTH
      "09999999999.9"                   // WF_CENFRQ
      "00000000000.0"                   // WF_BW
      "00000.0"                         // WF_PRF
      "0.0000000"                       // WF_PRI
      "099.990"                         // WF_CDP
      "000009999"                       // WF_NUMBER_OF_PULSES
      "Y"                               // VPH_COND
      );


   static const string data_error4(
      "00"                              // NUM_RELATED_TRES
      "                                                                                                               "
      "         " // RELATED_TRES
      "            "                    // RD_RPC_NO
      "    "                            // IF_PROCESS          // ERROR: IF_PROCESS all blank == INVALID
      "X   "                            // RD_CEN_FREQ
      "1FG  "                           // RD_MODE
      "0001"                            // RD_PATCH_NO
      "M1P2 "                           // CMPLX_DOMAIN
      "LLM "                            // CMPLX_MAG_REMAP_TYPE
      "1.94369"                         // CMPLX_LIN_SCALE
      "0000000"                         // CMPLX_AVG_POWER
      "00117"                           // CMPLX_LINLOG_TP
      "UQ1"                             // CMPLX_PHASE_QUANT_FLAG
      "00"                              // CMPLX_PHASE_QUANT_BIT_DEPTH
      "08"                              // CMPLX_SIZE_1
      "NC"                              // CMPLX_IC_1
      "08"                              // CMPLX_SIZE_2
      "NC"                              // CMPLX_IC_2
      "00000"                           // CMPLX_IC_BPP
      "TAY"                             // CMPLX_WEIGHT
      "40"                              // CMPLX_AZ_SLL
      "40"                              // CMPLX_RNG_SLL
      "12"                              // CMPLX_AZ_TAY_NBAR
      "12"                              // CMPLX_RNG_TAY_NBAR
      "   "                             // CMPLX_WEIGHT_NORM
      "G"                               // CMPLX_SIGNAL_PLANE
      "000000"                          // IF_DC_SF_ROW
      "000000"                          // IF_DC_SF_COL
      "000000"                          // IF_PATCH_1_ROW
      "000000"                          // IF_PATCH_1_COL
      "000000"                          // IF_PATCH_2_ROW
      "000000"                          // IF_PATCH_2_COL
      "000000"                          // IF_PATCH_3_ROW
      "000000"                          // IF_PATCH_3_COL
      "000000"                          // IF_PATCH_4_ROW
      "000000"                          // IF_PATCH_4_COL
      "00000000"                        // IF_DC_IS_ROW
      "00000000"                        // IF_DC_IS_COL
      "00000000"                        // IF_IMG_ROW_DC
      "00000000"                        // IF_IMG_COL_DC
      "000000"                          // IF_TILE_1_ROW
      "000000"                          // IF_TILE_1_COL
      "000000"                          // IF_TILE_2_ROW
      "000000"                          // IF_TILE_2_COL
      "000000"                          // IF_TILE_3_ROW
      "000000"                          // IF_TILE_3_COL
      "000000"                          // IF_TILE_4_ROW
      "000000"                          // IF_TILE_4_COL
      "N"                               // IF_RD
      "O"                               // IF_RGWLK
      "N"                               // IF_KEYSTN
      "O"                               // IF_LINSFT
      "O"                               // IF_SUBPATCH
      "N"                               // IF_GEODIST
      "N"                               // IF_RGFO
      "N"                               // IF_BEAM_COMP
      "0009.999"                        // IF_RGRES
      "0009.999"                        // IF_AZRES
      "09.99999"                        // IF_RSS
      "09.99999"                        // IF_AZSS
      "09.99999"                        // IF_RSR
      "09.99999"                        // IF_AZSR
      "0000256"                         // IF_RFFT_SAMP
      "0000256"                         // IF_AZFFT_SAMP
      "0000400"                         // IF_RFFT_TOT
      "0000400"                         // IF_AZFFT_TOT
      "000000"                          // IF_SUBP_ROW
      "000000"                          // IF_SUBP_COL
      "0000"                            // IF_SUB_RG
      "0000"                            // IF_SUB_AZ
      "+"                               // IF_RFFTS
      "+"                               // IF_AFFTS
      "ROW_INC"                         // IF_RANGE_DATA
      "+"                               // IF_INCPH
      "        "                        // IF_SR_NAME1
      "01.00000"                        // IF_SR_AMOUNT1
      "        "                        // IF_SR_NAME2
      "01.00000"                        // IF_SR_AMOUNT2
      "        "                        // IF_SR_NAME3
      "01.00000"                        // IF_SR_AMOUNT
      "N    "                           // AF_TYPE1
      "N    "                           // AF_TYPE2
      "N    "                           // AF_TYPE3
      "V"                               // POL_TR
      "V"                               // POL_RE
      "                                        " // POL_REFERENCE
      "P"                               // POL
      "Y"                               // POL_REG
      "00.00"                           // POL_ISO_1
      "A"                               // POL_BAL
      "0.000000"                        // POL_BAL_MAG
      "+0.00000"                        // POL_BAL_PHS
      " "                               // POL_HCOMP
      "          "                      // P_HCOMP_BASIS
      "000000000"                       // POL_HCOMP_COEF_1
      "000000000"                       // POL_HCOMP_COEF_2
      "000000000"                       // POL_HCOMP_COEF_3
      "A"                               // POL_AFCOMP
      "               "                 // POL_SPARE_A
      "000000000"                       // POL_SPARE_N
      "1999OCT31"                       // T_UTC_YYYYMMMDD
      "003045"                          // T_HHMMSSUTC
      "123045"                          // T_HHMMSSLOCAL
      "09999999.99"                     // CG_SRAC
      "0000.00"                         // CG_SLANT_CONFIDENCE
      "00000000.00"                     // CG_CROSS
      "0000.00"                         // CG_CROSS_CONFIDENCE
      "+010.0000"                       // CG_CAAC
      "0.0000"                          // CG_CONE_CONFIDENCE
      "+00.0000"                        // CG_GPSAC
      "0.0000"                          // CG_GPSAC_CONFIDENCE
      "+09.9999"                        // CG_SQUINT
      "09.9999"                         // CG_GAAC
      "0.0000"                          // CG_GAAC_CONFIDENCE
      "09.9999"                         // CG_INCIDENT
      "0000000"                         // CG_SLOPE
      "+09.9999"                        // CG_TILT
      "R"                               // CG_LD
      "009.9999"                        // CG_NORTH
      "0.0000"                          // CG_NORTH_CONFIDENCE
      "000.0000"                        // CG_EAST
      "000.0000"                        // CG_RLOS
      "0.0000"                          // CG_LOS_CONFIDENCE
      "009.9999"                        // CG_LAYOVER
      "000.0000"                        // CG_SHADOW
      "000.000"                         // CG_OPM
      "ECEF "                           // CG_MODEL
      "+00000000.000"                   // CG_AMPT_X
      "+00000000.000"                   // CG_AMPT_Y
      "+00000000.000"                   // CG_AMPT_Z
      "000000"                          // CG_AP_CONF_XY
      "000000"                          // CG_AP_CONF_Z
      "+00999999.999"                   // CG_APCEN_X
      "+00999999.999"                   // CG_APCEN_Y
      "+00999999.999"                   // CG_APCEN_Z
      "000.00"                          // CG_APER_CONF_XY
      "000.00"                          // CG_APER_CONF_Z
      "-0.999999"                       // CG_FPNUV_X
      "+0.999999"                       // CG_FPNUV_Y
      "+0.999999"                       // CG_FPNUV_Z
      "-0.999999"                       // CG_IDPNUVX
      "+0.999999"                       // CG_IDPNUVY
      "+0.999999"                       // CG_IDPNUVZ
      "-00999999.999"                   // CG_SCECN_X
      "+09999999.999"                   // CG_SCECN_Y
      "+09999999.999"                   // CG_SCECN_Z
      "000.00"                          // CG_SC_CONF_XY
      "000.00"                          // CG_SC_CONF_Z
      "00000.00"                        // CG_SWWD
      "-09999.999"                      // CG_SNVEL_X
      "-09999.999"                      // CG_SNVEL_Y
      "+09999.999"                      // CG_SNVEL_Z
      "+00.000000"                      // CG_SNACC_X
      "+00.000000"                      // CG_SNACC_Y
      "+00.000000"                      // CG_SNACC_Z
      "+000.000"                        // CG_SNATT_ROLL
      "+000.000"                        // CG_SNATT_PITCH
      "+000.000"                        // CG_SNATT_YAW
      "+0.000000"                       // CG_GTP_X
      "+0.000000"                       // CG_GTP_Y
      "+0.000000"                       // CG_GTP_Z

      "NA  "                            // CG_MAP_TYPE

      // start conditional:  CG_MAP_TYPE == "NA"
      "                                                                                                               "
      "                      "       // CG_MAP_TYPE_BLANK
      // end conditional:  CG_MAP_TYPE == "NA"

      "                                                                                                               "
      "                                 "   // CG_SPARE_A
      "0000000"                         // CA_CALPA
      "000000000000.0"                  // WF_SRTFR
      "000000000000.0"                  // WF_ENDFR
      "+00.000000"                      // WF_CHRPRT
      "0.0000000"                       // WF_WIDTH
      "09999999999.9"                   // WF_CENFRQ
      "00000000000.0"                   // WF_BW
      "00000.0"                         // WF_PRF
      "0.0000000"                       // WF_PRI
      "099.990"                         // WF_CDP
      "000009999"                       // WF_NUMBER_OF_PULSES
      "Y"                               // VPH_COND
      );

   static const string data_error5(
      "00"                              // NUM_RELATED_TRES
      "                                                                                                               "
      "         " // RELATED_TRES
      "            "                    // RD_RPC_NO
      "PF  "                            // IF_PROCESS
      "X   "                            // RD_CEN_FREQ
      "1FG  "                           // RD_MODE
      "0001"                            // RD_PATCH_NO
      "M1P2 "                           // CMPLX_DOMAIN
      "LLM "                            // CMPLX_MAG_REMAP_TYPE
      "1.94369"                         // CMPLX_LIN_SCALE
      "0000000"                         // CMPLX_AVG_POWER
      "00117"                           // CMPLX_LINLOG_TP
      "UQ1"                             // CMPLX_PHASE_QUANT_FLAG
      "00"                              // CMPLX_PHASE_QUANT_BIT_DEPTH
      "08"                              // CMPLX_SIZE_1
      "NC"                              // CMPLX_IC_1
      "08"                              // CMPLX_SIZE_2
      "NC"                              // CMPLX_IC_2
      "00000"                           // CMPLX_IC_BPP
      "TAY"                             // CMPLX_WEIGHT
      "40"                              // CMPLX_AZ_SLL
      "40"                              // CMPLX_RNG_SLL
      "12"                              // CMPLX_AZ_TAY_NBAR
      "12"                              // CMPLX_RNG_TAY_NBAR
      "   "                             // CMPLX_WEIGHT_NORM
      "G"                               // CMPLX_SIGNAL_PLANE
      "000000"                          // IF_DC_SF_ROW
      "000000"                          // IF_DC_SF_COL
      "000000"                          // IF_PATCH_1_ROW
      "000000"                          // IF_PATCH_1_COL
      "000000"                          // IF_PATCH_2_ROW
      "000000"                          // IF_PATCH_2_COL
      "000000"                          // IF_PATCH_3_ROW
      "000000"                          // IF_PATCH_3_COL
      "000000"                          // IF_PATCH_4_ROW
      "000000"                          // IF_PATCH_4_COL
      "00000000"                        // IF_DC_IS_ROW
      "00000000"                        // IF_DC_IS_COL
      "00000000"                        // IF_IMG_ROW_DC
      "00000000"                        // IF_IMG_COL_DC
      "000000"                          // IF_TILE_1_ROW
      "000000"                          // IF_TILE_1_COL
      "000000"                          // IF_TILE_2_ROW
      "000000"                          // IF_TILE_2_COL
      "000000"                          // IF_TILE_3_ROW
      "000000"                          // IF_TILE_3_COL
      "000000"                          // IF_TILE_4_ROW
      "000000"                          // IF_TILE_4_COL
      "N"                               // IF_RD
      "O"                               // IF_RGWLK
      "N"                               // IF_KEYSTN
      "O"                               // IF_LINSFT
      "O"                               // IF_SUBPATCH
      "N"                               // IF_GEODIST
      "N"                               // IF_RGFO
      "N"                               // IF_BEAM_COMP
      "0009.999"                        // IF_RGRES
      "0009.999"                        // IF_AZRES
      "09.99999"                        // IF_RSS
      "09.99999"                        // IF_AZSS
      "09.99999"                        // IF_RSR
      "09.99999"                        // IF_AZSR
      "0000256"                         // IF_RFFT_SAMP
      "0000256"                         // IF_AZFFT_SAMP
      "0000400"                         // IF_RFFT_TOT
      "0000400"                         // IF_AZFFT_TOT
      "000000"                          // IF_SUBP_ROW
      "000000"                          // IF_SUBP_COL
      "0000"                            // IF_SUB_RG
      "0000"                            // IF_SUB_AZ
      "+"                               // IF_RFFTS
      "+"                               // IF_AFFTS
      "ROW_INC"                         // IF_RANGE_DATA
      "+"                               // IF_INCPH
      "        "                        // IF_SR_NAME1
      "01.00000"                        // IF_SR_AMOUNT1
      "        "                        // IF_SR_NAME2
      "01.00000"                        // IF_SR_AMOUNT2
      "        "                        // IF_SR_NAME3
      "01.00000"                        // IF_SR_AMOUNT
      "N    "                           // AF_TYPE1
      "N    "                           // AF_TYPE2
      "N    "                           // AF_TYPE3
      "V"                               // POL_TR
      "V"                               // POL_RE
      "                                        " // POL_REFERENCE
      "P"                               // POL
      "Y"                               // POL_REG
      "00.00"                           // POL_ISO_1
      "A"                               // POL_BAL
      "0.000000"                        // POL_BAL_MAG
      "+0.00000"                        // POL_BAL_PHS
      " "                               // POL_HCOMP
      "          "                      // P_HCOMP_BASIS
      "000000000"                       // POL_HCOMP_COEF_1
      "000000000"                       // POL_HCOMP_COEF_2
      "000000000"                       // POL_HCOMP_COEF_3
      "A"                               // POL_AFCOMP
      "               "                 // POL_SPARE_A
      "000000000"                       // POL_SPARE_N
      "1999OCT31"                       // T_UTC_YYYYMMMDD
      "003045"                          // T_HHMMSSUTC
      "123045"                          // T_HHMMSSLOCAL
      "09999999.99"                     // CG_SRAC
      "0000.00"                         // CG_SLANT_CONFIDENCE
      "00000000.00"                     // CG_CROSS
      "0000.00"                         // CG_CROSS_CONFIDENCE
      "+010.0000"                       // CG_CAAC
      "0.0000"                          // CG_CONE_CONFIDENCE
      "+00.0000"                        // CG_GPSAC
      "0.0000"                          // CG_GPSAC_CONFIDENCE
      "+09.9999"                        // CG_SQUINT
      "09.9999"                         // CG_GAAC
      "0.0000"                          // CG_GAAC_CONFIDENCE
      "09.9999"                         // CG_INCIDENT
      "0000000"                         // CG_SLOPE
      "+09.9999"                        // CG_TILT
      "R"                               // CG_LD
      "009.9999"                        // CG_NORTH
      "0.0000"                          // CG_NORTH_CONFIDENCE
      "000.0000"                        // CG_EAST
      "000.0000"                        // CG_RLOS
      "0.0000"                          // CG_LOS_CONFIDENCE
      "009.9999"                        // CG_LAYOVER
      "000.0000"                        // CG_SHADOW
      "000.000"                         // CG_OPM
      "ECEF "                           // CG_MODEL
      "+00000000.000"                   // CG_AMPT_X
      "+00000000.000"                   // CG_AMPT_Y
      "+00000000.000"                   // CG_AMPT_Z
      "000000"                          // CG_AP_CONF_XY
      "000000"                          // CG_AP_CONF_Z
      "+00999999.999"                   // CG_APCEN_X
      "+00999999.999"                   // CG_APCEN_Y
      "+00999999.999"                   // CG_APCEN_Z
      "000.00"                          // CG_APER_CONF_XY
      "000.00"                          // CG_APER_CONF_Z
      "-0.999999"                       // CG_FPNUV_X
      "+0.999999"                       // CG_FPNUV_Y
      "+0.999999"                       // CG_FPNUV_Z
      "-0.999999"                       // CG_IDPNUVX
      "+0.999999"                       // CG_IDPNUVY
      "+0.999999"                       // CG_IDPNUVZ
      "-00999999.999"                   // CG_SCECN_X
      "+09999999.999"                   // CG_SCECN_Y
      "+09999999.999"                   // CG_SCECN_Z
      "000.00"                          // CG_SC_CONF_XY
      "000.00"                          // CG_SC_CONF_Z
      "00000.00"                        // CG_SWWD
      "-09999.999"                      // CG_SNVEL_X
      "-09999.999"                      // CG_SNVEL_Y
      "+09999.999"                      // CG_SNVEL_Z
      "+00.000000"                      // CG_SNACC_X
      "+00.000000"                      // CG_SNACC_Y
      "+00.000000"                      // CG_SNACC_Z
      "+000.000"                        // CG_SNATT_ROLL
      "+000.000"                        // CG_SNATT_PITCH
      "+000.000"                        // CG_SNATT_YAW
      "+0.000000"                       // CG_GTP_X
      "+0.000000"                       // CG_GTP_Y
      "+0.000000"                       // CG_GTP_Z

      "GEOD"                            // CG_MAP_TYPE      //ERROR: incorrect CG_MAP_TYPE

      // start conditional:  CG_MAP_TYPE == "NA"
      "                                                                                                               "
      "                      "       // CG_MAP_TYPE_BLANK
      // end conditional:  CG_MAP_TYPE == "NA"

      "                                                                                                               "
      "                                 "   // CG_SPARE_A
      "0000000"                         // CA_CALPA
      "000000000000.0"                  // WF_SRTFR
      "000000000000.0"                  // WF_ENDFR
      "+00.000000"                      // WF_CHRPRT
      "0.0000000"                       // WF_WIDTH
      "09999999999.9"                   // WF_CENFRQ
      "00000000000.0"                   // WF_BW
      "00000.0"                         // WF_PRF
      "0.0000000"                       // WF_PRI
      "099.990"                         // WF_CDP
      "000009999"                       // WF_NUMBER_OF_PULSES
      "Y"                               // VPH_COND
      );


   static const string data_error6(
      "00"                              // NUM_RELATED_TRES
      "                                                                                                               "
      "         " // RELATED_TRES
      "            "                    // RD_RPC_NO
      "PF  "                            // IF_PROCESS
      "X   "                            // RD_CEN_FREQ
      "1FG  "                           // RD_MODE
      "0001"                            // RD_PATCH_NO
      "M1P2 "                           // CMPLX_DOMAIN
      "LLM "                            // CMPLX_MAG_REMAP_TYPE
      "1.94369"                         // CMPLX_LIN_SCALE
      "0000000"                         // CMPLX_AVG_POWER
      "00117"                           // CMPLX_LINLOG_TP
      "UQ1"                             // CMPLX_PHASE_QUANT_FLAG
      "00"                              // CMPLX_PHASE_QUANT_BIT_DEPTH
      "08"                              // CMPLX_SIZE_1
      "NC"                              // CMPLX_IC_1
      "08"                              // CMPLX_SIZE_2
      "NC"                              // CMPLX_IC_2
      "00000"                           // CMPLX_IC_BPP
      "TAY"                             // CMPLX_WEIGHT
      "40"                              // CMPLX_AZ_SLL
      "40"                              // CMPLX_RNG_SLL
      "12"                              // CMPLX_AZ_TAY_NBAR
      "12"                              // CMPLX_RNG_TAY_NBAR
      "   "                             // CMPLX_WEIGHT_NORM
      "G"                               // CMPLX_SIGNAL_PLANE
      "000000"                          // IF_DC_SF_ROW
      "000000"                          // IF_DC_SF_COL
      "000000"                          // IF_PATCH_1_ROW
      "000000"                          // IF_PATCH_1_COL
      "000000"                          // IF_PATCH_2_ROW
      "000000"                          // IF_PATCH_2_COL
      "000000"                          // IF_PATCH_3_ROW
      "000000"                          // IF_PATCH_3_COL
      "000000"                          // IF_PATCH_4_ROW
      "000000"                          // IF_PATCH_4_COL
      "00000000"                        // IF_DC_IS_ROW
      "00000000"                        // IF_DC_IS_COL
      "00000000"                        // IF_IMG_ROW_DC
      "00000000"                        // IF_IMG_COL_DC
      "000000"                          // IF_TILE_1_ROW
      "000000"                          // IF_TILE_1_COL
      "000000"                          // IF_TILE_2_ROW
      "000000"                          // IF_TILE_2_COL
      "000000"                          // IF_TILE_3_ROW
      "000000"                          // IF_TILE_3_COL
      "000000"                          // IF_TILE_4_ROW
      "000000"                          // IF_TILE_4_COL
      "N"                               // IF_RD
      "O"                               // IF_RGWLK
      "N"                               // IF_KEYSTN
      "O"                               // IF_LINSFT
      "O"                               // IF_SUBPATCH
      "N"                               // IF_GEODIST
      "N"                               // IF_RGFO
      "N"                               // IF_BEAM_COMP
      "0009.999"                        // IF_RGRES
      "0009.999"                        // IF_AZRES
      "09.99999"                        // IF_RSS
      "09.99999"                        // IF_AZSS
      "09.99999"                        // IF_RSR
      "09.99999"                        // IF_AZSR
      "0000256"                         // IF_RFFT_SAMP
      "0000256"                         // IF_AZFFT_SAMP
      "0000400"                         // IF_RFFT_TOT
      "0000400"                         // IF_AZFFT_TOT
      "000000"                          // IF_SUBP_ROW
      "000000"                          // IF_SUBP_COL
      "0000"                            // IF_SUB_RG
      "0000"                            // IF_SUB_AZ
      "+"                               // IF_RFFTS
      "+"                               // IF_AFFTS
      "ROW_INC"                         // IF_RANGE_DATA
      "+"                               // IF_INCPH
      "        "                        // IF_SR_NAME1
      "01.00000"                        // IF_SR_AMOUNT1
      "        "                        // IF_SR_NAME2
      "01.00000"                        // IF_SR_AMOUNT2
      "        "                        // IF_SR_NAME3
      "01.00000"                        // IF_SR_AMOUNT
      "N    "                           // AF_TYPE1
      "N    "                           // AF_TYPE2
      "N    "                           // AF_TYPE3
      "V"                               // POL_TR
      "V"                               // POL_RE
      "                                        " // POL_REFERENCE
      "P"                               // POL
      "Y"                               // POL_REG
      "00.00"                           // POL_ISO_1
      "A"                               // POL_BAL
      "0.000000"                        // POL_BAL_MAG
      "+0.00000"                        // POL_BAL_PHS
      " "                               // POL_HCOMP
      "          "                      // P_HCOMP_BASIS
      "000000000"                       // POL_HCOMP_COEF_1
      "000000000"                       // POL_HCOMP_COEF_2
      "000000000"                       // POL_HCOMP_COEF_3
      "A"                               // POL_AFCOMP
      "               "                 // POL_SPARE_A
      "000000000"                       // POL_SPARE_N
      "1999OCT31"                       // T_UTC_YYYYMMMDD
      "003045"                          // T_HHMMSSUTC
      "123045"                          // T_HHMMSSLOCAL
      "09999999.99"                     // CG_SRAC
      "0000.00"                         // CG_SLANT_CONFIDENCE
      "00000000.00"                     // CG_CROSS
      "0000.00"                         // CG_CROSS_CONFIDENCE
      "+010.0000"                       // CG_CAAC
      "0.0000"                          // CG_CONE_CONFIDENCE
      "+00.0000"                        // CG_GPSAC
      "0.0000"                          // CG_GPSAC_CONFIDENCE
      "+09.9999"                        // CG_SQUINT
      "09.9999"                         // CG_GAAC
      "0.0000"                          // CG_GAAC_CONFIDENCE
      "09.9999"                         // CG_INCIDENT
      "0000000"                         // CG_SLOPE
      "+09.9999"                        // CG_TILT
      "R"                               // CG_LD
      "009.9999"                        // CG_NORTH
      "0.0000"                          // CG_NORTH_CONFIDENCE
      "000.0000"                        // CG_EAST
      "000.0000"                        // CG_RLOS
      "0.0000"                          // CG_LOS_CONFIDENCE
      "009.9999"                        // CG_LAYOVER
      "000.0000"                        // CG_SHADOW
      "000.000"                         // CG_OPM
      "ECEF "                           // CG_MODEL
      "+00000000.000"                   // CG_AMPT_X
      "+00000000.000"                   // CG_AMPT_Y
      "+00000000.000"                   // CG_AMPT_Z
      "000000"                          // CG_AP_CONF_XY
      "000000"                          // CG_AP_CONF_Z
      "+00999999.999"                   // CG_APCEN_X
      "+00999999.999"                   // CG_APCEN_Y
      "+00999999.999"                   // CG_APCEN_Z
      "000.00"                          // CG_APER_CONF_XY
      "000.00"                          // CG_APER_CONF_Z
      "-0.999999"                       // CG_FPNUV_X
      "+0.999999"                       // CG_FPNUV_Y
      "+0.999999"                       // CG_FPNUV_Z
      "-0.999999"                       // CG_IDPNUVX
      "+0.999999"                       // CG_IDPNUVY
      "+0.999999"                       // CG_IDPNUVZ
      "-00999999.999"                   // CG_SCECN_X
      "+09999999.999"                   // CG_SCECN_Y
      "+09999999.999"                   // CG_SCECN_Z
      "000.00"                          // CG_SC_CONF_XY
      "000.00"                          // CG_SC_CONF_Z
      "00000.00"                        // CG_SWWD
      "-09999.999"                      // CG_SNVEL_X
      "-09999.999"                      // CG_SNVEL_Y
      "+09999.999"                      // CG_SNVEL_Z
      "+00.000000"                      // CG_SNACC_X
      "+00.000000"                      // CG_SNACC_Y
      "+00.000000"                      // CG_SNACC_Z
      "+000.000"                        // CG_SNATT_ROLL
      "+000.000"                        // CG_SNATT_PITCH
      "+000.000"                        // CG_SNATT_YAW
      "+0.000000"                       // CG_GTP_X
      "+0.000000"                       // CG_GTP_Y
      "+0.000000"                       // CG_GTP_Z

      "JUNK"                            // CG_MAP_TYPE      // ERROR: INVALID CG_MAP_TYPE type

      // start conditional:  CG_MAP_TYPE == "NA"
      "                                                                                                               "
      "                      "       // CG_MAP_TYPE_BLANK
      // end conditional:  CG_MAP_TYPE == "NA"

      "                                                                                                               "
      "                                 "   // CG_SPARE_A
      "0000000"                         // CA_CALPA
      "000000000000.0"                  // WF_SRTFR
      "000000000000.0"                  // WF_ENDFR
      "+00.000000"                      // WF_CHRPRT
      "0.0000000"                       // WF_WIDTH
      "09999999999.9"                   // WF_CENFRQ
      "00000000000.0"                   // WF_BW
      "00000.0"                         // WF_PRF
      "0.0000000"                       // WF_PRI
      "099.990"                         // WF_CDP
      "000009999"                       // WF_NUMBER_OF_PULSES
      "Y"                               // VPH_COND
      );


   FactoryResource<DynamicObject> treDO;
   size_t numBytes(0);

   istringstream input(data);
   numBytes = data.size();
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
   stringstream input2(data);
   input2 << "1";          // Add one more byte; valid as alphanumberic or numeric
   numBytes = data.size() + 1;
   success = toDynamicObject(input2, numBytes, *treDO.get(), errorMessage);
   if (success == true)     // negative test so success must == false.
   {
      failure << "Error: Negative test of 1 extra byte failed\n";
      treDO->clear();
      return false;
   }

   // start of test 3 - Negative test: 1 byte short in input stream
   string negdata3(data);        // data for test 3 not the 3rd data set
   negdata3.resize(data.size()-1);
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


   stringstream input4(data_error4);
   numBytes = input4.str().size();

   success = toDynamicObject(input4, numBytes, *treDO.get(), errorMessage);

   status = INVALID;
   if (success)
   {
      stringstream tmpStream;
      status = isTreValid(*treDO.get(), tmpStream);
      if (status != INVALID)
      {
         failure << "Error: Negative test with all blank string failed: did not return INVALID\n";
         failure << tmpStream.str();
         treDO->clear();
         return false;
      }
      status = VALID;
   }

   stringstream input5(data_error5);
   numBytes = input5.str().size();

   success = toDynamicObject(input5, numBytes, *treDO.get(), errorMessage);
   if (success)
   {
      failure << "Error in negative test: incorrect CG_MAP_TYPE failed: should not have return true from the parser\n";
      treDO->clear();
      return false;
   }


   stringstream input6(data_error6);
   numBytes = input6.str().size();

   success = toDynamicObject(input6, numBytes, *treDO.get(), errorMessage);

   if (success)
   {
      failure << "Error: Negative test bad CG_MAP_TYPE type failed: should not have return true from the parser\n";
      treDO->clear();
      return false;
   }

   treDO->clear();

   return true;
}

bool Nitf::CmetaaParser::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output,
   string &errorMessage) const
{
   vector<char> buf;
   bool ok(true);
   bool success(true);

   // NOTE: The standard for CMETAA (STDI-0002) lists the name of the first two fields as "RELATED_TRES"
   // This was probably an error as the first field is a count of the related TREs. Because the DA
   // will not allow two fields to have the same name it has been changed to allow the storage into the DA

   readField<int>(input, output, success, CMETAA::NUM_RELATED_TRES, 2, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::RELATED_TRES, 120, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::RD_RPC_NO, 12, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_PROCESS, 4, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::RD_CEN_FREQ, 4, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::RD_MODE, 5, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::RD_PATCH_NO, 4, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_DOMAIN, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_MAG_REMAP_TYPE, 4, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CMPLX_LIN_SCALE, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CMPLX_AVG_POWER, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_LINLOG_TP, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_PHASE_QUANT_FLAG, 3, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_PHASE_QUANT_BIT_DEPTH, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_SIZE_1, 2, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_IC_1, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_SIZE_2, 2, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_IC_2, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_IC_BPP, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_WEIGHT, 3, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_AZ_SLL, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_RNG_SLL, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_AZ_TAY_NBAR, 2, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::CMPLX_RNG_TAY_NBAR, 2, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CMPLX_WEIGHT_NORM, 3, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::CMPLX_SIGNAL_PLANE, 1, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_DC_SF_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_DC_SF_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_1_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_1_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_2_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_2_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_3_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_3_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_4_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_PATCH_4_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_DC_IS_ROW, 8, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_DC_IS_COL, 8, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_IMG_ROW_DC, 8, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_IMG_COL_DC, 8, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_1_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_1_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_2_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_2_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_3_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_3_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_4_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_TILE_4_COL, 6, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_RD, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_RGWLK, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_KEYSTN, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_LINSFT, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_SUBPATCH, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_GEODIST, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_RGFO, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_BEAM_COMP, 1, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_RGRES, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_AZRES, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_RSS, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_AZSS, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_RSR, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::IF_AZSR, 8, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_RFFT_SAMP, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_AZFFT_SAMP, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_RFFT_TOT, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_AZFFT_TOT, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_SUBP_ROW, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_SUBP_COL, 6, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_SUB_RG, 4, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::IF_SUB_AZ, 4, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_RFFTS, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_AFFTS, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_RANGE_DATA, 7, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_INCPH, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::IF_SR_NAME1, 8, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::IF_SR_AMOUNT1, 8, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_SR_NAME2, 8, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::IF_SR_AMOUNT2, 8, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::IF_SR_NAME3, 8, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::IF_SR_AMOUNT, 8, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::AF_TYPE1, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::AF_TYPE2, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::AF_TYPE3, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_TR, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_RE, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_REFERENCE, 40, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::POL, 1, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_REG, 1, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::POL_ISO_1, 5, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_BAL, 1, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::POL_BAL_MAG, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::POL_BAL_PHS, 8, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_HCOMP, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::P_HCOMP_BASIS, 10, errorMessage, buf, true);
   readField<int>(input, output, success, CMETAA::POL_HCOMP_COEF_1, 9, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::POL_HCOMP_COEF_2, 9, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::POL_HCOMP_COEF_3, 9, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::POL_AFCOMP, 1, errorMessage, buf, true);
   readField<string>(input, output, success, CMETAA::POL_SPARE_A, 15, errorMessage, buf, true);
   readField<int>(input, output, success, CMETAA::POL_SPARE_N, 9, errorMessage, buf);

   // T_UTC_YYYYMMMDD and T_HHMMSSUTC can be combined into one DateTime field: T_UTC_YYYYMMMDD_HHMMSS
   success = success && readFromStream(input, buf, 15);  // YYYYMMMDDhhmmss

   string dtg;
   dtg.resize(16);
   memcpy(&dtg[0], &buf[0], 16);

   unsigned short year(0);
   unsigned short month(0);
   unsigned short day(0);
   unsigned short hour(0);
   unsigned short min(0);
   unsigned short sec(0);

   bool dtgValid = DtgParseCCYYMMMDDhhmmss(dtg, year, month, day, hour, min, sec);
   if (dtgValid)
   {
      FactoryResource<DateTime> appDTG;
      dtgValid = appDTG->set(year, month, day, hour, min, sec);
      if (dtgValid)
      {
         success = success && output.setAttribute(CMETAA::T_UTC_YYYYMMMDD_HHMMSS, *appDTG.get());
      }
   }

   // Need to handle time only without the date.
   // T_HHMMSSLOCAL is the local time of T_HHMMSSUTC and may not be on the same date as the UTC version
   readField<string>(input, output, success, CMETAA::T_HHMMSSLOCAL, 6, errorMessage, buf, true);

   readField<double>(input, output, success, CMETAA::CG_SRAC, 11, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SLANT_CONFIDENCE, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_CROSS, 11, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_CROSS_CONFIDENCE, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_CAAC, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_CONE_CONFIDENCE, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GPSAC, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GPSAC_CONFIDENCE, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SQUINT, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GAAC, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GAAC_CONFIDENCE, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_INCIDENT, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SLOPE, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_TILT, 8, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CG_LD, 1, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_NORTH, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_NORTH_CONFIDENCE, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_EAST, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_RLOS, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_LOS_CONFIDENCE, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_LAYOVER, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SHADOW, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_OPM, 7, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::CG_MODEL, 5, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_AMPT_X, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_AMPT_Y, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_AMPT_Z, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_AP_CONF_XY, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_AP_CONF_Z, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_APCEN_X, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_APCEN_Y, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_APCEN_Z, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_APER_CONF_XY, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_APER_CONF_Z, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_FPNUV_X, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_FPNUV_Y, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_FPNUV_Z, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_IDPNUVX, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_IDPNUVY, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_IDPNUVZ, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SCECN_X, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SCECN_Y, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SCECN_Z, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SC_CONF_XY, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SC_CONF_Z, 6, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SWWD, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNVEL_X, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNVEL_Y, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNVEL_Z, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNACC_X, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNACC_Y, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNACC_Z, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNATT_ROLL, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNATT_PITCH, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_SNATT_YAW, 8, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GTP_X, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GTP_Y, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::CG_GTP_Z, 9, errorMessage, buf);

   // Test value for following conditionals
   success = success && readFromStream(input, buf, 4) &&
      output.setAttribute(CMETAA::CG_MAP_TYPE, StringUtilities::stripWhitespace(string(&buf.front())));

   string map_type(StringUtilities::stripWhitespace(&buf.front()));

   if (map_type == "GEOD")
   {
      readField<double>(input, output, success, CMETAA::CG_PATCH_LATCEN, 11, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LNGCEN, 12, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LTCORUL, 11, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LGCORUL, 12, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LTCORUR, 11, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LGCORUR, 12, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LTCORLR, 11, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LGCORLR, 12, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LTCORLL, 11, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LNGCOLL, 12, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LAT_CONFIDENCE, 9, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_PATCH_LON_CONFIDENCE, 9, errorMessage, buf);
   }
   else if (map_type == "MGRS")
   {
      readField<string>(input, output, success, CMETAA::CG_MGRS_CENT, 23, errorMessage, buf);
      readField<string>(input, output, success, CMETAA::CG_MGRSCORUL, 23, errorMessage, buf);
      readField<string>(input, output, success, CMETAA::CG_MGRSCORUR, 23, errorMessage, buf);
      readField<string>(input, output, success, CMETAA::CG_MGRSCORLR, 23, errorMessage, buf);
      readField<string>(input, output, success, CMETAA::CG_MGRSCORLL, 23, errorMessage, buf);
      readField<double>(input, output, success, CMETAA::CG_MGRS_CONFIDENCE, 7, errorMessage, buf);
      readField<string>(input, output, success, CMETAA::CG_MGRS_PAD, 11, errorMessage, buf);
   }
   else if (map_type == "NA")
   {
      readField<string>(input, output, success, CMETAA::CG_MAP_TYPE_BLANK, 133, errorMessage, buf, true);
   }
   else
   {
      // ERROR map_type should always be one of the 3 values above.
      success = false;
   }

   // end conditional

   readField<string>(input, output, success, CMETAA::CG_SPARE_A, 144, errorMessage, buf, true);
   readField<double>(input, output, success, CMETAA::CA_CALPA, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_SRTFR, 14, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_ENDFR, 14, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_CHRPRT, 10, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_WIDTH, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_CENFRQ, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_BW, 13, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_PRF, 7, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_PRI, 9, errorMessage, buf);
   readField<double>(input, output, success, CMETAA::WF_CDP, 7, errorMessage, buf);
   readField<int>(input, output, success, CMETAA::WF_NUMBER_OF_PULSES, 9, errorMessage, buf);
   readField<string>(input, output, success, CMETAA::VPH_COND, 1, errorMessage, buf);

   size_t numRead = input.tellg();
   if (numRead != numBytes)
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

/*****************************************************************************/

Nitf::TreState Nitf::CmetaaParser::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   TreState status(VALID);
   set<string>             testSet;
   unsigned int numFields = 0;

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "NUM_RELATED_TRES", 0, 9999));

   testSet.clear();
   testSet.insert("AIMIDA");
   testSet.insert("AIMIDB");
   testSet.insert("MTXFIL");
   testSet.insert("AIPBCA");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RELATED_TRES", testSet, true, true, false));

   testSet.clear();
   testSet.insert("");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RD_RPC_NO", testSet, true, true, false));

   testSet.clear();
   testSet.insert("RM");
   testSet.insert("PF");
   testSet.insert("CD");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_PROCESS", testSet, false, true, true));

   testSet.clear();
   testSet.insert("L");
   testSet.insert("C");
   testSet.insert("P");
   testSet.insert("S");
   testSet.insert("SC");
   testSet.insert("X");
   testSet.insert("KA");
   testSet.insert("KU");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RD_CEN_FREQ", testSet, false, true, true));

   testSet.clear();
   testSet.insert("0FR");
   testSet.insert("0FG");
   testSet.insert("1FR");
   testSet.insert("1FG");
   testSet.insert("2FR");
   testSet.insert("2FG");
   testSet.insert("22FR");
   testSet.insert("22FG");
   testSet.insert("07A");
   testSet.insert("07L");
   testSet.insert("14A");
   testSet.insert("14L");
   testSet.insert("1SP");
   testSet.insert("3SP");
   testSet.insert("10S");
   testSet.insert("GSP");
   testSet.insert("GSH");
   testSet.insert("AIP13");
   testSet.insert("AIP14");
   testSet.insert("AIP15");
   testSet.insert("AIP16");
   testSet.insert("AIP17");
   testSet.insert("AIP18");
   testSet.insert("AIP19");
   testSet.insert("AIP20");
   testSet.insert("AS201");
   testSet.insert("AS202");
   testSet.insert("AS204");
   testSet.insert("AS207");
   testSet.insert("AS208");
   testSet.insert("AS209");
   testSet.insert("AS210");
   testSet.insert("AS211");
   testSet.insert("AS212");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "RD_MODE", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "RD_PATCH_NO", 0, 9999));

   testSet.clear();
   testSet.insert("IQ");
   testSet.insert("QI");
   testSet.insert("MP");
   testSet.insert("I1Q2");
   testSet.insert("Q1I2");
   testSet.insert("M1P2");
   testSet.insert("P1M2");
   testSet.insert("M");
   testSet.insert("P");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_DOMAIN", testSet, false, true, true));

   testSet.clear();
   testSet.insert("NS");
   testSet.insert("LINM");
   testSet.insert("LINP");
   testSet.insert("LOGM");
   testSet.insert("LOGP");
   testSet.insert("LLM");
   testSet.insert("or");
   testSet.insert("NA");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_MAG_REMAP_TYPE", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CMPLX_LIN_SCALE", 0.000001, 99999.9));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CMPLX_AVG_POWER", 0.000000, 99999.9));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_LINLOG_TP", 0U, 65535U));

   testSet.clear();
   testSet.insert("NS");
   testSet.insert("UQ1");
   testSet.insert("UQ2");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_PHASE_QUANT_FLAG", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_PHASE_QUANT_BIT_DEPTH", 0U, 32U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_SIZE_1", 4U, 64U));

   testSet.clear();
   testSet.insert("NC");
   testSet.insert("C3");
   testSet.insert("C5");
   testSet.insert("C6");
   testSet.insert("I1");
   testSet.insert("C4");
   testSet.insert("C7");
   testSet.insert("TC");
   testSet.insert("NS");
   testSet.insert("US");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_IC_1", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_SIZE_2", 4U, 64U));

   testSet.clear();
   testSet.insert("NC");
   testSet.insert("C3");
   testSet.insert("C5");
   testSet.insert("C6");
   testSet.insert("I1");
   testSet.insert("C4");
   testSet.insert("C7");
   testSet.insert("TC");
   testSet.insert("NS");
   testSet.insert("US");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_IC_2", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_IC_BPP", 0, 64));

   testSet.clear();
   testSet.insert("UWT");
   testSet.insert("SVA");
   testSet.insert("TAY");
   testSet.insert("HNW");
   testSet.insert("HMW");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_WEIGHT", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_AZ_SLL", 0U, 99U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_RNG_SLL", 0U, 99U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_AZ_TAY_NBAR", 0U, 99U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "CMPLX_RNG_TAY_NBAR", 0U, 99U));

   testSet.clear();
   testSet.insert("AVG");
   testSet.insert("RMS");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_WEIGHT_NORM", testSet, true, true, true));

   testSet.clear();
   testSet.insert("S");
   testSet.insert("G");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CMPLX_SIGNAL_PLANE", testSet, false, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_DC_SF_ROW", 0U, 999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_DC_SF_COL", 0U, 999999U));

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_1_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_1_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }

      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_1_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_1_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_2_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_2_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_2_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_2_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_3_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_3_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_3_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_3_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_4_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_4_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_4_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_PATCH_4_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "IF_DC_IS_ROW", 0U, 99999999U));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "IF_DC_IS_COL", 0U, 99999999U));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "IF_IMG_ROW_DC", 0U, 99999999U));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "IF_IMG_COL_DC", 0U, 99999999U));

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_1_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_1_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_1_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_1_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_2_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_2_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_2_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_2_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_3_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_3_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_3_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_3_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_4_ROW", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_4_ROW", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_4_COL", 0U, 999999U);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<int>(tre, tempStream, NULL, "IF_TILE_4_COL", -99999, -99999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_RD", testSet, false, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_RGWLK", testSet, false, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_KEYSTN", testSet, true, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_LINSFT", testSet, true, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_SUBPATCH", testSet, true, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_GEODIST", testSet, true, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_RGFO", testSet, false, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   testSet.insert("O");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_BEAM_COMP", testSet, false, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_RGRES", 0.0, 9999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_AZRES", 0.0, 9999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_RSS", 0.0, 99.99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_AZSS", 0.0, 99.99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_RSR", 0.0, 99.99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_AZSR", 0.0, 99.99999));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_RFFT_SAMP", 1U, 9999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_AZFFT_SAMP", 1U, 9999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_RFFT_TOT", 1U, 9999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_AZFFT_TOT", 1U, 9999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_SUBP_ROW", 0U, 999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_SUBP_COL", 0U, 999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_SUB_RG", 0U, 1000U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "IF_SUB_AZ", 0U, 1000U));

   testSet.clear();
   testSet.insert("+");
   testSet.insert("-");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_RFFTS", testSet, false, false, true));

   testSet.clear();
   testSet.insert("+");
   testSet.insert("-");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_AFFTS", testSet, false, false, true));

   testSet.clear();
   testSet.insert("ROW_INC");
   testSet.insert("ROW_DEC");
   testSet.insert("COL_INC");
   testSet.insert("COL_DEC");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_RANGE_DATA", testSet, false, false, true));

   testSet.clear();
   testSet.insert("+");
   testSet.insert("-");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_INCPH", testSet, true, false, true));


   testSet.clear();
   testSet.insert("S-SVA");
   testSet.insert("NLS");
   testSet.insert("HDI");
   testSet.insert("HDSAR");
   testSet.insert("CLEAN");
   testSet.insert("SPECEST");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_SR_NAME1", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_SR_AMOUNT1", 1.0, 99.99999));

   testSet.clear();
   testSet.insert("S-SVA");
   testSet.insert("NLS");
   testSet.insert("HDI");
   testSet.insert("HDSAR");
   testSet.insert("CLEAN");
   testSet.insert("SPECEST");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_SR_NAME2", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_SR_AMOUNT2", 1.0, 99.99999));

   testSet.clear();
   testSet.insert("S-SVA");
   testSet.insert("NLS");
   testSet.insert("HDI");
   testSet.insert("HDSAR");
   testSet.insert("CLEAN");
   testSet.insert("SPECEST");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "IF_SR_NAME3", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "IF_SR_AMOUNT", 1.0, 99.99999));

   testSet.clear();
   testSet.insert("N");
   testSet.insert("MD");
   testSet.insert("PGA");
   testSet.insert("PHDIF");
   testSet.insert("HOAF");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "AF_TYPE1", testSet, false, false, true));

   testSet.clear();
   testSet.insert("N");
   testSet.insert("MD");
   testSet.insert("PGA");
   testSet.insert("PHDIF");
   testSet.insert("HOAF");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "AF_TYPE2", testSet, false, false, true));

   testSet.clear();
   testSet.insert("N");
   testSet.insert("MD");
   testSet.insert("PGA");
   testSet.insert("PHDIF");
   testSet.insert("HOAF");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "AF_TYPE3", testSet, false, false, true));

   testSet.clear();
   testSet.insert("H");
   testSet.insert("V");
   testSet.insert("L");
   testSet.insert("R");
   testSet.insert("T");
   testSet.insert("P");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_TR", testSet, false, false, true));

   testSet.clear();
   testSet.insert("H");
   testSet.insert("V");
   testSet.insert("L");
   testSet.insert("R");
   testSet.insert("T");
   testSet.insert("P");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_RE", testSet, false, false, true));

   testSet.clear();
   testSet.insert("ANT");
   testSet.insert("SCN");
   testSet.insert("XYZ");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_REFERENCE", testSet, true, false, true));

   testSet.clear();
   testSet.insert("P");
   testSet.insert("D");
   testSet.insert("N");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL", testSet, false, false, true));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_REG", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "POL_ISO_1", 0.0, 99.99));

   testSet.clear();
   testSet.insert("A");
   testSet.insert("B");
   testSet.insert("C");
   testSet.insert("U");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_BAL", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "POL_BAL_MAG", 0.0, 0.999999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "POL_BAL_PHS", 0.0, 9.999999));

   testSet.clear();
   testSet.insert("A");
   testSet.insert("B");
   testSet.insert("C");
   testSet.insert("U");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_HCOMP", testSet, true, false, true));

   testSet.clear();
   testSet.insert("LEGENDRE");
   testSet.insert("POLYNOMIAL");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "P_HCOMP_BASIS", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "POL_HCOMP_COEF_1", -99999999, 999999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "POL_HCOMP_COEF_2", -99999999, 999999999U));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "POL_HCOMP_COEF_3", -99999999, 999999999U));

   testSet.clear();
   testSet.insert("A");
   testSet.insert("D");
   testSet.insert("M");
   testSet.insert("N");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_AFCOMP", testSet, true, false, true));

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "POL_SPARE_A", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<int>(tre, reporter,
      &numFields, "POL_SPARE_N", 0U, 0U));

   // T_UTC_YYYYMMMDD and T_HHMMSSUTC can be combined into one DateTime field: T_UTC_YYYYMMMDD_HHMMSS
   if (status != INVALID)
   {
      if (tre.getAttribute("T_UTC_YYYYMMMDD_HHMMSS").getPointerToValue<DateTime>() == NULL)
      {
         reporter << "Field \"" << "T_UTC_YYYYMMMDD_HHMMSS" << "\" missing from the Dynamic Object";
         status = INVALID;
      }
      else
      {
         ++numFields;
      }
   }

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "T_HHMMSSLOCAL", testSet, true, true, false));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SRAC", 0.0, 99999999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SLANT_CONFIDENCE", 0.0, 9999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_CROSS", 0.0, 99999999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_CROSS_CONFIDENCE", 0.0, 9999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_CAAC", -999.9999, 999.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_CONE_CONFIDENCE", 0.0, 0.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_GPSAC", -89.0, 89.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_GPSAC_CONFIDENCE", 0.0, 0.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SQUINT", -89.0, 89.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_GAAC", 0.0, 89.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_GAAC_CONFIDENCE", 0.0, 9999.9));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_INCIDENT", 0.0, 89.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SLOPE", 0.0, 89.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_TILT", -44.9999, 44.9999));

   testSet.clear();
   testSet.insert("L");
   testSet.insert("R");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CG_LD", testSet, false, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_NORTH", 0.0, 359.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_NORTH_CONFIDENCE", 0.0, 9.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_EAST", 0.0, 359.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_RLOS", 0.0, 359.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_LOS_CONFIDENCE", 0.0, 9.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_LAYOVER", 0.0, 359.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SHADOW", 0.0, 359.9999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_OPM", 0.0, 999.999));

   testSet.clear();
   testSet.insert("XYZSC");
   testSet.insert("ECEF");
   testSet.insert("WGS84");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CG_MODEL", testSet, false, true, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_AMPT_X", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_AMPT_Y", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_AMPT_Z", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_AP_CONF_XY", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_AP_CONF_Z", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_APCEN_X", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_APCEN_Y", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_APCEN_Z", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_APER_CONF_XY", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_APER_CONF_Z", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_FPNUV_X", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_FPNUV_Y", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_FPNUV_Z", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_IDPNUVX", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_IDPNUVY", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_IDPNUVZ", -1.0, 1.0));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SCECN_X", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SCECN_Y", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SCECN_Z", -99999999.999, 99999999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SC_CONF_XY", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SC_CONF_Z", 0.0, 999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SWWD", 0.0, 99999.99));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNVEL_X", -99999.999, 99999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNVEL_Y", -99999.999, 99999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNVEL_Z", -99999.999, 99999.999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNACC_X", -99.999999, 99.999999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNACC_Y", -99.999999, 99.99999));

   status = MaxState(status, testTagValueRange<double>(tre, reporter,
      &numFields, "CG_SNACC_Z", -99.999999, 99.99999));

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<double>(tre, tempStream, NULL, "CG_SNATT_ROLL", -179.999, 179.999);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<double>(tre, tempStream, NULL, "CG_SNATT_ROLL", -9999999, -9999999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   if (status != INVALID)
   {
      stringstream tempStream;
      TreState tmpstatus = testTagValueRange<double>(tre, tempStream, NULL, "CG_SNATT_PITCH", -179.999, 179.999);
      if (tmpstatus != VALID)
      {
         tmpstatus = MaxState(tmpstatus,
            testTagValueRange<double>(tre, tempStream, NULL, "CG_SNATT_ROLL", -9999999, -9999999));
      }

      status = MaxState(tmpstatus, status);
      if (tmpstatus != VALID)
      {
         string tmpStr(tempStream.str());
         reporter << tmpStr;
      }
      ++numFields;
   }

   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "CG_SNATT_YAW", -359.999, 359.999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "CG_GTP_X", -1.0, 1.0));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "CG_GTP_Y", -1.0, 1.0));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "CG_GTP_Z", -1.0, 1.0));

   testSet.clear();
   testSet.insert("GEOD");
   testSet.insert("MGRS");
   testSet.insert("NA");
   status = MaxState(status, testTagValidBcsASet(tre, reporter,
      &numFields, "CG_MAP_TYPE", testSet, false, false, true));

   string map_type;
   try
   {
      map_type = dv_cast<string>(tre.getAttribute("CG_MAP_TYPE"));
   }
   catch (const bad_cast&)
   {
      reporter << "Field \"" << "CG_MAP_TYPE" << "\" missing from the Dynamic Object";
      status = INVALID;
   }

   if (map_type == "GEOD")
   {
      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LATCEN", -89.9999999, 89.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LNGCEN", -179.9999999, 179.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LTCORUL", -89.9999999, 89.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LGCORUL", -179.9999999, 179.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LTCORUR", -89.9999999, 89.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LGCORUR", -179.9999999, 179.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LTCORLR", -89.9999999, 89.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LGCORLR", -179.9999999, 179.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LTCORLL", -89.9999999, 89.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LNGCOLL", -179.9999999, 179.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LAT_CONFIDENCE", 0.0, 9.9999999));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_PATCH_LON_CONFIDENCE", 0.0, 9.9999999));
   }
   else if (map_type == "MGRS")
   {
      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRS_CENT", testSet, false, true, true));

      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRSCORUL", testSet, false, true, true));

      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRSCORUR", testSet, false, true, true));

      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRSCORLR", testSet, false, true, true));

      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRSCORLL", testSet, false, true, true));

      status = MaxState(status, testTagValueRange<double>(tre, reporter,
         &numFields, "CG_MGRS_CONFIDENCE", 0.0, 9999.99));

      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MGRS_PAD", testSet, true, false, true));
   }
   else if (map_type == "NA")
   {
      testSet.clear();
      status = MaxState(status, testTagValidBcsASet(tre, reporter,
         &numFields, "CG_MAP_TYPE_BLANK", testSet, true, false, true));
   }
   else
   {
      status = MaxState(status, SUSPECT);
   }

   // end conditional

   testSet.clear();
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, "CG_SPARE_A", testSet, true, false, true));

   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "CA_CALPA", 0.0, 999.999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_SRTFR", 0.0, 999999999999.9));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_ENDFR", 0.0, 999999999999.9));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_CHRPRT", -99.999999, 99.999999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_WIDTH", 0.0, 0.9999999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_CENFRQ", 0.0, 99999999999.9));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_BW", 0.0, 99999999999.9));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_PRF", 0.0, 99999.9));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_PRI", 0.0, 0.9999999));
   status = MaxState(status, testTagValueRange<double>(tre, reporter, &numFields, "WF_CDP", 0.0, 100.000));
   status = MaxState(status, testTagValueRange<int>(tre, reporter, &numFields, "WF_NUMBER_OF_PULSES", 2U, 999999999U));

   testSet.clear();
   testSet.insert("Y");
   testSet.insert("N");
   status = MaxState(status, testTagValidBcsASet(tre, reporter, &numFields, "VPH_COND", testSet, false, false, true));

   unsigned int totalFields = tre.getNumAttributes();
   if (status != INVALID && totalFields != numFields)
   {
      reporter << "Total fields in the Dynamic Object(" <<
         totalFields << ") did not match the number found(" << numFields << ") ";
      status = INVALID;
   }

   if (status == INVALID)
   {
      reporter << " INVALID fields found in the CMETAA TAG/SDE\n" ;
   }
   else if (status == SUSPECT)
   {
      reporter << " SUSPECT fields found in the CMETAA TAG/SDE\n" ;
   }

   return status;
}

bool Nitf::CmetaaParser::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten,
   string &errorMessage) const
{
   size_t sizeIn = max(static_cast<ostream::pos_type>(0), output.tellp());
   size_t sizeOut(sizeIn);

   try
   {
      // NOTE: The standard for CMETAA (STDI-0002) lists the name of the first two fields as "RELATED_TRES"
      // This was probably an error as the first field is a count of the related TREs. Because the DO
      // will not allow two fields to have the same name it has been changed to allow the storage into the DO

      output << toString(dv_cast<int>(input.getAttribute(CMETAA::NUM_RELATED_TRES)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::RELATED_TRES)), 120);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::RD_RPC_NO)), 12);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_PROCESS)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::RD_CEN_FREQ)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::RD_MODE)), 5);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::RD_PATCH_NO)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_DOMAIN)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_MAG_REMAP_TYPE)), 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CMPLX_LIN_SCALE)), 7);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CMPLX_AVG_POWER)), 7);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_LINLOG_TP)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_PHASE_QUANT_FLAG)), 3);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_PHASE_QUANT_BIT_DEPTH)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_SIZE_1)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_IC_1)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_SIZE_2)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_IC_2)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_IC_BPP)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_WEIGHT)), 3);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_AZ_SLL)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_RNG_SLL)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_AZ_TAY_NBAR)), 2);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::CMPLX_RNG_TAY_NBAR)), 2);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_WEIGHT_NORM)), 3);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CMPLX_SIGNAL_PLANE)), 1);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_DC_SF_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_DC_SF_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_1_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_1_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_2_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_2_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_3_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_3_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_4_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_PATCH_4_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_DC_IS_ROW)), 8);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_DC_IS_COL)), 8);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_IMG_ROW_DC)), 8);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_IMG_COL_DC)), 8);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_1_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_1_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_2_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_2_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_3_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_3_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_4_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_TILE_4_COL)), 6);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_RD)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_RGWLK)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_KEYSTN)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_LINSFT)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_SUBPATCH)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_GEODIST)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_RGFO)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_BEAM_COMP)), 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_RGRES)), 8, 3);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_AZRES)), 8, 3);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_RSS)), 8, 5);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_AZSS)), 8, 5);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_RSR)), 8, 5);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_AZSR)), 8, 5);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_RFFT_SAMP)), 7);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_AZFFT_SAMP)), 7);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_RFFT_TOT)), 7);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_AZFFT_TOT)), 7);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_SUBP_ROW)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_SUBP_COL)), 6);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_SUB_RG)), 4);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::IF_SUB_AZ)), 4);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_RFFTS)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_AFFTS)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_RANGE_DATA)), 7);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_INCPH)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_SR_NAME1)), 8);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_SR_AMOUNT1)), 8, 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_SR_NAME2)), 8);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_SR_AMOUNT2)), 8, 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::IF_SR_NAME3)), 8);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::IF_SR_AMOUNT)), 8, 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::AF_TYPE1)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::AF_TYPE2)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::AF_TYPE3)), 5);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_TR)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_RE)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_REFERENCE)), 40);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_REG)), 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::POL_ISO_1)), 5, 2);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_BAL)), 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::POL_BAL_MAG)), 8);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::POL_BAL_PHS)), 8, 5, ZERO_FILL, POS_SIGN_TRUE);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_HCOMP)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::P_HCOMP_BASIS)), 10);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::POL_HCOMP_COEF_1)), 9);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::POL_HCOMP_COEF_2)), 9);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::POL_HCOMP_COEF_3)), 9);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_AFCOMP)), 1);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::POL_SPARE_A)), 15);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::POL_SPARE_N)), 9);

      // T_UTC_YYYYMMMDD and T_HHMMSSUTC are combined into one DateTime field: T_UTC_YYYYMMMDD_HHMMSS
      // put date in form CCYYMMMDDhhmmss for this TAG    see: strftime() for format info
      const DateTime* pAppDtg = dv_cast<DateTime>(&input.getAttribute(CMETAA::T_UTC_YYYYMMMDD_HHMMSS));
      if (pAppDtg == NULL)
      {
         return false;
      }

      string CCYYMMMDDhhmmss = pAppDtg->getFormattedUtc("%Y%b%d%H%M%S");
      boost::to_upper(CCYYMMMDDhhmmss);
      output << sizeString(CCYYMMMDDhhmmss, 15);

      // Need to handle time only without the date.
      // T_HHMMSSLOCAL is the local time of T_HHMMSSUTC and may not be on the same date as the UTC version
      output << sizeString( dv_cast<string>(input.getAttribute (CMETAA::T_HHMMSSLOCAL)), 6);

      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SRAC)), 11, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SLANT_CONFIDENCE)), 7, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_CROSS)), 11, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_CROSS_CONFIDENCE)), 7, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_CAAC)), 9, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_CONE_CONFIDENCE)), 6);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GPSAC)), 8, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GPSAC_CONFIDENCE)), 6);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SQUINT)), 8, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GAAC)), 7, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GAAC_CONFIDENCE)), 6);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_INCIDENT)), 7, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SLOPE)), 7, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_TILT)), 8, 4, ZERO_FILL, POS_SIGN_TRUE);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_LD)), 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_NORTH)), 8, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_NORTH_CONFIDENCE)), 6);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_EAST)), 8, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_RLOS)), 8, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_LOS_CONFIDENCE)), 6);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_LAYOVER)), 8, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SHADOW)), 8, 4);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_OPM)), 7, 3);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MODEL)), 5);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_AMPT_X)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_AMPT_Y)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_AMPT_Z)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_AP_CONF_XY)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_AP_CONF_Z)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_APCEN_X)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_APCEN_Y)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_APCEN_Z)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_APER_CONF_XY)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_APER_CONF_Z)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_FPNUV_X)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_FPNUV_Y)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_FPNUV_Z)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_IDPNUVX)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_IDPNUVY)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_IDPNUVZ)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SCECN_X)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SCECN_Y)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SCECN_Z)), 13, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SC_CONF_XY)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SC_CONF_Z)), 6, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SWWD)), 8, 2);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNVEL_X)), 10, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNVEL_Y)), 10, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNVEL_Z)), 10, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNACC_X)), 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNACC_Y)), 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNACC_Z)), 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNATT_ROLL)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNATT_PITCH)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_SNATT_YAW)), 8, 3, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GTP_X)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GTP_Y)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_GTP_Z)), 9, 6, ZERO_FILL, POS_SIGN_TRUE);

      // Test value for following conditionals
      string map_type = dv_cast<string>(input.getAttribute (CMETAA::CG_MAP_TYPE));
      output << sizeString( map_type, 4);

      if (map_type == "GEOD")
      {
         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LATCEN)), 11, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LNGCEN)), 12, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LTCORUL)), 11, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LGCORUL)), 12, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LTCORUR)), 11, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LGCORUR)), 12, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LTCORLR)), 11, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LGCORLR)), 12, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LTCORLL)), 11, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>
            (input.getAttribute(CMETAA::CG_PATCH_LNGCOLL)), 12, 7, ZERO_FILL, POS_SIGN_TRUE);

         output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_PATCH_LAT_CONFIDENCE)), 9, 7);

         output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_PATCH_LON_CONFIDENCE)), 9, 7);
      }
      else if (map_type == "MGRS")
      {
         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRS_CENT)), 23);

         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRSCORUL)), 23);

         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRSCORUR)), 23);

         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRSCORLR)), 23);

         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRSCORLL)), 23);

         output << toString(dv_cast<double>(input.getAttribute(CMETAA::CG_MGRS_CONFIDENCE)), 7, 2);

         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MGRS_PAD)), 11);
      }
      else if (map_type == "NA")
      {
         output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_MAP_TYPE_BLANK)), 133);
      }
      else
      {
         // ERROR map_type should always be one of the 3 values above.
      }

      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::CG_SPARE_A)), 144);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::CA_CALPA)), 7);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_SRTFR)), 14, 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_ENDFR)), 14, 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_CHRPRT)), 10, 6, ZERO_FILL, POS_SIGN_TRUE);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_WIDTH)), 9);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_CENFRQ)), 13, 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_BW)), 13, 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_PRF)), 7, 1);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_PRI)), 9);
      output << toString(dv_cast<double>(input.getAttribute(CMETAA::WF_CDP)), 7, 3);
      output << toString(dv_cast<int>(input.getAttribute(CMETAA::WF_NUMBER_OF_PULSES)), 9);
      output << sizeString(dv_cast<string>(input.getAttribute(CMETAA::VPH_COND)), 1);
   }
   catch (const bad_cast&)
   {
      return false;
   }

   sizeOut = output.tellp();
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
