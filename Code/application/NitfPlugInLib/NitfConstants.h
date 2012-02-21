/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFCONSTANTS_H
#define NITFCONSTANTS_H

#include <iomanip>
#include <sstream>
#include <string>

namespace Nitf
{
   static const bool POS_SIGN_TRUE = true;
   static const bool POS_SIGN_FALSE = false;
   static const bool ALL_BLANK_TRUE = true;
   static const bool ALL_BLANK_FALSE = false;
   static const bool NOT_IN_SET_TRUE = true;
   static const bool NOT_IN_SET_FALSE = false;
   static const bool EMIT_MSG_NOT_IN_SET_TRUE = true;
   static const bool EMIT_MSG_NOT_IN_SET_FALSE = false;
   static const bool USE_SCIENTIFIC_NOTATION = true;
   static const char ZERO_FILL = '0';
   static const int ONE_EXP_DIGIT = 1;

   const static std::string NITF_METADATA = "NITF";
   const static std::string FILE_HEADER = "File Header";
   const static std::string IMAGE_SUBHEADER = "Image Subheader";
   const static std::string DES_METADATA = "DES";
   const static std::string TRE_METADATA = "TRE";
   const static std::string TRE_INFO_METADATA = "TRE Information";

   const static std::string VERSION_02_00 = "02.00";
   const static std::string VERSION_02_10 = "02.10";

   namespace FileHeaderFieldNames
   {
      const std::string CLEVEL = "CLEVEL";
      const std::string ENCRYPTION = "ENCRYP";
      const std::string BACKGROUND_COLOR = "FBKGC";
      const std::string DATE_TIME = "FDT";
      const std::string TYPE = "FHDR";
      const std::string LENGTH = "FL";
      const std::string TITLE = "FTITLE";
      const std::string FILE_VERSION = "FVER";
      const std::string HEADER_LENGTH = "HL";
      const std::string ONAME = "ONAME";
      const std::string OPHONE = "OPHONE";
      const std::string OSTAID = "OSTAID";
      const std::string SYSTEM_TYPE = "STYPE";

      const std::string SECURITY_AUTH = "FSCAUT";
      const std::string SECURITY_COPY_NUMBER = "FSCOP";
      const std::string SECURITY_NUM_COPIES = "FSCPYS";
      const std::string SECURITY_CTRL_AND_HANDL = "FSCTLH";
      const std::string SECURITY_CTRL_NUM = "FSCTLN";
      const std::string SECURITY_DOWNGRADE = "FSDG";
      const std::string SECURITY_DOWNGRADE_2_0 = "FSDWNG";
      const std::string SECURITY_RELEASE_INSTRUCTIONS = "FSREL";
      const std::string SECURITY_LEVEL = "FSCLAS";
      const std::string SECURITY_SYSTEM = "FSCLSY";
      const std::string SECURITY_CLASS_TEXT = "FSCLTX";
      const std::string SECURITY_CLASS_REASON = "FSCRSN";
      const std::string SECURITY_CODEWORDS = "FSCODE";
      const std::string SECURITY_DECLASS_EXEMPT = "FSDCXM";
      const std::string SECURITY_AUTH_TYPE = "FSCATP";
      const std::string SECURITY_DECLASS_DATE = "FSDCDT";
      const std::string SECURITY_DECLASS_TYPE = "FSDCTP";
      const std::string SECURITY_DOWNGRADE_DATE = "FSDGDT";
      const std::string SECURITY_SOURCE_DATE = "FSSRDT";

      const std::string NUM_DES = "NUMDES";
      const std::string NUM_RES = "NUMRES";
      const std::string NUM_IMAGE_SEGMENTS = "NUMI";
      const std::string NUM_GRAPHIC_SEGMENTS = "NUMS";
      const std::string NUM_TEXT_SEGMENTS = "NUMT";
      const std::string NUM_LABELS = "NUML"; // NITF 02.00 only
   }

   namespace ImageSubheaderFieldNames
   {
      const std::string ACTUAL_BITS_PER_PIXEL = "ABPP";
      const std::string COMPRESSION_RATIO = "COMRAT";
      const std::string ATTACHMENT_LEVEL = "IALVL";
      const std::string COMPRESSION = "IC";
      const std::string CATEGORY = "ICAT";
      const std::string NUMBER_IMAGE_COMENTS = "ICOM";
      const std::string DATETIME = "IDATIM";
      const std::string DISPLAY_LEVEL = "IDLVL";
      const std::string ENCRYPTION = "ENCRYP";
      const std::string ICORDS = "ICORDS";
      const std::string ID_1 = "IID1";
      const std::string ID_2 = "IID2";
      const std::string IGEOLO = "IGEOLO";
      const std::string LOCATION = "ILOC";
      const std::string MAGNIFICATION = "IMAG";
      const std::string HAS_LUTS = "LUTS"; // has lookup tables
      const std::string MODE = "IMODE";
      const std::string REPRESENTATION = "IREP";
      const std::string SOURCE = "ISORCE";
      const std::string BLOCKS_PER_COLUMN = "NBPC";
      const std::string BITS_PER_PIXEL = "NBPP";
      const std::string BLOCKS_PER_ROW = "NBPR";
      const std::string NUMBER_COMMENTS = "NICOM";
      const std::string PIXELS_PER_BLOCK_HORIZONTAL = "NPPBH";
      const std::string PIXELS_PER_BLOCK_VERTICAL = "NPPBV";
      const std::string PIXEL_VALUE_TYPE = "PVTYPE";
      const std::string PIXEL_JUSTIFICATION = "PJUST";
      const std::string TARGET_ID = "TGTID";
      const std::string NBANDS = "NBANDS";
      const std::string XBANDS = "XBANDS";
      const std::string BAND_SIGNIFICANCES = "ISUBCAT[n]";
      const std::string BAND_REPRESENTATIONS = "IREPBAND[n]";
      const std::string NUMBER_OF_LUTS = "NLUTS[n]";
      const std::string SECURITY_AUTH = "ISCAUT";
      const std::string SECURITY_CTRL_AND_HANDL = "ISCTLH";
      const std::string SECURITY_CTRL_NUM = "ISCTLN";
      const std::string SECURITY_DOWNGRADE = "ISDG";
      const std::string SECURITY_RELEASE_INSTRUCTIONS = "ISREL";
      const std::string SECURITY_LEVEL = "ISCLAS";
      const std::string SECURITY_SYSTEM = "ISCLSY";
      const std::string SECURITY_CLASS_TEXT = "ISCLTX";
      const std::string SECURITY_CLASS_REASON = "ISCRSN";
      const std::string SECURITY_CODEWORDS = "ISCODE";
      const std::string SECURITY_DECLASS_EXEMPT = "ISDCXM";
      const std::string SECURITY_AUTH_TYPE = "ISCATP";
      const std::string SECURITY_DECLASS_DATE = "ISDCDT";
      const std::string SECURITY_DECLASS_TYPE = "ISDCTP";
      const std::string SECURITY_DOWNGRADE_DATE = "ISDGDT";
      const std::string SECURITY_SOURCE_DATE = "ISSRDT";
   }

   namespace ImageSubheaderFieldValues
   {
      const std::string REPRESENTATION_LUT = "RGB/LUT";
      const std::string REPRESENTATION_MONO = "MONO";
      const std::string REPRESENTATION_MULTI = "MULTI";
      const std::string REPRESENTATION_RGB = "RGB";
      const std::string BAND_REPRESENTATIONS_RED = "R";
      const std::string BAND_REPRESENTATIONS_GREEN = "G";
      const std::string BAND_REPRESENTATIONS_BLUE = "B";
      const std::string BAND_REPRESENTATIONS_MONO = "M";
      const std::string BAND_REPRESENTATIONS_LUT = "LU";
      const std::string ICORDS_NONE = "N";   // NITF 02.00 only
      const std::string ICORDS_UTM_MGRS = "U";
      const std::string ICORDS_UTM_NORTH = "N";
      const std::string ICORDS_UTM_SOUTH = "S";
      const std::string ICORDS_GEOGRAPHIC = "G";
      const std::string ICORDS_GEOCENTRIC = "C";   // NITF 02.00 only
      const std::string ICORDS_DECIMAL_DEGREES = "D";
   }

   namespace DesSubheaderFieldNames
   {
      const std::string DE = "DE";
      const std::string DESID = "DESID";
      const std::string DESVER = "DESVER";
      const std::string SECURITY_LEVEL = "DECLAS";
      const std::string SECURITY_SYSTEM = "DESCLSY";
      const std::string SECURITY_CODEWORDS = "DESCODE";
      const std::string SECURITY_CTRL_AND_HANDL = "DESCTLH";
      const std::string SECURITY_RELEASE_INSTRUCTIONS = "DESREL";
      const std::string SECURITY_DECLASS_TYPE = "DESDCTP";
      const std::string SECURITY_DECLASS_DATE = "DESDCDT";
      const std::string SECURITY_DECLASS_EXEMPT = "DESDCXM";
      const std::string SECURITY_DOWNGRADE = "DESDG";
      const std::string SECURITY_DOWNGRADE_DATE = "DESDGDT";
      const std::string SECURITY_CLASS_TEXT = "DESCLTX";
      const std::string SECURITY_AUTH_TYPE = "DESCATP";
      const std::string SECURITY_AUTH = "DESCAUT";
      const std::string SECURITY_CLASS_REASON = "DESCRSN";
      const std::string SECURITY_SOURCE_DATE = "DESSRDT";
      const std::string SECURITY_CTRL_NUM = "DESCTLN";
      const std::string DESOFLW = "DESOFLW";
      const std::string DESITEM = "DESITEM";
      const std::string DESSHL = "DESSHL";
      const std::string DESSHF = "DESSHF";
      const std::string DESDATA = "DESDATA";
   }

