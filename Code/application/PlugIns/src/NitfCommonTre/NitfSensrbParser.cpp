/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "DataVariant.h"
#include "DateTime.h"
#include "NitfConstants.h"
#include "NitfSensrbParser.h"
#include "NitfUtilities.h"
#include "ObjectResource.h"
#include "PlugInRegistration.h"

#include <QtCore/QString>
#include <sstream>
#include <string>

using namespace Nitf::TRE;

REGISTER_PLUGIN(OpticksNitfCommonTre, SensrbParser, Nitf::SensrbParser());

namespace
{
   int getDynamicValueTypeLength(const std::string& type)
   {
      int valueLength = 0;
      if (type == "07a" ||
          type == "07e")
      {
         valueLength = 1;
      }
      else if (type == "06d" ||
               type == "06e" ||
               type == "06f")
      {
         valueLength = 8;
      }
      else if (type == "07c" ||
               type == "07f" ||
               type == "07g" ||
               type == "10a" ||
               type == "10b" ||
               type == "10c")
      {
         valueLength = 9;
      }
      else if (type == "07b" ||
               type == "07d" ||
               type == "07h" ||
               type == "08a" ||
               type == "08b" ||
               type == "08c" ||
               type == "08d" ||
               type == "08e" ||
               type == "08f" ||
               type == "08g" ||
               type == "08h" ||
               type == "08i" ||
               type == "09a" ||
               type == "09b" ||
               type == "09c" ||
               type == "09d")
      {
         valueLength = 10;
      }
      else if (type == "06a" ||
               type == "06c")
      {
         valueLength = 11;
      }
      else if (type == "06b")
      {
         valueLength = 12;
      }
      return valueLength;
   }
}

