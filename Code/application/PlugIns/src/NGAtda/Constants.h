/*
 * Copyright (c) 2011, ITT Geospatial Systems, Ball Aerospace, and Booz Allen
 * Hamilton
 *
 * $Id: Constants.h 92 2012-03-05 18:46:18Z blacktf $
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any
 * later version.
 *
 * Alternatively, you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License and
 * a copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * @author Bradley J Chambers
 */

#ifndef INCLUDE_CONSTANTS_H_
#define INCLUDE_CONSTANTS_H_

namespace camp {

/** NoData value used in matrices and outputted geotiffs */
const float kNoData = -9999;

const double kPI = 3.14159265358979323846;

const float kPIDegree = 180.0f;
const float MAX_SPACING = 4.0f;
const float SPACING_INCREMENT = 0.25f;

enum Curvature {
  CONTOUR = 0,
  PROFILE,
  TANGENTIAL,
  TOTAL
};

enum Rotation {
  EW = 0,
  NESW,
  NS,
  NWSE
};

/** For topography classification, the features are identified with this enum */
enum Feature {
  UNCLASSIFIED = 0,
  PEAK,
  RIDGE,
  PASS,
  PLANE,
  CHANNEL,
  PIT
};


enum SlopeAngleClass {
  ClassI = 0,
  ClassII,
  ClassIII,
  ClassIV,
  ClassV
};

enum CurvatureClass {
  CONVEX = 0,
  PLANAR,
  CONCAVE
};

enum RiskLevel {
  MUDSLIDE_LOW = 0,
  MUDSLIDE_MEDIUM,
  MUDSLIDE_HIGH
};

enum PostAlgorithm {
  MIN = 0,      /*!< Minimum point value for the post. */
  MAX = 1,      /*!< Maximum point value for the post. */
  AVERAGE = 3,  /*!< Average point value for the post. */
  CLOSEST = 4   /*!< Closest post (Not fully implemented for DEM generation). */
};

enum Result {
  FAILURE = 0,
  SUCCESS
};
}  // namespace camp

#endif  // INCLUDE_CONSTANTS_H_