   // contains built-in TRE information
   namespace TRE
   {
      const static std::string UNPARSED_TAGS = "_UNPARSED_";
      
      namespace ACFTA
      {
         const static std::string AC_MSN_ID = "AC_MSN_ID";
         const static std::string SCTYPE = "SCTYPE";
         const static std::string SCNUM = "SCNUM";
         const static std::string SENSOR_ID = "SENSOR_ID";
         const static std::string PATCH_TOT = "PATCH_TOT";
         const static std::string MTI_TOT = "MTI_TOT";
         const static std::string PDATE = "PDATE";
         const static std::string IMHOSTNO = "IMHOSTNO";
         const static std::string IMREQID = "IMREQID";
         const static std::string SCENE_SOURCE = "SCENE_SOURCE";
         const static std::string MPLAN = "MPLAN";
         const static std::string ENTLOC = "ENTLOC";
         const static std::string ENTELV = "ENTELV";
         const static std::string EXITLOC = "EXITLOC";
         const static std::string EXITELV = "EXITELV";
         const static std::string TMAP = "TMAP";
         const static std::string RCS = "RCS";
         const static std::string ROW_SPACING = "ROW_SPACING";
         const static std::string COL_SPACING = "COL_SPACING";
         const static std::string SENSERIAL = "SENSERIAL";
         const static std::string ABSWVER = "ABSWVER";
      }

      namespace ACFTB
      {
         const static std::string AC_MSN_ID = "AC_MSN_ID";
         const static std::string AC_TAIL_NO = "AC_TAIL_NO";
         const static std::string AC_TO = "AC_TO";
         const static std::string SENSOR_ID_TYPE = "SENSOR_ID_TYPE";
         const static std::string SENSOR_ID = "SENSOR_ID";
         const static std::string SCENE_SOURCE = "SCENE_SOURCE";
         const static std::string SCNUM = "SCNUM";
         const static std::string PDATE = "PDATE";
         const static std::string IMHOSTNO = "IMHOSTNO";
         const static std::string IMREQID = "IMREQID";
         const static std::string MPLAN = "MPLAN";
         const static std::string ENTLOC = "ENTLOC";
         const static std::string LOC_ACCY = "LOC_ACCY";
         const static std::string ENTELV = "ENTELV";
         const static std::string ELV_UNIT = "ELV_UNIT";
         const static std::string EXITLOC = "EXITLOC";
         const static std::string EXITELV = "EXITELV";
         const static std::string TMAP = "TMAP";
         const static std::string ROW_SPACING = "ROW_SPACING";
         const static std::string ROW_SPACING_UNITS = "ROW_SPACING_UNITS";
         const static std::string COL_SPACING = "COL_SPACING";
         const static std::string COL_SPACING_UNITS = "COL_SPACING_UNITS";
         const static std::string FOCAL_LENGTH = "FOCAL_LENGTH";
         const static std::string SENSERIAL = "SENSERIAL";
         const static std::string ABSWVER = "ABSWVER";
         const static std::string CAL_DATE = "CAL_DATE";
         const static std::string PATCH_TOT = "PATCH_TOT";
         const static std::string MTI_TOT = "MTI_TOT";
      }

      namespace AIMIDA
      {
         const static std::string MISSION_DATE = "MISSION_DATE";
         const static std::string MISSION_NO = "MISSION_NO";
         const static std::string FLIGHT_NO = "FLIGHT_NO";
         const static std::string OP_NUM = "OP_NUM";
         const static std::string START_SEGMENT = "START_SEGMENT";
         const static std::string REPRO_NUM = "REPRO_NUM";
         const static std::string REPLAY = "REPLAY";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string START_COLUMN = "START_COLUMN";
         const static std::string START_ROW = "START_ROW";
         const static std::string END_SEGMENT = "END_SEGMENT";
         const static std::string END_COLUMN = "END_COLUMN";
         const static std::string END_ROW = "END_ROW";
         const static std::string COUNTRY = "COUNTRY";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string LOCATION = "LOCATION";
         const static std::string TIME = "TIME";
         const static std::string CREATION_DATE = "CREATION_DATE";
      }

      namespace AIMIDB
      {
         const static std::string ACQUISITION_DATE = "ACQUISITION_DATE";
         const static std::string MISSION_NO = "MISSION_NO";
         const static std::string MISSION_IDENTIFICATION = "MISSION_IDENTIFICATION";
         const static std::string FLIGHT_NO = "FLIGHT_NO";
         const static std::string OP_NUM = "OP_NUM";
         const static std::string CURRENT_SEGMENT = "CURRENT_SEGMENT";
         const static std::string REPRO_NUM = "REPRO_NUM";
         const static std::string REPLAY = "REPLAY";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string START_TILE_COLUMN = "START_TILE_COLUMN";
         const static std::string START_TILE_ROW = "START_TILE_ROW";
         const static std::string END_SEGMENT = "END_SEGMENT";
         const static std::string END_TILE_COLUMN = "END_TILE_COLUMN";
         const static std::string END_TILE_ROW = "END_TILE_ROW";
         const static std::string COUNTRY = "COUNTRY";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string LOCATION = "LOCATION";
         const static std::string RESERVED3 = "RESERVED3";
      }

      namespace BANDSA
      {
         const static std::string ROW_SPACING = "ROW_SPACING";
         const static std::string ROW_SPACING_UNITS = "ROW_SPACING_UNITS";
         const static std::string COL_SPACING = "COL_SPACING";
         const static std::string COL_SPACING_UNITS = "COL_SPACING_UNITS";
         const static std::string FOCAL_LENGTH = "FOCAL_LENGTH";
         const static std::string BANDCOUNT = "BANDCOUNT";
         const static std::string BANDPEAK = "BANDPEAK";
         const static std::string BANDLBOUND = "BANDLBOUND";
         const static std::string BANDUBOUND = "BANDUBOUND";
         const static std::string BANDWIDTH = "BANDWIDTH";
         const static std::string BANDCALDRK = "BANDCALDRK";
         const static std::string BANDCALINC = "BANDCALINC";
         const static std::string BANDRESP = "BANDRESP";
         const static std::string BANDASD = "BANDASD";
         const static std::string BANDGSD = "BANDGSD";
      }