Nitf::SensrbParser::SensrbParser()
{
   setName("SENSRB");
   setDescriptorId("{36ade5ad-bfca-4123-b43f-f91474a495e9}");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

bool Nitf::SensrbParser::runAllTests(Progress* pProgress, std::ostream& failure)
{
   // Bare minimum TRE.
   // Note that this TRE is not actually valid per the spec, but we should still be able to parse it without error.
   static const std::string data(
      "N"            // GENERAL_DATA
      "N"            // SENSOR_ARRAY_DATA
      "N"            // SENSOR_CALIBRATION_DATA
      "N"            // IMAGE_FORMATION_DATA
      "------------" // REFERENCE_TIME
      "--------"     // REFERENCE_ROW
      "--------"     // REFERENCE_COLUMN
      "0.000000000"  // LATITUDE_OR_X
      "0.0000000000" // LONGITUDE_OR_Y
      "0.000000000"  // ALTITUDE_OR_Z
      "00000000"     // SENSOR_X_OFFSET
      "00000000"     // SENSOR_Y_OFFSET
      "00000000"     // SENSOR_Z_OFFSET
      "N"            // ATTITUDE_EULER_ANGLES
      "N"            // ATTITUDE_UNIT_VECTORS
      "N"            // ATTITUDE_QUATERNION
      "N"            // SENSOR_VELOCITY_DATA
      "00"           // POINT_SET_DATA
      "00"           // TIME_STAMPED_DATA_SETS
      "00"           // PIXEL_REFERENCED_DATA_SETS
      "000"          // UNCERTAINTY_DATA
      "000"          // ADDITIONAL_PARAMETER_DATA
      );

   FactoryResource<DynamicObject> treDO;
   size_t numBytes(0);


   // Start of test 1
   std::stringstream input(data);
   numBytes = data.size();

   std::string errorMessage;
   bool success = toDynamicObject(input, numBytes, *treDO.get(), errorMessage);

   if (!errorMessage.empty())
   {
      failure << errorMessage << std::endl;
      errorMessage.clear();
   }

   if (success == false)
   {
      return false;
   }

   TreState status(INVALID);
   if (success == true)
   {
      status = isTreValid(*treDO.get(), failure);
   }

   if (status == INVALID)
   {
      return false;
   }

   std::stringstream tmpStream;
   success = fromDynamicObject(*treDO.get(), tmpStream, numBytes, errorMessage);
   if (success == false)
   {
      failure << errorMessage;
      return false;
   }

   if (input.str() != tmpStream.str())
   {
      failure << "Error: Test with blank data failed: fromDynamicObject returned an unexpected value\n";
      return false;
   }

   treDO->clear();
   return true;
}

Nitf::TreState Nitf::SensrbParser::isTreValid(const DynamicObject& tre, std::ostream& reporter) const
{
   TreState status(VALID);
   return status;
}

bool Nitf::SensrbParser::toDynamicObject(std::istream& input,
                                         size_t numBytes,
                                         DynamicObject& output,
                                         std::string &errorMessage) const
{
   std::vector<char> buf;
   bool success(true);

   // General data
   std::string generalDataPresent;
   Nitf::readAndConvertFromStream(input, generalDataPresent, success, SENSRB::GENERAL_DATA::TAG, 1, errorMessage, buf);
   if (generalDataPresent == "Y")
   {
      {
         FactoryResource<DynamicObject> general;
         output.setAttribute(SENSRB::GENERAL_DATA::TAG, *general.get());
      }
      DynamicObject& general = dv_cast<DynamicObject>(output.getAttribute(SENSRB::GENERAL_DATA::TAG));
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::SENSOR, 25, errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::SENSOR_URI, 32, errorMessage, buf, true, true);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::PLATFORM, 25, errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::PLATFORM_URI, 32, errorMessage, buf, true, true);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::OPERATION_DOMAIN, 10,
         errorMessage, buf);
      Nitf::readField<unsigned short>(input, general, success, SENSRB::GENERAL_DATA::CONTENT_LEVEL, 1,
         errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::GEODETIC_SYSTEM, 5,
         errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::GEODETIC_TYPE, 1, errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::ELEVATION_DATUM, 3,
         errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::LENGTH_UNIT, 2, errorMessage, buf);
      Nitf::readField<std::string>(input, general, success, SENSRB::GENERAL_DATA::ANGULAR_UNIT, 3, errorMessage, buf);
      std::string dateString;
      std::string timeString;
      float secondsInDay = 0.0;

      FactoryResource<DateTime> pStartDate;
      Nitf::readAndConvertFromStream(input, dateString, success, SENSRB::GENERAL_DATA::START_DATE, 8,
         errorMessage, buf);
      Nitf::readAndConvertFromStream(input, timeString, success, SENSRB::GENERAL_DATA::START_TIME, 14,
         errorMessage, buf);
      Nitf::DtgParseCCYYMMDDssss(dateString, timeString, pStartDate.get(), secondsInDay);
      general.setAttribute(SENSRB::GENERAL_DATA::START_DATE, *pStartDate.get());
      general.setAttribute(SENSRB::GENERAL_DATA::START_TIME, secondsInDay);

      FactoryResource<DateTime> pEndDate;
      Nitf::readAndConvertFromStream(input, dateString, success, SENSRB::GENERAL_DATA::END_DATE, 8, errorMessage, buf);
      Nitf::readAndConvertFromStream(input, timeString, success, SENSRB::GENERAL_DATA::END_TIME, 14, errorMessage, buf);
      Nitf::DtgParseCCYYMMDDssss(dateString, timeString, pEndDate.get(), secondsInDay);
      general.setAttribute(SENSRB::GENERAL_DATA::END_DATE, *pEndDate.get());
      general.setAttribute(SENSRB::GENERAL_DATA::END_TIME, secondsInDay);

      Nitf::readField<unsigned short>(input, general, success, SENSRB::GENERAL_DATA::GENERATION_COUNT, 2,
         errorMessage, buf);
      FactoryResource<DateTime> pGenerationDate;
      Nitf::readAndConvertFromStream(input, dateString, success, SENSRB::GENERAL_DATA::GENERATION_DATE, 8,
         errorMessage, buf, true, true);
      Nitf::readAndConvertFromStream(input, timeString, success, SENSRB::GENERAL_DATA::GENERATION_TIME, 10,
         errorMessage, buf, true, true);
      secondsInDay = 0.0;
      Nitf::DtgParseCCYYMMDDssss(dateString, timeString, pGenerationDate.get(), secondsInDay);
      general.setAttribute(SENSRB::GENERAL_DATA::GENERATION_DATE, *pGenerationDate.get());
      general.setAttribute(SENSRB::GENERAL_DATA::GENERATION_TIME, secondsInDay);
   }

   // Sensor array data
   std::string sensorArrayDataPresent;
   Nitf::readAndConvertFromStream(input, sensorArrayDataPresent, success,
      SENSRB::SENSOR_ARRAY_DATA::TAG, 1, errorMessage, buf);
   if (sensorArrayDataPresent == "Y")
   {
      {
         FactoryResource<DynamicObject> sensor;
         output.setAttribute(SENSRB::SENSOR_ARRAY_DATA::TAG, *sensor.get());
      }
      DynamicObject& sensor = dv_cast<DynamicObject>(output.getAttribute(SENSRB::SENSOR_ARRAY_DATA::TAG));
      Nitf::readField<std::string>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::DETECTION, 20, errorMessage, buf);
      Nitf::readField<unsigned int>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::ROW_DETECTORS, 8,
         errorMessage, buf);
      Nitf::readField<unsigned int>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::COLUMN_DETECTORS, 8,
         errorMessage, buf);
      Nitf::readField<double>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::ROW_METRIC, 8,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::COLUMN_METRIC, 8,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::FOCAL_LENGTH, 8,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::ROW_FOV, 8,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::COLUMN_FOV, 8,
         errorMessage, buf, true, true);
      Nitf::readField<std::string>(input, sensor, success, SENSRB::SENSOR_ARRAY_DATA::CALIBRATED, 1, errorMessage, buf);
   }

   // Sensor calibration data
   std::string sensorCalibrationDataPresent;
   Nitf::readAndConvertFromStream(input, sensorCalibrationDataPresent, success,
      SENSRB::SENSOR_CALIBRATION_DATA::TAG, 1, errorMessage, buf);
   if (sensorCalibrationDataPresent == "Y")
   {
      {
         FactoryResource<DynamicObject> calib;
         output.setAttribute(SENSRB::SENSOR_CALIBRATION_DATA::TAG, *calib.get());
      }
      DynamicObject& calib = dv_cast<DynamicObject>(output.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::TAG));

      Nitf::readField<std::string>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::CALIBRATION_UNIT, 2,
         errorMessage, buf);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::PRINCIPAL_POINT_OFFSET_X, 9,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::PRINCIPAL_POINT_OFFSET_Y, 9,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_1, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_2, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_3, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_LIMIT, 9,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::DECENT_DISTORT_1, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::DECENT_DISTORT_2, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::AFFINITY_DISTORT_1, 12,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, calib, success, SENSRB::SENSOR_CALIBRATION_DATA::AFFINITY_DISTORT_2, 12,
         errorMessage, buf, true, true);
      std::string tmp;
      Nitf::readAndConvertFromStream<std::string>(input, tmp, success,
         SENSRB::SENSOR_CALIBRATION_DATA::CALIBRATION_DATE, 8, errorMessage, buf, true, true);
      FactoryResource<DateTime> pCalibDate;
      Nitf::DtgParseCCYYMMDD(tmp, pCalibDate.get());
      calib.setAttribute(SENSRB::SENSOR_CALIBRATION_DATA::CALIBRATION_DATE, *pCalibDate.get());
   }

   // Image formation data
   std::string imageFormationDataPresent;
   Nitf::readAndConvertFromStream(input, imageFormationDataPresent, success,
      SENSRB::IMAGE_FORMATION_DATA::TAG, 1, errorMessage, buf);
   if (imageFormationDataPresent == "Y")
   {
      {
         FactoryResource<DynamicObject> iform;
         output.setAttribute(SENSRB::IMAGE_FORMATION_DATA::TAG, *iform.get());
      }
      DynamicObject& iform = dv_cast<DynamicObject>(output.getAttribute(SENSRB::IMAGE_FORMATION_DATA::TAG));

      Nitf::readField<std::string>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::METHOD, 15, errorMessage, buf);
      Nitf::readField<unsigned short>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::MODE, 3, errorMessage, buf);
      Nitf::readField<unsigned int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::ROW_COUNT, 8,
         errorMessage, buf);
      Nitf::readField<unsigned int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::COLUMN_COUNT, 8,
         errorMessage, buf);
      Nitf::readField<int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::ROW_SET, 8, errorMessage, buf);
      Nitf::readField<int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::COLUMN_SET, 8,
         errorMessage, buf);
      Nitf::readField<float>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::ROW_DETECTION_RATE, 10,
         errorMessage, buf);
      Nitf::readField<float>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::COLUMN_DETECTION_RATE, 10,
         errorMessage, buf);
      Nitf::readField<unsigned int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::FIRST_PIXEL_ROW, 8,
         errorMessage, buf);
      Nitf::readField<unsigned int>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::FIRST_PIXEL_COLUMN, 8,
         errorMessage, buf);
      int numParams = 0;
      Nitf::readAndConvertFromStream(input, numParams, success, SENSRB::IMAGE_FORMATION_DATA::TRANSFORM_PARAMS, 1,
         errorMessage, buf);
      for (int paramId = 1; paramId <= numParams; ++paramId)
      {
         Nitf::readField<double>(input, iform, success, SENSRB::IMAGE_FORMATION_DATA::TRANSFORM_PARAM_
            + StringUtilities::toDisplayString(paramId), 12, errorMessage, buf);
      }
   }

   Nitf::readField<double>(input, output, success, SENSRB::REFERENCE_TIME, 12, errorMessage, buf, true, true);
   Nitf::readField<int>(input, output, success, SENSRB::REFERENCE_ROW, 8, errorMessage, buf, true, true);
   Nitf::readField<int>(input, output, success, SENSRB::REFERENCE_COLUMN, 8, errorMessage, buf, true, true);
   Nitf::readField<double>(input, output, success, SENSRB::LATITUDE_OR_X, 11, errorMessage, buf);
   Nitf::readField<double>(input, output, success, SENSRB::LONGITUDE_OR_Y, 12, errorMessage, buf);
   Nitf::readField<double>(input, output, success, SENSRB::ALTITUDE_OR_Z, 11, errorMessage, buf);
   Nitf::readField<int>(input, output, success, SENSRB::SENSOR_X_OFFSET, 8, errorMessage, buf);
   Nitf::readField<int>(input, output, success, SENSRB::SENSOR_Y_OFFSET, 8, errorMessage, buf);
   Nitf::readField<int>(input, output, success, SENSRB::SENSOR_Z_OFFSET, 8, errorMessage, buf);

   // Attitude euler angles
   std::string attitudeEulerAngles;
   Nitf::readAndConvertFromStream(input, attitudeEulerAngles, success,
      SENSRB::ATTITUDE_EULER_ANGLES::TAG, 1, errorMessage, buf);
   if (attitudeEulerAngles == "Y")
   {
      {
         FactoryResource<DynamicObject> euler;
         output.setAttribute(SENSRB::ATTITUDE_EULER_ANGLES::TAG, *euler.get());
      }
      DynamicObject& euler = dv_cast<DynamicObject>(output.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::TAG));

      Nitf::readField<unsigned short>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_MODEL, 1,
         errorMessage, buf);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_1, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_2, 9,
         errorMessage, buf);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_3, 10,
         errorMessage, buf);
      Nitf::readField<std::string>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_RELATIVE, 1,
         errorMessage, buf);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_HEADING, 9,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_PITCH, 9,
         errorMessage, buf, true, true);
      Nitf::readField<double>(input, euler, success, SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_ROLL, 10,
         errorMessage, buf, true, true);
   }

   // Attitude unit vectors
   std::string attitudeUnitVectors;
   Nitf::readAndConvertFromStream(input, attitudeUnitVectors, success,
      SENSRB::ATTITUDE_UNIT_VECTORS::TAG, 1, errorMessage, buf);
   if (attitudeUnitVectors == "Y")
   {
      {
         FactoryResource<DynamicObject> vectors;
         output.setAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::TAG, *vectors.get());
      }
      DynamicObject& vectors = dv_cast<DynamicObject>(output.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::TAG));

      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICX_NORTH_OR_X, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICX_EAST_OR_Y, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICX_DOWN_OR_Z, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICY_NORTH_OR_X, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICY_EAST_OR_Y, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICY_DOWN_OR_Z, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_NORTH_OR_X, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_EAST_OR_Y, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, vectors, success, SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_DOWN_OR_Z, 10,
         errorMessage, buf);
   }

   // Attitude quaternion
   std::string attitudeQuaternion;
   Nitf::readAndConvertFromStream(input, attitudeQuaternion, success,
      SENSRB::ATTITUDE_QUATERNION::TAG, 1, errorMessage, buf);
   if (attitudeQuaternion == "Y")
   {
      {
         FactoryResource<DynamicObject> quaternion;
         output.setAttribute(SENSRB::ATTITUDE_QUATERNION::TAG, *quaternion.get());
      }
      DynamicObject& quaternion = dv_cast<DynamicObject>(output.getAttribute(SENSRB::ATTITUDE_QUATERNION::TAG));

      Nitf::readField<double>(input, quaternion, success, SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q1, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, quaternion, success, SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q2, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, quaternion, success, SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q3, 10,
         errorMessage, buf);
      Nitf::readField<double>(input, quaternion, success, SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q4, 10,
         errorMessage, buf);
   }

   // Sensor velocity data
   std::string sensorVelocity;
   Nitf::readAndConvertFromStream(input, sensorVelocity, success,
      SENSRB::SENSOR_VELOCITY_DATA::TAG, 1, errorMessage, buf);
   if (sensorVelocity == "Y")
   {
      {
         FactoryResource<DynamicObject> velocity;
         output.setAttribute(SENSRB::SENSOR_VELOCITY_DATA::TAG, *velocity.get());
      }
      DynamicObject& velocity = dv_cast<DynamicObject>(output.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::TAG));

      Nitf::readField<double>(input, velocity, success, SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_NORTH_OR_X, 9,
         errorMessage, buf);
      Nitf::readField<double>(input, velocity, success, SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_EAST_OR_Y, 9,
         errorMessage, buf);
      Nitf::readField<double>(input, velocity, success, SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_DOWN_OR_Z, 9,
         errorMessage, buf);
   }

   // Point set data
   int pointSetDataCount = 0;
   Nitf::readAndConvertFromStream(input, pointSetDataCount, success,
      SENSRB::POINT_SET_DATA::TAG, 2, errorMessage, buf);
   if (pointSetDataCount > 0)
   {
      {
         FactoryResource<DynamicObject> pointSets;
         output.setAttribute(SENSRB::POINT_SET_DATA::TAG, *pointSets.get());
      }
      DynamicObject& pointSets = dv_cast<DynamicObject>(output.getAttribute(SENSRB::POINT_SET_DATA::TAG));

      for (int pointSetDatum = 0; pointSetDatum < pointSetDataCount; ++pointSetDatum)
      {
         {
            FactoryResource<DynamicObject> pointSet;
            pointSets.setAttribute(StringUtilities::toDisplayString(pointSetDatum), *pointSet.get());
         }
         DynamicObject& pointSet = dv_cast<DynamicObject>(pointSets.getAttribute(
            StringUtilities::toDisplayString(pointSetDatum)));
         Nitf::readField<std::string>(input, pointSet, success, SENSRB::POINT_SET_DATA::POINT_SET_TYPE, 25,
            errorMessage, buf);
         int pointCount = 0;
         Nitf::readAndConvertFromStream(input, pointCount, success, SENSRB::POINT_SET_DATA::POINT_COUNT, 3,
            errorMessage, buf);
         std::vector<unsigned int> rows;
         std::vector<unsigned int> columns;
         std::vector<double> latitudes;
         std::vector<double> longitudes;
         std::vector<double> elevations;
         std::vector<double> ranges;
         for (int pointIdx = 0; pointIdx < pointCount; ++pointIdx)
         {
            unsigned int row = 0;
            unsigned int column = 0;
            double latitude = 0.0;
            double longitude = 0.0;
            double elevation = 0.0;
            double range = 0.0;
            Nitf::readAndConvertFromStream(input, row, success, SENSRB::POINT_SET_DATA::P_ROW, 8,
               errorMessage, buf);
            Nitf::readAndConvertFromStream(input, column, success, SENSRB::POINT_SET_DATA::P_COLUMN, 8,
               errorMessage, buf);
            Nitf::readAndConvertFromStream(input, latitude, success, SENSRB::POINT_SET_DATA::P_LATITUDE, 10,
               errorMessage, buf, true, true);
            Nitf::readAndConvertFromStream(input, longitude, success, SENSRB::POINT_SET_DATA::P_LONGITUDE, 11,
               errorMessage, buf, true, true);
            Nitf::readAndConvertFromStream(input, elevation, success, SENSRB::POINT_SET_DATA::P_ELEVATION, 6,
               errorMessage, buf, true, true);
            Nitf::readAndConvertFromStream(input, range, success, SENSRB::POINT_SET_DATA::P_RANGE, 8,
               errorMessage, buf, true, true);
            rows.push_back(row);
            columns.push_back(column);
            latitudes.push_back(latitude);
            longitudes.push_back(longitude);
            elevations.push_back(elevation);
            ranges.push_back(range);
         }
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_ROW, rows);
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_COLUMN, columns);
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_LATITUDE, latitudes);
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_LONGITUDE, longitudes);
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_ELEVATION, elevations);
         pointSet.setAttribute(SENSRB::POINT_SET_DATA::P_RANGE, ranges);
      }
   }

   // Time stamped data
   int timeStampedDataCount = 0;
   Nitf::readAndConvertFromStream(input, timeStampedDataCount, success,
      SENSRB::TIME_STAMPED_DATA_SETS::TAG, 2, errorMessage, buf);
   if (timeStampedDataCount > 0)
   {
      {
         FactoryResource<DynamicObject> timeStampedData;
         output.setAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TAG, *timeStampedData.get());
      }
      DynamicObject& timeStampedData = dv_cast<DynamicObject>(output.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TAG));

      for (int timeStampedDatum = 0; timeStampedDatum < timeStampedDataCount; ++timeStampedDatum)
      {
         {
            FactoryResource<DynamicObject> timeStamped;
            timeStampedData.setAttribute(StringUtilities::toDisplayString(timeStampedDatum), *timeStamped.get());
         }
         DynamicObject& timeStamped = dv_cast<DynamicObject>(
            timeStampedData.getAttribute(StringUtilities::toDisplayString(timeStampedDatum)));
         std::string stampType;
         Nitf::readAndConvertFromStream(input, stampType, success,
            SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TYPE, 3, errorMessage, buf);
         timeStamped.setAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TYPE, stampType);
         int valueLength = getDynamicValueTypeLength(stampType);
         int timeStampCount = 0;
         Nitf::readAndConvertFromStream(input, timeStampCount, success,
            SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_COUNT, 4, errorMessage, buf);
         std::vector<double> timeStampTimes;
         std::vector<double> timeStampValues;
         for (int timeStampIdx = 0; timeStampIdx < timeStampCount; ++timeStampIdx)
         {
            double timeStampTime = 0.0;
            double timeStampValue = 0.0;
            Nitf::readAndConvertFromStream(input, timeStampTime, success,
               SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TIME, 12, errorMessage, buf);
            Nitf::readAndConvertFromStream(input, timeStampValue, success,
               SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_VALUE, valueLength, errorMessage, buf);
            timeStampTimes.push_back(timeStampTime);
            timeStampValues.push_back(timeStampValue);
         }
         timeStamped.setAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TIME, timeStampTimes);
         timeStamped.setAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_VALUE, timeStampValues);
      }
   }

   // Pixel referenced data sets
   int pixelReferencedDataCount = 0;
   Nitf::readAndConvertFromStream(input, pixelReferencedDataCount, success,
      SENSRB::PIXEL_REFERENCED_DATA_SETS::TAG, 2, errorMessage, buf);
   if (pixelReferencedDataCount > 0)
   {
      {
         FactoryResource<DynamicObject> pixelReferencedData;
         output.setAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::TAG, *pixelReferencedData.get());
      }
      DynamicObject& pixelReferencedData = dv_cast<DynamicObject>(
         output.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::TAG));

      for (int pixelReferencedDatum = 0; pixelReferencedDatum < pixelReferencedDataCount; ++pixelReferencedDatum)
      {
         {
            FactoryResource<DynamicObject> pixelReferenced;
            pixelReferencedData.setAttribute(
               StringUtilities::toDisplayString(pixelReferencedDatum), *pixelReferenced.get());
         }
         DynamicObject& pixelReferenced = dv_cast<DynamicObject>(
            pixelReferencedData.getAttribute(StringUtilities::toDisplayString(pixelReferencedDatum)));
         std::string refType;
         Nitf::readAndConvertFromStream(input, refType, success,
            SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_TYPE, 3, errorMessage, buf);
         pixelReferenced.setAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_TYPE, refType);
         int valueLength = getDynamicValueTypeLength(refType);
         int pixelReferenceCount = 0;
         Nitf::readAndConvertFromStream(input, pixelReferenceCount, success,
            SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_COUNT, 4, errorMessage, buf);
         std::vector<int> pixelReferenceRows;
         std::vector<int> pixelReferenceColumns;
         std::vector<double> pixelReferenceValues;
         for (int pixelReferenceIdx = 0; pixelReferenceIdx < pixelReferenceCount; ++pixelReferenceIdx)
         {
            int pixelReferenceRow = 0;
            int pixelReferenceColumn = 0;
            double pixelReferenceValue = 0.0;
            Nitf::readAndConvertFromStream(input, pixelReferenceRow, success,
               SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_ROW, 8, errorMessage, buf);
            Nitf::readAndConvertFromStream(input, pixelReferenceColumn, success,
               SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_COLUMN, 8, errorMessage, buf);
            Nitf::readAndConvertFromStream(input, pixelReferenceValue, success,
               SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_VALUE, valueLength, errorMessage, buf);
            pixelReferenceRows.push_back(pixelReferenceRow);
            pixelReferenceColumns.push_back(pixelReferenceColumn);
            pixelReferenceValues.push_back(pixelReferenceValue);
         }
         pixelReferenced.setAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_ROW,
            pixelReferenceRows);
         pixelReferenced.setAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_COLUMN,
            pixelReferenceColumns);
         pixelReferenced.setAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_VALUE,
            pixelReferenceValues);
      }
   }

   // Uncertainty data
   int uncertaintyCount = 0;
   Nitf::readAndConvertFromStream(input, uncertaintyCount, success,
      SENSRB::UNCERTAINTY_DATA::TAG, 3, errorMessage, buf);
   if (uncertaintyCount > 0)
   {
      {
         FactoryResource<DynamicObject> uncertaintyData;
         output.setAttribute(SENSRB::UNCERTAINTY_DATA::TAG, *uncertaintyData.get());
      }
      DynamicObject& uncertaintyData = dv_cast<DynamicObject>(output.getAttribute(SENSRB::UNCERTAINTY_DATA::TAG));

      for (int uncertaintyIdx = 0; uncertaintyIdx < uncertaintyCount; ++uncertaintyIdx)
      {
         {
            FactoryResource<DynamicObject> uncertainty;
            uncertaintyData.setAttribute(StringUtilities::toDisplayString(uncertaintyIdx), *uncertainty.get());
         }
         DynamicObject& uncertainty = dv_cast<DynamicObject>(
            uncertaintyData.getAttribute(StringUtilities::toDisplayString(uncertaintyIdx)));

         Nitf::readField<std::string>(input, uncertainty, success,
            SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_FIRST_TYPE, 11, errorMessage, buf);
         Nitf::readField<std::string>(input, uncertainty, success,
            SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_SECOND_TYPE, 11, errorMessage, buf, true, true);
         Nitf::readField<double>(input, uncertainty, success,
            SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_VALUE, 10, errorMessage, buf);
      }
   }

   // Additional parameter data
   int additionalCount = 0;
   Nitf::readAndConvertFromStream(input, additionalCount, success,
      SENSRB::ADDITIONAL_PARAMETER_DATA::TAG, 3, errorMessage, buf);
   if (additionalCount > 0)
   {
      {
         FactoryResource<DynamicObject> additionalData;
         output.setAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::TAG, *additionalData.get());
      }
      DynamicObject& additionalData = dv_cast<DynamicObject>(
         output.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::TAG));

      for (int additionalIdx = 0; additionalIdx < additionalCount; ++additionalIdx)
      {
         {
            FactoryResource<DynamicObject> additional;
            additionalData.setAttribute(StringUtilities::toDisplayString(additionalIdx), *additional.get());
         }
         DynamicObject& additional = dv_cast<DynamicObject>(
            additionalData.getAttribute(StringUtilities::toDisplayString(additionalIdx)));

         Nitf::readField<std::string>(input, additional, success, SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_NAME, 25,
            errorMessage, buf);
         unsigned short parameterSize = 0;
         Nitf::readAndConvertFromStream(input, parameterSize, success,
            SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_SIZE, 3, errorMessage, buf);
         additional.setAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_SIZE, parameterSize);
         int parameterCount = 0;
         Nitf::readAndConvertFromStream(input, parameterCount, success,
            SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_COUNT, 4, errorMessage, buf);
         std::vector<std::string> additionalValues;
         for (int parameterIdx = 0; parameterIdx < parameterCount; ++parameterIdx)
         {
            std::string additionalValue;
            Nitf::readAndConvertFromStream(input, additionalValue, success,
               SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_VALUE, parameterSize, errorMessage, buf);
            additionalValues.push_back(additionalValue);
         }
         additional.setAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_VALUE, additionalValues);
      }
   }

   // verify length
   int64_t numRead = input.tellg();
   if (numRead < 0 || static_cast<uint64_t>(numRead) > std::numeric_limits<size_t>::max() ||
      numRead != static_cast<int64_t>(numBytes))
   {
      numReadErrMsg(numRead, numBytes, errorMessage);
      return false;
   }

   return success;
}