      namespace BANDSB
      {
         const static std::string COUNT = "COUNT";
         const static std::string RADIOMETRIC_QUANTITY = "RADIOMETRIC_QUANTITY";
         const static std::string RADIOMETRIC_QUANTITY_UNIT = "RADIOMETRIC_QUANTITY_UNIT";
         const static std::string SCALE_FACTOR = "SCALE_FACTOR";
         const static std::string ADDITIVE_FACTOR = "ADDITIVE_FACTOR";
         const static std::string ROW_GSD = "ROW_GSD";
         const static std::string ROW_GSD_UNIT = "ROW_GSD_UNIT";
         const static std::string COL_GSD = "COL_GSD";
         const static std::string COL_GSD_UNITS = "COL_GSD_UNITS";
         const static std::string COL_GSD_UNIT = "COL_GSD_UNIT";
         const static std::string SPT_RESP_ROW = "SPT_RESP_ROW";
         const static std::string SPT_RESP_UNIT_ROW = "SPT_RESP_UNIT_ROW";
         const static std::string SPT_RESP_COL = "SPT_RESP_COL";
         const static std::string SPT_RESP_UNIT_COL = "SPT_RESP_UNIT_COL";
         const static std::string DATA_FLD_1 = "DATA_FLD_1";
         const static std::string EXISTENCE_MASK = "EXISTENCE_MASK";
         const static std::string RADIOMETRIC_ADJUSTMENT_SURFACE = "RADIOMETRIC_ADJUSTMENT_SURFACE";
         const static std::string ATMOSPHERIC_ADJUSTMENT_ALTITUDE = "ATMOSPHERIC_ADJUSTMENT_ALTITUDE";
         const static std::string DIAMETER = "DIAMETER";
         const static std::string DATA_FLD_2 = "DATA_FLD_2";
         const static std::string WAVE_LENGTH_UNIT = "WAVE_LENGTH_UNIT";
         const static std::string BANDID = "BANDID";
         const static std::string BAD_BAND = "BAD_BAND";
         const static std::string NIIRS = "NIIRS";
         const static std::string FOCAL_LEN = "FOCAL_LEN";
         const static std::string CWAVE = "CWAVE";
         const static std::string FWHM = "FWHM";
         const static std::string FWHM_UNC = "FWHM_UNC";
         const static std::string NOM_WAVE = "NOM_WAVE";
         const static std::string NOM_WAVE_UNC = "NOM_WAVE_UNC";
         const static std::string LBOUND = "LBOUND";
         const static std::string UBOUND = "UBOUND";
         const static std::string START_TIME = "START_TIME";
         const static std::string START_TIME_FRAC = "START_TIME_FRAC";
         const static std::string INT_TIME = "INT_TIME";
         const static std::string CALDRK = "CALDRK";
         const static std::string CALIBRATION_SENSITIVITY = "CALIBRATION_SENSITIVITY";
         const static std::string ROW_GSD_UNC = "ROW_GSD_UNC";
         const static std::string COL_GSD_UNC = "COL_GSD_UNC";
         const static std::string BKNOISE = "BKNOISE";
         const static std::string SCNNOISE = "SCNNOISE";
         const static std::string SPT_RESP_FUNCTION_ROW = "SPT_RESP_FUNCTION_ROW";
         const static std::string SPT_RESPUNC_ROW = "SPT_RESPUNC_ROW";
         const static std::string SPT_RESP_FUNCTION_COL = "SPT_RESP_FUNCTION_COL";
         const static std::string SPT_RESPUNC_COL = "SPT_RESPUNC_COL";
         const static std::string DATA_FLD_3 = "DATA_FLD_3";
         const static std::string DATA_FLD_4 = "DATA_FLD_4";
         const static std::string DATA_FLD_5 = "DATA_FLD_5";
         const static std::string DATA_FLD_6 = "DATA_FLD_6";
         const static std::string NUM_AUX_B = "NUM_AUX_B";
         const static std::string NUM_AUX_C = "NUM_AUX_C";
         const static std::string BAPF = "BAPF";
         const static std::string UBAP = "UBAP";
         const static std::string APN = "APN";
         const static std::string APR = "APR";
         const static std::string APA = "APA";
         const static std::string CAPF = "CAPF";
         const static std::string UCAP = "UCAP";
      }

      namespace BLOCKA
      {
         const static std::string BLOCK_INSTANCE = "BLOCK_INSTANCE";
         const static std::string N_GRAY = "N_GRAY";
         const static std::string L_LINES = "L_LINES";
         const static std::string LAYOVER_ANGLE = "LAYOVER_ANGLE";
         const static std::string SHADOW_ANGLE = "SHADOW_ANGLE";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string FRLC_LOC = "FRLC_LOC";
         const static std::string LRLC_LOC = "LRLC_LOC";
         const static std::string LRFC_LOC = "LRFC_LOC";
         const static std::string FRFC_LOC = "FRFC_LOC";
         const static std::string RESERVED2 = "RESERVED2";
      }


      namespace CMETAA
      {
         const static std::string NUM_RELATED_TRES = "NUM_RELATED_TRES";
         const static std::string RELATED_TRES = "RELATED_TRES";
         const static std::string RD_PRC_NO = "RD_PRC_NO";
         const static std::string IF_PROCESS = "IF_PROCESS";
         const static std::string RD_CEN_FREQ = "RD_CEN_FREQ";
         const static std::string RD_MODE = "RD_MODE";
         const static std::string RD_PATCH_NO = "RD_PATCH_NO";
         const static std::string CMPLX_DOMAIN = "CMPLX_DOMAIN";
         const static std::string CMPLX_MAG_REMAP_TYPE = "CMPLX_MAG_REMAP_TYPE";
         const static std::string CMPLX_LIN_SCALE = "CMPLX_LIN_SCALE";
         const static std::string CMPLX_AVG_POWER = "CMPLX_AVG_POWER";
         const static std::string CMPLX_LINLOG_TP = "CMPLX_LINLOG_TP";
         const static std::string CMPLX_PHASE_QUANT_FLAG = "CMPLX_PHASE_QUANT_FLAG";
         const static std::string CMPLX_PHASE_QUANT_BIT_DEPTH = "CMPLX_PHASE_QUANT_BIT_DEPTH";
         const static std::string CMPLX_SIZE_1 = "CMPLX_SIZE_1";
         const static std::string CMPLX_IC_1 = "CMPLX_IC_1";
         const static std::string CMPLX_SIZE_2 = "CMPLX_SIZE_2";
         const static std::string CMPLX_IC_2 = "CMPLX_IC_2";
         const static std::string CMPLX_IC_BPP = "CMPLX_IC_BPP";
         const static std::string CMPLX_WEIGHT = "CMPLX_WEIGHT";
         const static std::string CMPLX_AZ_SLL = "CMPLX_AZ_SLL";
         const static std::string CMPLX_RNG_SLL = "CMPLX_RNG_SLL";
         const static std::string CMPLX_AZ_TAY_NBAR = "CMPLX_AZ_TAY_NBAR";
         const static std::string CMPLX_RNG_TAY_NBAR = "CMPLX_RNG_TAY_NBAR";
         const static std::string CMPLX_WEIGHT_NORM = "CMPLX_WEIGHT_NORM";
         const static std::string CMPLX_SIGNAL_PLANE = "CMPLX_SIGNAL_PLANE";
         const static std::string IF_DC_SF_ROW = "IF_DC_SF_ROW";
         const static std::string IF_DC_SF_COL = "IF_DC_SF_COL";
         const static std::string IF_PATCH_1_ROW = "IF_PATCH_1_ROW";
         const static std::string IF_PATCH_1_COL = "IF_PATCH_1_COL";
         const static std::string IF_PATCH_2_ROW = "IF_PATCH_2_ROW";
         const static std::string IF_PATCH_2_COL = "IF_PATCH_2_COL";
         const static std::string IF_PATCH_3_ROW = "IF_PATCH_3_ROW";
         const static std::string IF_PATCH_3_COL = "IF_PATCH_3_COL";
         const static std::string IF_PATCH_4_ROW = "IF_PATCH_4_ROW";
         const static std::string IF_PATCH_4_COL = "IF_PATCH_4_COL";
         const static std::string IF_DC_IS_ROW = "IF_DC_IS_ROW";
         const static std::string IF_DC_IS_COL = "IF_DC_IS_COL";
         const static std::string IF_IMG_ROW_DC = "IF_IMG_ROW_DC";
         const static std::string IF_IMG_COL_DC = "IF_IMG_COL_DC";
         const static std::string IF_TILE_1_ROW = "IF_TILE_1_ROW";
         const static std::string IF_TILE_1_COL = "IF_TILE_1_COL";
         const static std::string IF_TILE_2_ROW = "IF_TILE_2_ROW";
         const static std::string IF_TILE_2_COL = "IF_TILE_2_COL";
         const static std::string IF_TILE_3_ROW = "IF_TILE_3_ROW";
         const static std::string IF_TILE_3_COL = "IF_TILE_3_COL";
         const static std::string IF_TILE_4_ROW = "IF_TILE_4_ROW";
         const static std::string IF_TILE_4_COL = "IF_TILE_4_COL";
         const static std::string IF_RD = "IF_RD";
         const static std::string IF_RGWLK = "IF_RGWLK";
         const static std::string IF_KEYSTN = "IF_KEYSTN";
         const static std::string IF_LINSFT = "IF_LINSFT";
         const static std::string IF_SUBPATCH = "IF_SUBPATCH";
         const static std::string IF_GEODIST = "IF_GEODIST";
         const static std::string IF_RGFO = "IF_RGFO";
         const static std::string IF_BEAM_COMP = "IF_BEAM_COMP";
         const static std::string IF_RGRES = "IF_RGRES";
         const static std::string IF_AZRES = "IF_AZRES";
         const static std::string IF_RSS = "IF_RSS";
         const static std::string IF_AZSS = "IF_AZSS";
         const static std::string IF_RSR = "IF_RSR";
         const static std::string IF_AZSR = "IF_AZSR";
         const static std::string IF_RFFT_SAMP = "IF_RFFT_SAMP";
         const static std::string IF_AZFFT_SAMP = "IF_AZFFT_SAMP";
         const static std::string IF_RFFT_TOT = "IF_RFFT_TOT";
         const static std::string IF_AZFFT_TOT = "IF_AZFFT_TOT";
         const static std::string IF_SUBP_ROW = "IF_SUBP_ROW";
         const static std::string IF_SUBP_COL = "IF_SUBP_COL";
         const static std::string IF_SUB_RG = "IF_SUB_RG";
         const static std::string IF_SUB_AZ = "IF_SUB_AZ";
         const static std::string IF_RFFTS = "IF_RFFTS";
         const static std::string IF_AFFTS = "IF_AFFTS";
         const static std::string IF_RANGE_DATA = "IF_RANGE_DATA";
         const static std::string IF_INCPH = "IF_INCPH";
         const static std::string IF_SR_NAME1 = "IF_SR_NAME1";
         const static std::string IF_SR_AMOUNT1 = "IF_SR_AMOUNT1";
         const static std::string IF_SR_NAME2 = "IF_SR_NAME2";
         const static std::string IF_SR_AMOUNT2 = "IF_SR_AMOUNT2";
         const static std::string IF_SR_NAME3 = "IF_SR_NAME3";
         const static std::string IF_SR_AMOUNT = "IF_SR_AMOUNT";
         const static std::string AF_TYPE1 = "AF_TYPE1";
         const static std::string AF_TYPE2 = "AF_TYPE2";
         const static std::string AF_TYPE3 = "AF_TYPE3";
         const static std::string POL_TR = "POL_TR";
         const static std::string POL_RE = "POL_RE";
         const static std::string POL_REFERENCE = "POL_REFERENCE";
         const static std::string POL = "POL";
         const static std::string POL_REG = "POL_REG";
         const static std::string POL_ISO_1 = "POL_ISO_1";
         const static std::string POL_BAL = "POL_BAL";
         const static std::string POL_BAL_MAG = "POL_BAL_MAG";
         const static std::string POL_BAL_PHS = "POL_BAL_PHS";
         const static std::string POL_HCOMP = "POL_HCOMP";
         const static std::string P_HCOMP_BASIS = "P_HCOMP_BASIS";
         const static std::string POL_HCOMP_COEF_1 = "POL_HCOMP_COEF_1";
         const static std::string POL_HCOMP_COEF_2 = "POL_HCOMP_COEF_2";
         const static std::string POL_HCOMP_COEF_3 = "POL_HCOMP_COEF_3";
         const static std::string POL_AFCOMP = "POL_AFCOMP";
         const static std::string POL_SPARE_A = "POL_SPARE_A";
         const static std::string POL_SPARE_N = "POL_SPARE_N";

         const static std::string T_UTC_YYYYMMMDD_HHMMSS = "T_UTC_YYYYMMMDD_HHMMSS";
         const static std::string T_HHMMSSLOCAL = "T_HHMMSSLOCAL";

         const static std::string CG_SRAC = "CG_SRAC";
         const static std::string CG_SLANT_CONFIDENCE = "CG_SLANT_CONFIDENCE";
         const static std::string CG_CROSS = "CG_CROSS";
         const static std::string CG_CROSS_CONFIDENCE = "CG_CROSS_CONFIDENCE";
         const static std::string CG_CAAC = "CG_CAAC";
         const static std::string CG_CONE_CONFIDENCE = "CG_CONE_CONFIDENCE";
         const static std::string CG_GPSAC = "CG_GPSAC";
         const static std::string CG_GPSAC_CONFIDENCE = "CG_GPSAC_CONFIDENCE";
         const static std::string CG_SQUINT = "CG_SQUINT";
         const static std::string CG_GAAC = "CG_GAAC";
         const static std::string CG_GAAC_CONFIDENCE = "CG_GAAC_CONFIDENCE";
         const static std::string CG_INCIDENT = "CG_INCIDENT";
         const static std::string CG_SLOPE = "CG_SLOPE";
         const static std::string CG_TILT = "CG_TILT";
         const static std::string CG_LD = "CG_LD";
         const static std::string CG_NORTH = "CG_NORTH";
         const static std::string CG_NORTH_CONFIDENCE = "CG_NORTH_CONFIDENCE";
         const static std::string CG_EAST = "CG_EAST";
         const static std::string CG_RLOS = "CG_RLOS";
         const static std::string CG_LOS_CONFIDENCE = "CG_LOS_CONFIDENCE";
         const static std::string CG_LAYOVER = "CG_LAYOVER";
         const static std::string CG_SHADOW = "CG_SHADOW";
         const static std::string CG_OPM = "CG_OPM";
         const static std::string CG_MODEL = "CG_MODEL";
         const static std::string CG_AMPT_X = "CG_AMPT_X";
         const static std::string CG_AMPT_Y = "CG_AMPT_Y";
         const static std::string CG_AMPT_Z = "CG_AMPT_Z";
         const static std::string CG_AP_CONF_XY = "CG_AP_CONF_XY";
         const static std::string CG_AP_CONF_Z = "CG_AP_CONF_Z";
         const static std::string CG_APCEN_X = "CG_APCEN_X";
         const static std::string CG_APCEN_Y = "CG_APCEN_Y";
         const static std::string CG_APCEN_Z = "CG_APCEN_Z";
         const static std::string CG_APER_CONF_XY = "CG_APER_CONF_XY";
         const static std::string CG_APER_CONF_Z = "CG_APER_CONF_Z";
         const static std::string CG_FPNUV_X = "CG_FPNUV_X";
         const static std::string CG_FPNUV_Y = "CG_FPNUV_Y";
         const static std::string CG_FPNUV_Z = "CG_FPNUV_Z";
         const static std::string CG_IDPNUVX = "CG_IDPNUVX";
         const static std::string CG_IDPNUVY = "CG_IDPNUVY";
         const static std::string CG_IDPNUVZ = "CG_IDPNUVZ";
         const static std::string CG_SCECN_X = "CG_SCECN_X";
         const static std::string CG_SCECN_Y = "CG_SCECN_Y";
         const static std::string CG_SCECN_Z = "CG_SCECN_Z";
         const static std::string CG_SC_CONF_XY = "CG_SC_CONF_XY";
         const static std::string CG_SC_CONF_Z = "CG_SC_CONF_Z";
         const static std::string CG_SWWD = "CG_SWWD";
         const static std::string CG_SNVEL_X = "CG_SNVEL_X";
         const static std::string CG_SNVEL_Y = "CG_SNVEL_Y";
         const static std::string CG_SNVEL_Z = "CG_SNVEL_Z";
         const static std::string CG_SNACC_X = "CG_SNACC_X";
         const static std::string CG_SNACC_Y = "CG_SNACC_Y";
         const static std::string CG_SNACC_Z = "CG_SNACC_Z";
         const static std::string CG_SNATT_ROLL = "CG_SNATT_ROLL";
         const static std::string CG_SNATT_PITCH = "CG_SNATT_PITCH";
         const static std::string CG_SNATT_YAW = "CG_SNATT_YAW";
         const static std::string CG_GTP_X = "CG_GTP_X";
         const static std::string CG_GTP_Y = "CG_GTP_Y";
         const static std::string CG_GTP_Z = "CG_GTP_Z";

         const static std::string CG_MAP_TYPE = "CG_MAP_TYPE";

         const static std::string CG_PATCH_LATCEN = "CG_PATCH_LATCEN";
         const static std::string CG_PATCH_LNGCEN = "CG_PATCH_LNGCEN";
         const static std::string CG_PATCH_LTCORUL = "CG_PATCH_LTCORUL";
         const static std::string CG_PATCH_LGCORUL = "CG_PATCH_LGCORUL";
         const static std::string CG_PATCH_LTCORUR = "CG_PATCH_LTCORUR";
         const static std::string CG_PATCH_LGCORUR = "CG_PATCH_LGCORUR";
         const static std::string CG_PATCH_LTCORLR = "CG_PATCH_LTCORLR";
         const static std::string CG_PATCH_LGCORLR = "CG_PATCH_LGCORLR";
         const static std::string CG_PATCH_LTCORLL = "CG_PATCH_LTCORLL";
         const static std::string CG_PATCH_LNGCOLL = "CG_PATCH_LNGCOLL";
         const static std::string CG_PATCH_LAT_CONFIDENCE = "CG_PATCH_LAT_CONFIDENCE";
         const static std::string CG_PATCH_LON_CONFIDENCE = "CG_PATCH_LON_CONFIDENCE";

         const static std::string CG_MGRS_CENT = "CG_MGRS_CENT";
         const static std::string CG_MGRSCORUL = "CG_MGRSCORUL";
         const static std::string CG_MGRSCORUR = "CG_MGRSCORUR";
         const static std::string CG_MGRSCORLR = "CG_MGRSCORLR";
         const static std::string CG_MGRSCORLL = "CG_MGRSCORLL";
         const static std::string CG_MGRS_CONFIDENCE = "CG_MGRS_CONFIDENCE";
         const static std::string CG_MGRS_PAD = "CG_MGRS_PAD";

         const static std::string CG_MAP_TYPE_BLANK = "CG_MAP_TYPE_BLANK";

         const static std::string CG_SPARE_A = "CG_SPARE_A";
         const static std::string CA_CALPA = "CA_CALPA";
         const static std::string WF_SRTFR = "WF_SRTFR";
         const static std::string WF_ENDFR = "WF_ENDFR";
         const static std::string WF_CHRPRT = "WF_CHRPRT";
         const static std::string WF_WIDTH = "WF_WIDTH";
         const static std::string WF_CENFRQ = "WF_CENFRQ";
         const static std::string WF_BW = "WF_BW";
         const static std::string WF_PRF = "WF_PRF";
         const static std::string WF_PRI = "WF_PRI";
         const static std::string WF_CDP = "WF_CDP";
         const static std::string WF_NUMBER_OF_PULSES = "WF_NUMBER_OF_PULSES";
         const static std::string VPH_COND = "VPH_COND";
      }