bool Nitf::SensrbParser::fromDynamicObject(const DynamicObject& input,
                                           std::ostream& output,
                                           size_t& numBytesWritten,
                                           std::string &errorMessage) const
{
   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   size_t sizeIn = std::max<size_t>(0, static_cast<size_t>(output.tellp()));
   size_t sizeOut(sizeIn);

   try
   {
      if (input.getAttribute(SENSRB::GENERAL_DATA::TAG).isValid())
      {
         const DynamicObject& general =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::GENERAL_DATA::TAG));
         output << "Y";
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::SENSOR)), 25);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::SENSOR_URI)), 32);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::PLATFORM)), 25);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::PLATFORM_URI)), 32);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::OPERATION_DOMAIN)), 10);
         output << toString(dv_cast<unsigned short>(general.getAttribute(SENSRB::GENERAL_DATA::CONTENT_LEVEL)), 1);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::GEODETIC_SYSTEM)), 5);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::GEODETIC_TYPE)), 1);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::ELEVATION_DATUM)), 3);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::LENGTH_UNIT)), 2);
         output << sizeString(dv_cast<std::string>(general.getAttribute(SENSRB::GENERAL_DATA::ANGULAR_UNIT)), 3);
         output << dv_cast<DateTime>(general.getAttribute(SENSRB::GENERAL_DATA::START_DATE)).getFormattedUtc("%Y%m%d");
         output << toString(dv_cast<float>(general.getAttribute(SENSRB::GENERAL_DATA::START_TIME)), 14);
         output << dv_cast<DateTime>(general.getAttribute(SENSRB::GENERAL_DATA::END_DATE)).getFormattedUtc("%Y%m%d");
         output << toString(dv_cast<float>(general.getAttribute(SENSRB::GENERAL_DATA::END_TIME)), 14);
         output << toString(dv_cast<unsigned short>(general.getAttribute(SENSRB::GENERAL_DATA::GENERATION_COUNT)), 2);
         if (general.getAttribute(SENSRB::GENERAL_DATA::GENERATION_DATE).isValid())
         {
            output << dv_cast<DateTime>(
               general.getAttribute(SENSRB::GENERAL_DATA::GENERATION_DATE)).getFormattedUtc("%Y%m%d");
            output << toString(dv_cast<float>(
               general.getAttribute(SENSRB::GENERAL_DATA::GENERATION_TIME)), 10);
         }
         else
         {
            output << "------------------";
         }

      }
      else
      {
         output << "N";
      }
      if (input.getAttribute(SENSRB::SENSOR_ARRAY_DATA::TAG).isValid())
      {
         const DynamicObject& sensor =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::SENSOR_ARRAY_DATA::TAG));
         output << "Y";
         output << sizeString(dv_cast<std::string>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::DETECTION)), 20);
         output << toString(dv_cast<unsigned int>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::ROW_DETECTORS)), 8);
         output << toString(dv_cast<unsigned int>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::COLUMN_DETECTORS)), 8);
         output << toString(dv_cast<double>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::ROW_METRIC)), 8,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::COLUMN_METRIC)), 8,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::FOCAL_LENGTH)), 8,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::ROW_FOV)), 8,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::COLUMN_FOV)), 8,
            -1, '0', false, false, 3, true, '-');
         output << sizeString(dv_cast<std::string>(sensor.getAttribute(SENSRB::SENSOR_ARRAY_DATA::CALIBRATED)), 1);
      }
      else
      {
         output << "N";
      }
      if (input.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::TAG).isValid())
      {
         const DynamicObject& calib =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::TAG));
         output << "Y";
         output << sizeString(dv_cast<std::string>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::CALIBRATION_UNIT)), 2);
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::PRINCIPAL_POINT_OFFSET_X)), 9,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::PRINCIPAL_POINT_OFFSET_Y)), 9,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_1)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_2)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_3)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::RADIAL_DISTORT_LIMIT)), 9,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::DECENT_DISTORT_1)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::DECENT_DISTORT_2)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::AFFINITY_DISTORT_1)), 12,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::AFFINITY_DISTORT_2)), 12,
            -1, '0', false, false, 3, true, '-');
         output << dv_cast<DateTime>(
            calib.getAttribute(SENSRB::SENSOR_CALIBRATION_DATA::CALIBRATION_DATE)).getFormattedUtc("%Y%m%d");
      }
      else
      {
         output << "N";
      }
      if (input.getAttribute(SENSRB::IMAGE_FORMATION_DATA::TAG).isValid())
      {
         const DynamicObject& iform =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::IMAGE_FORMATION_DATA::TAG));
         output << "Y";
         output << sizeString(dv_cast<std::string>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::METHOD)), 15);
         output << toString(dv_cast<unsigned short>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::MODE)), 3);
         output << toString(dv_cast<unsigned int>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::ROW_COUNT)), 8);
         output << toString(dv_cast<unsigned int>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::COLUMN_COUNT)), 8);
         output << toString(dv_cast<int>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::ROW_SET)), 8);
         output << toString(dv_cast<int>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::COLUMN_SET)), 8);
         output << toString(dv_cast<float>(iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::ROW_DETECTION_RATE)), 10);
         output << toString(dv_cast<float>(
            iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::COLUMN_DETECTION_RATE)), 10);
         output << toString(dv_cast<unsigned int>(
            iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::FIRST_PIXEL_ROW)), 8);
         output << toString(dv_cast<unsigned int>(
            iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::FIRST_PIXEL_COLUMN)), 8);
         std::vector<std::string> recNames;
         iform.getAttributeNames(recNames);
         size_t count = 0;
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            if (recName->find(SENSRB::IMAGE_FORMATION_DATA::TRANSFORM_PARAM_) != std::string::npos)
            {
               count++;
            }
         }
         output << toString(count, 1);
         for (size_t paramId = 1; paramId <= count; ++paramId)
         {
            output << toString(dv_cast<double>(
               iform.getAttribute(SENSRB::IMAGE_FORMATION_DATA::TRANSFORM_PARAM_
                  + StringUtilities::toDisplayString(paramId))), 12);
         }
      }
      else
      {
         output << "N";
      }
      output << toString(dv_cast<double>(input.getAttribute(SENSRB::REFERENCE_TIME)), 12,
            -1, '0', false, false, 3, true, '-');
      output << toString(dv_cast<int>(input.getAttribute(SENSRB::REFERENCE_ROW)), 8,
            -1, '0', false, false, 3, true, '-');
      output << toString(dv_cast<int>(input.getAttribute(SENSRB::REFERENCE_COLUMN)), 8,
            -1, '0', false, false, 3, true, '-');
      output << toString(dv_cast<double>(input.getAttribute(SENSRB::LATITUDE_OR_X)), 11);
      output << toString(dv_cast<double>(input.getAttribute(SENSRB::LONGITUDE_OR_Y)), 12);
      output << toString(dv_cast<double>(input.getAttribute(SENSRB::ALTITUDE_OR_Z)), 11);
      output << toString(dv_cast<int>(input.getAttribute(SENSRB::SENSOR_X_OFFSET)), 8);
      output << toString(dv_cast<int>(input.getAttribute(SENSRB::SENSOR_Y_OFFSET)), 8);
      output << toString(dv_cast<int>(input.getAttribute(SENSRB::SENSOR_Z_OFFSET)), 8);

      if (input.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::TAG).isValid())
      {
         const DynamicObject& euler =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::TAG));
         output << "Y";
         output << toString(dv_cast<unsigned short>(
            euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_MODEL)), 1);
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_1)), 10);
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_2)), 9);
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::SENSOR_ANGLE_3)), 10);
         output << sizeString(dv_cast<std::string>(
            euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_RELATIVE)), 1);
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_HEADING)), 9,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_PITCH)), 9,
            -1, '0', false, false, 3, true, '-');
         output << toString(dv_cast<double>(euler.getAttribute(SENSRB::ATTITUDE_EULER_ANGLES::PLATFORM_ROLL)), 10,
            -1, '0', false, false, 3, true, '-');
      }
      else
      {
         output << "N";
      }

      if (input.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::TAG).isValid())
      {
         const DynamicObject& vectors =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::TAG));
         output << "Y";
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICX_NORTH_OR_X)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICX_EAST_OR_Y)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICX_DOWN_OR_Z)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICY_NORTH_OR_X)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICY_EAST_OR_Y)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICY_DOWN_OR_Z)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_NORTH_OR_X)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_EAST_OR_Y)), 10);
         output << toString(dv_cast<double>(vectors.getAttribute(SENSRB::ATTITUDE_UNIT_VECTORS::ICZ_DOWN_OR_Z)), 10);
      }
      else
      {
         output << "N";
      }

      if (input.getAttribute(SENSRB::ATTITUDE_QUATERNION::TAG).isValid())
      {
         const DynamicObject& quaternion =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::ATTITUDE_QUATERNION::TAG));
         output << "Y";
         output << toString(dv_cast<double>(quaternion.getAttribute(SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q1)), 10);
         output << toString(dv_cast<double>(quaternion.getAttribute(SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q2)), 10);
         output << toString(dv_cast<double>(quaternion.getAttribute(SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q3)), 10);
         output << toString(dv_cast<double>(quaternion.getAttribute(SENSRB::ATTITUDE_QUATERNION::ATTITUDE_Q4)), 10);
      }
      else
      {
         output << "N";
      }

      if (input.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::TAG).isValid())
      {
         const DynamicObject& velocity =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::TAG));
         output << "Y";
         output << toString(dv_cast<double>(
            velocity.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_NORTH_OR_X)), 9);
         output << toString(dv_cast<double>(
            velocity.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_EAST_OR_Y)), 9);
         output << toString(dv_cast<double>(
            velocity.getAttribute(SENSRB::SENSOR_VELOCITY_DATA::VELOCITY_DOWN_OR_Z)), 9);
      }
      else
      {
         output << "N";
      }

      if (input.getAttribute(SENSRB::POINT_SET_DATA::TAG).isValid())
      {
         const DynamicObject& pointSets =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::POINT_SET_DATA::TAG));
         std::vector<std::string> recNames;
         pointSets.getAttributeNames(recNames);
         output << toString(recNames.size(), 2);
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            const DynamicObject& pointSet = dv_cast<DynamicObject>(pointSets.getAttribute(*recName));
            output << sizeString(dv_cast<std::string>(
               pointSet.getAttribute(SENSRB::POINT_SET_DATA::POINT_SET_TYPE)), 25);
            const std::vector<unsigned int>& rows =
               dv_cast<std::vector<unsigned int> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_ROW));
            const std::vector<unsigned int>& columns =
               dv_cast<std::vector<unsigned int> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_COLUMN));
            const std::vector<double>& latitudes =
               dv_cast<std::vector<double> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_LATITUDE));
            const std::vector<double>& longitudes =
               dv_cast<std::vector<double> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_LONGITUDE));
            const std::vector<double>& elevations =
               dv_cast<std::vector<double> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_ELEVATION));
            const std::vector<double>& ranges =
               dv_cast<std::vector<double> >(pointSet.getAttribute(SENSRB::POINT_SET_DATA::P_RANGE));
            if (rows.size() != columns.size() ||
                rows.size() != latitudes.size() ||
                rows.size() != longitudes.size() ||
                rows.size() != elevations.size() ||
                rows.size() != ranges.size())
            {
               return false;
            }
            output << toString(rows.size(), 3);
            for (size_t idx = 0; idx < rows.size(); ++idx)
            {
               output << toString(rows[idx], 8);
               output << toString(columns[idx], 8);
               output << toString(latitudes[idx], 10, -1, '0', false, false, 3, true, '-');
               output << toString(longitudes[idx], 11, -1, '0', false, false, 3, true, '-');
               output << toString(elevations[idx], 6, -1, '0', false, false, 3, true, '-');
               output << toString(ranges[idx], 8, -1, '0', false, false, 3, true, '-');
            }
         }
      }
      else
      {
         output << toString(0, 2);
      }

      if (input.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TAG).isValid())
      {
         const DynamicObject& timeStampedData =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TAG));
         std::vector<std::string> recNames;
         timeStampedData.getAttributeNames(recNames);
         output << toString(recNames.size(), 2);
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            const DynamicObject& timeStamped = dv_cast<DynamicObject>(timeStampedData.getAttribute(*recName));
            std::string stampType = dv_cast<std::string>(
               timeStamped.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TYPE));
            int valueLength = getDynamicValueTypeLength(stampType);
            output << sizeString(stampType, 3);
            const std::vector<double>& timeStampTimes = dv_cast<std::vector<double> >(
               timeStamped.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_TIME));
            const std::vector<double>& timeStampValues = dv_cast<std::vector<double> >(
               timeStamped.getAttribute(SENSRB::TIME_STAMPED_DATA_SETS::TIME_STAMP_VALUE));
            if (timeStampTimes.size() != timeStampValues.size())
            {
               return false;
            }
            output << toString(timeStampTimes.size(), 4);
            for (size_t idx = 0; idx < timeStampTimes.size(); ++idx)
            {
               output << toString(timeStampTimes[idx], 12);
               output << toString(timeStampValues[idx], valueLength);
            }
         }
      }
      else
      {
         output << toString(0, 2);
      }

      if (input.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::TAG).isValid())
      {
         const DynamicObject& pixRefData =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::TAG));
         std::vector<std::string> recNames;
         pixRefData.getAttributeNames(recNames);
         output << toString(recNames.size(), 2);
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            const DynamicObject& pixRef = dv_cast<DynamicObject>(pixRefData.getAttribute(*recName));
            std::string pixRefType = dv_cast<std::string>(
               pixRef.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_TYPE));
            int valueLength = getDynamicValueTypeLength(pixRefType);
            output << sizeString(pixRefType, 3);
            const std::vector<int>& pixRefRows = dv_cast<std::vector<int> >(
               pixRef.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_ROW));
            const std::vector<int>& pixRefColumns = dv_cast<std::vector<int> >(
               pixRef.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_COLUMN));
            const std::vector<double>& pixRefValues = dv_cast<std::vector<double> >(
               pixRef.getAttribute(SENSRB::PIXEL_REFERENCED_DATA_SETS::PIXEL_REFERENCE_VALUE));
            if (pixRefRows.size() != pixRefColumns.size() ||
                pixRefRows.size() != pixRefValues.size())
            {
               return false;
            }
            output << toString(pixRefRows.size(), 4);
            for (size_t idx = 0; idx < pixRefRows.size(); ++idx)
            {
               output << toString(pixRefRows[idx], 4);
               output << toString(pixRefColumns[idx], 4);
               output << toString(pixRefValues[idx], valueLength);
            }
         }
      }
      else
      {
         output << toString(0, 2);
      }

      if (input.getAttribute(SENSRB::UNCERTAINTY_DATA::TAG).isValid())
      {
         const DynamicObject& uncertaintyData =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::UNCERTAINTY_DATA::TAG));
         std::vector<std::string> recNames;
         uncertaintyData.getAttributeNames(recNames);
         output << toString(recNames.size(), 3);
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            const DynamicObject& uncertainty = dv_cast<DynamicObject>(uncertaintyData.getAttribute(*recName));
            output << sizeString(dv_cast<std::string>(
               uncertainty.getAttribute(SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_FIRST_TYPE)), 11);
            output << sizeString(dv_cast<std::string>(
               uncertainty.getAttribute(SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_SECOND_TYPE)), 11);
            output << toString(dv_cast<double>(
               uncertainty.getAttribute(SENSRB::UNCERTAINTY_DATA::UNCERTAINTY_VALUE)), 10);
         }
      }
      else
      {
         output << toString(0, 3);
      }

      if (input.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::TAG).isValid())
      {
         const DynamicObject& additionalData =
            dv_cast<const DynamicObject>(input.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::TAG));
         std::vector<std::string> recNames;
         additionalData.getAttributeNames(recNames);
         output << toString(recNames.size(), 3);
         for (std::vector<std::string>::const_iterator recName = recNames.begin();
                  recName != recNames.end(); ++recName)
         {
            const DynamicObject& additionalDatum = dv_cast<DynamicObject>(additionalData.getAttribute(*recName));
            output << sizeString(dv_cast<std::string>(
               additionalDatum.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_NAME)), 25);
            unsigned short parameterSize = dv_cast<unsigned short>(
               additionalDatum.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_SIZE));
            output << toString(parameterSize, 3);
            const std::vector<std::string>& additionalValues = dv_cast<std::vector<std::string> >(
               additionalDatum.getAttribute(SENSRB::ADDITIONAL_PARAMETER_DATA::PARAMETER_VALUE));
            output << toString(additionalValues.size(), 4);
            for (std::vector<std::string>::const_iterator additionalValue = additionalValues.begin();
                 additionalValue != additionalValues.end(); ++additionalValue)
            {
               output << sizeString(*additionalValue, parameterSize);
            }
         }
      }
      else
      {
         output << toString(0, 3);
      }
   }
   catch (const std::bad_cast&)
   {
      return false;
   }

   if (output.tellp() < 0 || static_cast<uint64_t>(output.tellp()) > std::numeric_limits<size_t>::max())
   {
      return false;
   }
   sizeOut = static_cast<size_t>(output.tellp());
   numBytesWritten = sizeOut - sizeIn;
   return true;
}