      namespace ENGRDA
      {
         const static std::string RESRC = "RESRC";
         const static std::string RECNT = "RECNT";
         const static std::string ENGLN = "ENGLN";
         const static std::string ENGLBL = "ENGLBL";
         const static std::string ENGMTXC = "ENGMTXC";
         const static std::string ENGMTXR = "ENGMTXR";
         const static std::string ENGTYP = "ENGTYP";
         const static std::string ENGDTS = "ENGDTS";
         const static std::string ENGDATU = "ENGDATU";
         const static std::string ENGDATC = "ENGDATC";
         const static std::string ENGDATA = "ENGDATA";
      }

      namespace EXOPTA
      {
         const static std::string ANGLE_TO_NORTH = "ANGLE_TO_NORTH";
         const static std::string MEAN_GSD = "MEAN_GSD";
         const static std::string RESERVED = "RESERVED";
         const static std::string DYNAMIC_RANGE = "DYNAMIC_RANGE";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string OBL_ANG = "OBL_ANG";
         const static std::string ROLL_ANG = "ROLL_ANG";
         const static std::string PRIME_ID = "PRIME_ID";
         const static std::string PRIME_BE = "PRIME_BE";
         const static std::string RESERVED3 = "RESERVED3";
         const static std::string N_SEC = "N_SEC";
         const static std::string RESERVED4 = "RESERVED4";
         const static std::string RESERVED5 = "RESERVED5";
         const static std::string N_SEG = "N_SEG";
         const static std::string MAX_LP_SEG = "MAX_LP_SEG";
         const static std::string RESERVED6 = "RESERVED6";
         const static std::string SUN_EL = "SUN_EL";
         const static std::string SUN_AZ = "SUN_AZ";
      }

      namespace EXPLTA
      {
         const static std::string ANGLE_TO_NORTH = "ANGLE_TO_NORTH";
         const static std::string SQUINT_ANGLE = "SQUINT_ANGLE";
         const static std::string MODE = "MODE";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string GRAZE_ANG = "GRAZE_ANG";
         const static std::string SLOPE_ANG = "SLOPE_ANG";
         const static std::string POLAR = "POLAR";
         const static std::string NSAMP = "NSAMP";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string SEQ_NUM = "SEQ_NUM";
         const static std::string PRIME_ID = "PRIME_ID";
         const static std::string PRIME_BE = "PRIME_BE";
         const static std::string RESERVED3 = "RESERVED3";
         const static std::string N_SEC = "N_SEC";
         const static std::string IPR = "IPR";
         const static std::string RESERVED4 = "RESERVED4";
         const static std::string RESERVED5 = "RESERVED5";
         const static std::string RESERVED6 = "RESERVED6";
         const static std::string RESERVED7 = "RESERVED7";
      }

      namespace EXPLTB
      {
         const static std::string ANGLE_TO_NORTH = "ANGLE_TO_NORTH";
         const static std::string ANGLE_TO_NORTH_ACCY = "ANGLE_TO_NORTH_ACCY";
         const static std::string SQUINT_ANGLE = "SQUINT_ANGLE";
         const static std::string SQUINT_ANGLE_ACCY = "SQUINT_ANGLE_ACCY";
         const static std::string MODE = "MODE";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string GRAZE_ANG = "GRAZE_ANG";
         const static std::string GRAZE_ANG_ACCY = "GRAZE_ANG_ACCY";
         const static std::string SLOPE_ANG = "SLOPE_ANG";
         const static std::string POLAR = "POLAR";
         const static std::string NSAMP = "NSAMP";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string SEQ_NUM = "SEQ_NUM";
         const static std::string PRIME_ID = "PRIME_ID";
         const static std::string PRIME_BE = "PRIME_BE";
         const static std::string RESERVED3 = "RESERVED3";
         const static std::string N_SEC = "N_SEC";
         const static std::string IPR = "IPR";
      }

      namespace ICHIPB
      {
         const static std::string OP_COL_11 = "OP_COL_11";
         const static std::string OP_COL_12 = "OP_COL_12";
         const static std::string OP_COL_21 = "OP_COL_21";
         const static std::string OP_COL_22 = "OP_COL_22";
         const static std::string OP_ROW_11 = "OP_ROW_11";
         const static std::string OP_ROW_12 = "OP_ROW_12";
         const static std::string OP_ROW_21 = "OP_ROW_21";
         const static std::string OP_ROW_22 = "OP_ROW_22";
         const static std::string FI_COL_11 = "FI_COL_11";
         const static std::string FI_COL_12 = "FI_COL_12";
         const static std::string FI_COL_21 = "FI_COL_21";
         const static std::string FI_COL_22 = "FI_COL_22";
         const static std::string FI_ROW_11 = "FI_ROW_11";
         const static std::string FI_ROW_12 = "FI_ROW_12";
         const static std::string FI_ROW_21 = "FI_ROW_21";
         const static std::string FI_ROW_22 = "FI_ROW_22";
         const static std::string FI_ROW = "FI_ROW";
         const static std::string FI_COL = "FI_COL";

         const static std::string XFRM_FLAG = "XFRM_FLAG";
         const static std::string SCALE_FACTOR = "SCALE_FACTOR";
         const static std::string ANAMRPH_CORR = "ANAMRPH_CORR";
         const static std::string SCANBLK_NUM = "SCANBLK_NUM";
      }

      namespace MENSRA
      {
         const static std::string CCRP_LOC = "CCRP_LOC";
         const static std::string CCRP_ALT = "CCRP_ALT";
         const static std::string OF_PC_R = "OF_PC_R";
         const static std::string OF_PC_A = "OF_PC_A";
         const static std::string COSGRZ = "COSGRZ";
         const static std::string RGCCRP = "RGCCRP";
         const static std::string RLMAP = "RLMAP";
         const static std::string CCRP_ROW = "CCRP_ROW";
         const static std::string CCRP_COL = "CCRP_COL";
         const static std::string ACFT_LOC = "ACFT_LOC";
         const static std::string ACFT_ALT = "ACFT_ALT";
         const static std::string C_R_NC = "C_R_NC";
         const static std::string C_R_EC = "C_R_EC";
         const static std::string C_R_DC = "C_R_DC";
         const static std::string C_AZ_NC = "C_AZ_NC";
         const static std::string C_AZ_EC = "C_AZ_EC";
         const static std::string C_AZ_DC = "C_AZ_DC";
         const static std::string C_AL_NC = "C_AL_NC";
         const static std::string C_AL_EC = "C_AL_EC";
         const static std::string C_AL_DC = "C_AL_DC";
      }

      namespace MENSRB
      {
         const static std::string ACFT_LOC = "ACFT_LOC";
         const static std::string ACFT_LOC_ACCY = "ACFT_LOC_ACCY";
         const static std::string ACFT_ALT = "ACFT_ALT";
         const static std::string RP_LOC = "RP_LOC";
         const static std::string RP_LOC_ACCY = "RP_LOC_ACCY";
         const static std::string RP_ELEV = "RP_ELEV";
         const static std::string OF_PC_R = "OF_PC_R";
         const static std::string OF_PC_A = "OF_PC_A";
         const static std::string COSGRZ = "COSGRZ";
         const static std::string RGCRP = "RGCRP";
         const static std::string RLMAP = "RLMAP";
         const static std::string RP_ROW = "RP_ROW";
         const static std::string RP_COL = "RP_COL";
         const static std::string C_R_NC = "C_R_NC";
         const static std::string C_R_EC = "C_R_EC";
         const static std::string C_R_DC = "C_R_DC";
         const static std::string C_AZ_NC = "C_AZ_NC";
         const static std::string C_AZ_EC = "C_AZ_EC";
         const static std::string C_AZ_DC = "C_AZ_DC";
         const static std::string C_AL_NC = "C_AL_NC";
         const static std::string C_AL_EC = "C_AL_EC";
         const static std::string C_AL_DC = "C_AL_DC";
         const static std::string TOTAL_TILES_COLS = "TOTAL_TILES_COLS";
         const static std::string TOTAL_TILES_ROWS = "TOTAL_TILES_ROWS";
      }

      namespace MOD26A
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
         const static std::string FIELD6 = "FIELD6";
         const static std::string FIELD7 = "FIELD7";
         const static std::string FIELD8 = "FIELD8";
         const static std::string FIELD9 = "FIELD9";
         const static std::string FIELD10 = "FIELD10";
      }

      namespace MPD26A
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
         const static std::string FIELD6 = "FIELD6";
         const static std::string FIELD7 = "FIELD7";
         const static std::string FIELD8 = "FIELD8";
         const static std::string FIELD9 = "FIELD9";
         const static std::string FIELD10 = "FIELD10";
         const static std::string FIELD11 = "FIELD11";
         const static std::string FIELD12 = "FIELD12";
         const static std::string FIELD13 = "FIELD13";
         const static std::string FIELD14 = "FIELD14";
         const static std::string FIELD15 = "FIELD15";
         const static std::string FIELD16 = "FIELD16";
         const static std::string FIELD17 = "FIELD17";
         const static std::string FIELD18 = "FIELD18";
         const static std::string FIELD19 = "FIELD19";
         const static std::string FIELD20 = "FIELD20";
         const static std::string FIELD21 = "FIELD21";
         const static std::string FIELD22 = "FIELD22";
         const static std::string FIELD23 = "FIELD23";
         const static std::string FIELD24 = "FIELD24";
         const static std::string FIELD25 = "FIELD25";
         const static std::string FIELD26 = "FIELD26";
         const static std::string FIELD27 = "FIELD27";
         const static std::string FIELD28 = "FIELD28";
         const static std::string FIELD29 = "FIELD29";
         const static std::string FIELD30 = "FIELD30";
         const static std::string FIELD31 = "FIELD31";
         const static std::string FIELD32 = "FIELD32";
         const static std::string FIELD33 = "FIELD33";
         const static std::string FIELD34 = "FIELD34";
         const static std::string FIELD35 = "FIELD35";
         const static std::string FIELD36 = "FIELD36";
         const static std::string FIELD37 = "FIELD37";
         const static std::string FIELD38 = "FIELD38";
         const static std::string FIELD39 = "FIELD39";
         const static std::string FIELD40 = "FIELD40";
         const static std::string FIELD41 = "FIELD41";
         const static std::string FIELD42 = "FIELD42";
         const static std::string FIELD43 = "FIELD43";
         const static std::string FIELD44 = "FIELD44";
         const static std::string FIELD45 = "FIELD45";
      }

      namespace PATCHA
      {
         const static std::string PAT_NO = "PAT_NO";
         const static std::string LAST_PAT_FLAG = "LAST_PAT_FLAG";
         const static std::string LNSTRT = "LNSTRT";
         const static std::string LNSTOP = "LNSTOP";
         const static std::string AZL = "AZL";
         const static std::string NVL = "NVL";
         const static std::string FVL = "FVL";
         const static std::string NPIXEL = "NPIXEL";
         const static std::string FVPIX = "FVPIX";
         const static std::string FRAME = "FRAME";
         const static std::string UTC = "UTC";
         const static std::string SHEAD = "SHEAD";
         const static std::string GRAVITY = "GRAVITY";
         const static std::string INS_V_NC = "INS_V_NC";
         const static std::string INS_V_EC = "INS_V_EC";
         const static std::string INS_V_DC = "INS_V_DC";
         const static std::string OFFLAT = "OFFLAT";
         const static std::string OFFLONG = "OFFLONG";
         const static std::string TRACK = "TRACK";
         const static std::string GSWEEP = "GSWEEP";
         const static std::string SHEAR = "SHEAR";
      }

      namespace PATCHB
      {
         const static std::string PAT_NO = "PAT_NO";
         const static std::string LAST_PAT_FLAG = "LAST_PAT_FLAG";
         const static std::string LNSTRT = "LNSTRT";
         const static std::string LNSTOP = "LNSTOP";
         const static std::string AZL = "AZL";
         const static std::string NVL = "NVL";
         const static std::string FVL = "FVL";
         const static std::string NPIXEL = "NPIXEL";
         const static std::string FVPIX = "FVPIX";
         const static std::string FRAME = "FRAME";
         const static std::string UTC = "UTC";
         const static std::string SHEAD = "SHEAD";
         const static std::string GRAVITY = "GRAVITY";
         const static std::string INS_V_NC = "INS_V_NC";
         const static std::string INS_V_EC = "INS_V_EC";
         const static std::string INS_V_DC = "INS_V_DC";
         const static std::string OFFLAT = "OFFLAT";
         const static std::string OFFLONG = "OFFLONG";
         const static std::string TRACK = "TRACK";
         const static std::string GSWEEP = "GSWEEP";
         const static std::string SHEAR = "SHEAR";
         const static std::string BATCH_NO = "BATCH_NO";
      }

      namespace RADSDA
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
         const static std::string FIELD6 = "FIELD6";
         const static std::string FIELD7 = "FIELD7";
         const static std::string FIELD8 = "FIELD8";
         const static std::string FIELD9 = "FIELD9";
         const static std::string FIELD10 = "FIELD10";
         const static std::string FIELD11 = "FIELD11";
         const static std::string FIELD12 = "FIELD12";
         const static std::string FIELD13 = "FIELD13";
         const static std::string FIELD14 = "FIELD14";
         const static std::string FIELD15 = "FIELD15";
         const static std::string FIELD16 = "FIELD16";
         const static std::string FIELD17 = "FIELD17";
         const static std::string FIELD18 = "FIELD18";
         const static std::string FIELD19 = "FIELD19";
         const static std::string FIELD20 = "FIELD20";
         const static std::string FIELD21 = "FIELD21";
         const static std::string FIELD22 = "FIELD22";
         const static std::string FIELD23 = "FIELD23";
         const static std::string FIELD24 = "FIELD24";
         const static std::string FIELD25 = "FIELD25";
         const static std::string FIELD26 = "FIELD26";
         const static std::string FIELD27 = "FIELD27";
         const static std::string FIELD28 = "FIELD28";
         const static std::string FIELD29 = "FIELD29";
         const static std::string FIELD30 = "FIELD30";
         const static std::string FIELD31 = "FIELD31";
         const static std::string FIELD32 = "FIELD32";
         const static std::string FIELD33 = "FIELD33";
         const static std::string FIELD34 = "FIELD34";
         const static std::string FIELD35 = "FIELD35";
         const static std::string FIELD36 = "FIELD36";
         const static std::string FIELD37 = "FIELD37";
         const static std::string FIELD38 = "FIELD38";

         const static std::string FIELD39_ = "FIELD39_";
         const static std::string FIELD40_ = "FIELD40_";
         const static std::string FIELD41_ = "FIELD41_";
         const static std::string FIELD42_ = "FIELD42_";
         const static std::string FIELD43_ = "FIELD43_";
         const static std::string FIELD44_ = "FIELD44_";
         const static std::string FIELD45_ = "FIELD45_";
         const static std::string FIELD46_ = "FIELD46_";

         const static std::string FIELD47 = "FIELD47";
         const static std::string FIELD48 = "FIELD48";
         const static std::string FIELD49 = "FIELD49";
         const static std::string FIELD50 = "FIELD50";
         const static std::string FIELD51 = "FIELD51";
         const static std::string FIELD52 = "FIELD52";

         const static std::string FIELD53_ = "FIELD53_";
         const static std::string FIELD54_ = "FIELD54_";
         const static std::string FIELD55_ = "FIELD55_";
         const static std::string FIELD56_ = "FIELD56_";
         const static std::string FIELD57_ = "FIELD57_";
         const static std::string FIELD58_ = "FIELD58_";
         const static std::string FIELD59_ = "FIELD59_";
         const static std::string FIELD60_ = "FIELD60_";
         const static std::string FIELD61_ = "FIELD61_";
         const static std::string FIELD62_ = "FIELD62_";
         const static std::string FIELD63_ = "FIELD63_";
         const static std::string FIELD64_ = "FIELD64_";
         const static std::string FIELD65_ = "FIELD65_";
         const static std::string FIELD66_ = "FIELD66_";
         const static std::string FIELD67_ = "FIELD67_";

         const static std::string FIELD68 = "FIELD68";
         const static std::string FIELD69 = "FIELD69";
         const static std::string FIELD70 = "FIELD70";

         const static std::string FIELD71_ = "FIELD71_";
         const static std::string FIELD72_ = "FIELD72_";
         const static std::string FIELD73_ = "FIELD73_";
         const static std::string FIELD74_ = "FIELD74_";
         const static std::string FIELD75_ = "FIELD75_";
         const static std::string FIELD76_ = "FIELD76_";
         const static std::string FIELD77_ = "FIELD77_";
         const static std::string FIELD78_ = "FIELD78_";
         const static std::string FIELD79_ = "FIELD79_";
         const static std::string FIELD80_ = "FIELD80_";
         const static std::string FIELD81_ = "FIELD81_";
         const static std::string FIELD82_ = "FIELD82_";
         const static std::string FIELD83_ = "FIELD83_";
         const static std::string FIELD84_ = "FIELD84_";
         const static std::string FIELD85_ = "FIELD85_";
         const static std::string FIELD86_ = "FIELD86_";
         const static std::string FIELD87_ = "FIELD87_";
         const static std::string FIELD88_ = "FIELD88_";
         const static std::string FIELD89_ = "FIELD89_";
         const static std::string FIELD90_ = "FIELD90_";
         const static std::string FIELD91_ = "FIELD91_";
         const static std::string FIELD92_ = "FIELD92_";
         const static std::string FIELD93_ = "FIELD93";
      }

      namespace REFLNA
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
      }

      namespace RPC
      {
         const static std::string LINE_NUMERATOR_COEF_PREFIX = "LNNUMCOEF";
         const static std::string LINE_DENOMINATOR_COEF_PREFIX = "LNDENCOEF";
         const static std::string SAMPLE_NUMERATOR_COEF_PREFIX = "SMPNUMCOEF";
         const static std::string SAMPLE_DENOMINATOR_COEF_PREFIX = "SMPDENCOEF";
         const static std::string LINE_SCALE = "LINE_SCALE";
         const static std::string SAMP_SCALE = "SAMP_SCALE";
         const static std::string LAT_SCALE = "LAT_SCALE";
         const static std::string LONG_SCALE = "LONG_SCALE";
         const static std::string HEIGHT_SCALE = "HEIGHT_SCALE";
         const static std::string LINE_OFFSET = "LINE_OFF";
         const static std::string SAMP_OFFSET = "SAMP_OFF";
         const static std::string LAT_OFFSET = "LAT_OFF";
         const static std::string LONG_OFFSET = "LONG_OFF";
         const static std::string HEIGHT_OFFSET = "HEIGHT_OFF";
         const static std::string SUCCESS = "SUCCESS";
         const static std::string ERR_RAND = "ERR_RAND";
         const static std::string ERR_BIAS = "ERR_BIAS";
      }

      namespace SECTGA
      {
         const static std::string SEC_ID = "SEC_ID";
         const static std::string SEC_BE = "SEC_BE";
         const static std::string RESERVED001 = "RESERVED001";
      }

      namespace SENSRA
      {
         const static std::string REF_ROW = "REF_ROW";
         const static std::string REF_COL = "REF_COL";
         const static std::string SENSOR_MODEL = "SENSOR_MODEL";
         const static std::string SENSOR_MOUNT = "SENSOR_MOUNT";
         const static std::string SENSOR_LOC = "SENSOR_LOC";
         const static std::string SENSOR_ALT_SOURCE = "SENSOR_ALT_SOURCE";
         const static std::string SENSOR_ALT = "SENSOR_ALT";
         const static std::string SENSOR_ALT_UNIT = "SENSOR_ALT_UNIT";
         const static std::string SENSOR_AGL = "SENSOR_AGL";
         const static std::string SENSOR_PITCH = "SENSOR_PITCH";
         const static std::string SENSOR_ROLL = "SENSOR_ROLL";
         const static std::string SENSOR_YAW = "SENSOR_YAW";
         const static std::string PLATFORM_PITCH = "PLATFORM_PITCH";
         const static std::string PLATFORM_ROLL = "PLATFORM_ROLL";
         const static std::string PLATFORM_HDG = "PLATFORM_HDG";
         const static std::string GROUND_SPD_SOURCE = "GROUND_SPD_SOURCE";
         const static std::string GROUND_SPD = "GROUND_SPD";
         const static std::string GRND_SPD_UNIT = "GRND_SPD_UNIT";
         const static std::string GROUND_TRACK = "GROUND_TRACK";
         const static std::string VERT_VEL = "VERT_VEL";
         const static std::string VERT_VEL_UNIT = "VERT_VEL_UNIT";
         const static std::string SWATH_FRAMES = "SWATH_FRAMES";
         const static std::string NUM_SWATHS = "NUM_SWATHS";
         const static std::string SPOT_NUM = "SPOT_NUM";
      }

      namespace SENSRB
      {
         namespace GENERAL_DATA
         {
            const static std::string TAG = "GENERAL_DATA";
            const static std::string SENSOR = "SENSOR";
            const static std::string SENSOR_URI = "SENSOR_URI";
            const static std::string PLATFORM = "PLARFORM";
            const static std::string PLATFORM_URI = "PLATFORM_URI";
            const static std::string OPERATION_DOMAIN = "OPERATION_DOMAIN";
            const static std::string CONTENT_LEVEL = "CONTENT_LEVEL";
            const static std::string GEODETIC_SYSTEM = "GEODETIC_SYSTEM";
            const static std::string GEODETIC_TYPE = "GEODETIC_TYPE";
            const static std::string ELEVATION_DATUM = "ELEVATION_DATUM";
            const static std::string LENGTH_UNIT = "LENGTH_UNIT";
            const static std::string ANGULAR_UNIT = "ANGULAR_UNIT";
            const static std::string START_DATE = "START_DATE";
            const static std::string START_TIME = "START_TIME";
            const static std::string END_DATE = "END_DATE";
            const static std::string END_TIME = "END_TIME";
            const static std::string GENERATION_COUNT = "GENERATION_COUNT";
            const static std::string GENERATION_DATE = "GENERATION_DATE";
            const static std::string GENERATION_TIME = "GENERATION_TIME";
         }
         namespace SENSOR_ARRAY_DATA
         {
            const static std::string TAG = "SENSOR_ARRAY_DATA";
            const static std::string DETECTION = "DETECTION";
            const static std::string ROW_DETECTORS = "ROW_DETECTORS";
            const static std::string COLUMN_DETECTORS = "COLUMN_DETECTORS";
            const static std::string ROW_METRIC = "ROW_METRIC";
            const static std::string COLUMN_METRIC = "COLUMN_METRIC";
            const static std::string FOCAL_LENGTH = "FOCAL_LENGTH";
            const static std::string ROW_FOV = "ROW_FOV";
            const static std::string COLUMN_FOV = "COLUMN_FOV";
            const static std::string CALIBRATED = "CALIBRATED";
         }
         namespace SENSOR_CALIBRATION_DATA
         {
            const static std::string TAG = "SENSOR_CALIBRATION_DATA";
            const static std::string CALIBRATION_UNIT = "CALIBRATION_UNIT";
            const static std::string PRINCIPAL_POINT_OFFSET_X = "PRINCIPAL_POINT_OFFSET_X";
            const static std::string PRINCIPAL_POINT_OFFSET_Y = "PRINCIPAL_POINT_OFFSET_Y";
            const static std::string RADIAL_DISTORT_1 = "RADIAL_DISTORT_1";
            const static std::string RADIAL_DISTORT_2 = "RADIAL_DISTORT_2";
            const static std::string RADIAL_DISTORT_3 = "RADIAL_DISTORT_3";
            const static std::string RADIAL_DISTORT_LIMIT = "RADIAL_DISTORT_LIMIT";
            const static std::string DECENT_DISTORT_1 = "DECENT_DISTORT_1";
            const static std::string DECENT_DISTORT_2 = "DECENT_DISTORT_2";
            const static std::string AFFINITY_DISTORT_1 = "AFFINITY_DISTORT_1";
            const static std::string AFFINITY_DISTORT_2 = "AFFINITY_DISTORT_2";
            const static std::string CALIBRATION_DATE = "CALIBRATION_DATE";
         }
         namespace IMAGE_FORMATION_DATA
         {
            const static std::string TAG = "IMAGE_FORMATION_DATA";
            const static std::string METHOD = "METHOD";
            const static std::string MODE = "MODE";
            const static std::string ROW_COUNT = "ROW_COUNT";
            const static std::string COLUMN_COUNT = "COLUMN_COUNT";
            const static std::string ROW_SET = "ROW_SET";
            const static std::string COLUMN_SET = "COLUMN_SET";
            const static std::string ROW_DETECTION_RATE = "ROW_DETECTION_RATE";
            const static std::string COLUMN_DETECTION_RATE = "COLUMN_DETECTION_RATE";
            const static std::string FIRST_PIXEL_ROW = "FIRST_PIXEL_ROW";
            const static std::string FIRST_PIXEL_COLUMN = "FIRST_PIXEL_COLUMN";
            const static std::string TRANSFORM_PARAMS = "TRANSFORM_PARAMS";
            const static std::string TRANSFORM_PARAM_ = "TRANSFORM_PARAM_";
         }
         const static std::string REFERENCE_TIME = "REFERENCE_TIME";
         const static std::string REFERENCE_ROW = "REFERENCE_ROW";
         const static std::string REFERENCE_COLUMN = "REFERENCE_COLUMN";
         const static std::string LATITUDE_OR_X = "LATITUDE_OR_X";
         const static std::string LONGITUDE_OR_Y = "LONGITUDE_OR_Y";
         const static std::string ALTITUDE_OR_Z = "ALTITUDE_OR_Z";
         const static std::string SENSOR_X_OFFSET = "SENSOR_X_OFFSET";
         const static std::string SENSOR_Y_OFFSET = "SENSOR_Y_OFFSET";
         const static std::string SENSOR_Z_OFFSET = "SENSOR_Z_OFFSET";
         namespace ATTITUDE_EULER_ANGLES
         {
            const static std::string TAG = "ATTITUDE_EULER_ANGLES";
            const static std::string SENSOR_ANGLE_MODEL = "SENSOR_ANGLE_MODEL";
            const static std::string SENSOR_ANGLE_1 = "SENSOR_ANGLE_1";
            const static std::string SENSOR_ANGLE_2 = "SENSOR_ANGLE_2";
            const static std::string SENSOR_ANGLE_3 = "SENSOR_ANGLE_3";
            const static std::string PLATFORM_RELATIVE = "PLATFORM_RELATIVE";
            const static std::string PLATFORM_HEADING = "PLATFORM_HEADING";
            const static std::string PLATFORM_PITCH = "PLATFORM_PITCH";
            const static std::string PLATFORM_ROLL = "PLATFORM_ROLL";
         }
         namespace ATTITUDE_UNIT_VECTORS
         {
            const static std::string TAG = "ATTITUDE_UNIT_VECTORS";
            const static std::string ICX_NORTH_OR_X = "ICX_NORTH_OR_X";
            const static std::string ICX_EAST_OR_Y = "ICX_EAST_OR_Y";
            const static std::string ICX_DOWN_OR_Z = "ICX_DOWN_OR_Z";
            const static std::string ICY_NORTH_OR_X = "ICY_NORTH_OR_X";
            const static std::string ICY_EAST_OR_Y = "ICY_EAST_OR_Y";
            const static std::string ICY_DOWN_OR_Z = "ICY_DOWN_OR_Z";
            const static std::string ICZ_NORTH_OR_X = "ICZ_NORTH_OR_X";
            const static std::string ICZ_EAST_OR_Y = "ICZ_EAST_OR_Y";
            const static std::string ICZ_DOWN_OR_Z = "ICZ_DOWN_OR_Z";
         }
         namespace ATTITUDE_QUATERNION
         {
            const static std::string TAG = "ATTITUDE_QUATERNION";
            const static std::string ATTITUDE_Q1 = "ATTITUDE_Q1";
            const static std::string ATTITUDE_Q2 = "ATTITUDE_Q2";
            const static std::string ATTITUDE_Q3 = "ATTITUDE_Q3";
            const static std::string ATTITUDE_Q4 = "ATTITUDE_Q4";
         }
         namespace SENSOR_VELOCITY_DATA
         {
            const static std::string TAG = "SENSOR_VELOCITY_DATA";
            const static std::string VELOCITY_NORTH_OR_X = "VELOCITY_NORTH_OR_X";
            const static std::string VELOCITY_EAST_OR_Y = "VELOCITY_EAST_OR_Y";
            const static std::string VELOCITY_DOWN_OR_Z = "VELOCITY_DOWN_OR_Z";
         }
         namespace POINT_SET_DATA
         {
            const static std::string TAG = "POINT_SET_DATA";
            const static std::string POINT_SET_TYPE = "POINT_SET_TYPE";
            const static std::string POINT_COUNT = "POINT_COUNT";
            const static std::string P_ROW = "P_ROW";
            const static std::string P_COLUMN = "P_COLUMN";
            const static std::string P_LATITUDE = "P_LATITUDE";
            const static std::string P_LONGITUDE = "P_LONGITUDE";
            const static std::string P_ELEVATION = "P_ELEVATION";
            const static std::string P_RANGE = "P_RANGE";
         }
         namespace TIME_STAMPED_DATA_SETS
         {
            const static std::string TAG = "TIME_STAMPED_DATA_SETS";
            const static std::string TIME_STAMP_TYPE = "TIME_STAMP_TYPE";
            const static std::string TIME_STAMP_COUNT = "TIME_STAMP_COUNT";
            const static std::string TIME_STAMP_TIME = "TIME_STAMP_TIME";
            const static std::string TIME_STAMP_VALUE = "TIME_STAMP_VALUE";
         }
         namespace PIXEL_REFERENCED_DATA_SETS
         {
            const static std::string TAG = "PIXEL_REFERENCED_DATA_SETS";
            const static std::string PIXEL_REFERENCE_TYPE = "PIXEL_REFERENCE_TYPE";
            const static std::string PIXEL_REFERENCE_COUNT = "PIXEL_REFERENCE_COUNT";
            const static std::string PIXEL_REFERENCE_ROW = "PIXEL_REFERENCE_ROW";
            const static std::string PIXEL_REFERENCE_COLUMN = "PIXEL_REFERENCE_COLUMN";
            const static std::string PIXEL_REFERENCE_VALUE = "PIXEL_REFERENCE_VALUE";
         }
         namespace UNCERTAINTY_DATA
         {
            const static std::string TAG = "UNCERTAINTY_DATA";
            const static std::string UNCERTAINTY_FIRST_TYPE = "UNCERTAINTY_FIRST_TYPE";
            const static std::string UNCERTAINTY_SECOND_TYPE = "UNCERTAINTY_SECOND_TYPE";
            const static std::string UNCERTAINTY_VALUE = "UNCERTAINTY_VALUE";
         }
         namespace ADDITIONAL_PARAMETER_DATA
         {
            const static std::string TAG = "ADDITIONAL_PARAMETER_DATA";
            const static std::string PARAMETER_NAME = "PARAMETER_NAME";
            const static std::string PARAMETER_SIZE = "PARAMETER_SIZE";
            const static std::string PARAMETER_COUNT = "PARAMETER_COUNT";
            const static std::string PARAMETER_VALUE = "PARAMETER_VALUE";
         }
      }

      namespace STDIDB
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
         const static std::string FIELD6 = "FIELD6";
         const static std::string FIELD7 = "FIELD7";
         const static std::string FIELD8 = "FIELD8";
         const static std::string FIELD9 = "FIELD9";
         const static std::string FIELD10 = "FIELD10";
         const static std::string FIELD11 = "FIELD11";
         const static std::string FIELD12 = "FIELD12";
         const static std::string FIELD13 = "FIELD13";
         const static std::string FIELD14 = "FIELD14";
         const static std::string FIELD15 = "FIELD15";
         const static std::string FIELD16 = "FIELD16";
         const static std::string FIELD17 = "FIELD17";
      }

      namespace STDIDC
      {
         const static std::string ACQUISITION_DATE = "ACQUISITION_DATE";
         const static std::string MISSION = "MISSION";
         const static std::string PASS = "PASS";
         const static std::string OP_NUM = "OP_NUM";
         const static std::string START_SEGMENT = "START_SEGMENT";
         const static std::string REPRO_NUM = "REPRO_NUM";
         const static std::string REPLAY_REGEN = "REPLAY_REGEN";
         const static std::string BLANK_FILL = "BLANK_FILL";
         const static std::string START_COLUMN = "START_COLUMN";
         const static std::string START_ROW = "START_ROW";
         const static std::string END_SEGMENT = "END_SEGMENT";
         const static std::string END_COLUMN = "END_COLUMN";
         const static std::string END_ROW = "END_ROW";
         const static std::string COUNTRY = "COUNTRY";
         const static std::string WAC = "WAC";
         const static std::string LOCATION = "LOCATION";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string RESERVED3 = "RESERVED3";
      }

      namespace USE00A
      {
         const static std::string ANGLE_TO_NORTH = "ANGLE_TO_NORTH";
         const static std::string MEAN_GSD = "MEAN_GSD";
         const static std::string RESERVED1 = "RESERVED1";
         const static std::string DYNAMIC_RANGE = "DYNAMIC_RANGE";
         const static std::string RESERVED2 = "RESERVED2";
         const static std::string RESERVED3 = "RESERVED3";
         const static std::string RESERVED4 = "RESERVED4";
         const static std::string OBL_ANG = "OBL_ANG";
         const static std::string ROLL_ANG = "ROLL_ANG";
         const static std::string RESERVED5 = "RESERVED5";
         const static std::string RESERVED6 = "RESERVED6";
         const static std::string RESERVED7 = "RESERVED7";
         const static std::string RESERVED8 = "RESERVED8";
         const static std::string RESERVED9 = "RESERVED9";
         const static std::string RESERVED10 = "RESERVED10";
         const static std::string RESERVED11 = "RESERVED11";
         const static std::string N_REF = "N_REF";
         const static std::string REV_NUM = "REV_NUM";
         const static std::string N_SEG = "N_SEG";
         const static std::string MAX_LP_SEG = "MAX_LP_SEG";
         const static std::string RESERVED12 = "RESERVED12";
         const static std::string RESERVED13 = "RESERVED13";
         const static std::string SUN_EL = "SUN_EL";
         const static std::string SUN_AZ = "SUN_AZ";
      }

      namespace USE26A
      {
         const static std::string FIELD1 = "FIELD1";
         const static std::string FIELD2 = "FIELD2";
         const static std::string FIELD3 = "FIELD3";
         const static std::string FIELD4 = "FIELD4";
         const static std::string FIELD5 = "FIELD5";
         const static std::string FIELD6 = "FIELD6";
         const static std::string FIELD7 = "FIELD7";
         const static std::string FIELD8 = "FIELD8";
         const static std::string FIELD9 = "FIELD9";
         const static std::string FIELD10 = "FIELD10";
         const static std::string FIELD11 = "FIELD11";
         const static std::string FIELD12 = "FIELD12";
         const static std::string FIELD13 = "FIELD13";
         const static std::string FIELD14 = "FIELD14";
         const static std::string FIELD15 = "FIELD15";
         const static std::string FIELD16 = "FIELD16";
         const static std::string FIELD17 = "FIELD17";
         const static std::string FIELD18 = "FIELD18";
         const static std::string FIELD19 = "FIELD19";
         const static std::string FIELD20 = "FIELD20";
         const static std::string FIELD21 = "FIELD21";
      }
   }
}

#endif
