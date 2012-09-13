/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAM_H
#define ASPAM_H

#include "AnyData.h"
#include "DateTime.h"
#include "LocationType.h"
#include "ObjectResource.h"
#include "Subject.h"
#include <vector>

/**
 * Storage Atmospheric Slant Path Analysis Model (ASPAM) data.
 *
 * ASPAM is an atmospherics data file format. This class encapsulates
 * some of the information in an ASPAM file. Currently included:
 *    Paragraph A - Point Analysis Site Identification
 *    Paragraph B - Date and Time
 *    Paragraph D - Pseudo-Surface Observation (Weather at Site)
 *    Paragraph F - Wind, Temperature, Absolute Humidity, Density, Pressure
 *    Paragraph G - Remarks
 *    Paragraph H - Aerosol Parameters
 *    Paragraph J - Surface Weather History
 *
 * @note ASPAMs are currently intended to be "set once." There is no ASPAM exporter
 *       and modified ASPAMs will not be recovered during session restore. Only
 *       the ASPAM importer should call set methods.
 *
 * @see      DataElement
 */
class Aspam : public AnyData, public Subject
{
public: // Structures
   /**
    *  This represents cloud layer information from paragraph D.
    */
   struct CloudLayer
   {
      CloudLayer() :
         mCoverage(0),
         mBaseAltitude(0),
         mTopAltitude(0)
      {
      }

      CloudLayer(const CloudLayer& b) :
         mCoverage(b.mCoverage),
         mType(b.mType),
         mBaseAltitude(b.mBaseAltitude),
         mTopAltitude(b.mTopAltitude)
      {
      }

      unsigned int mCoverage;
      std::string mType;
      unsigned int mBaseAltitude;
      unsigned int mTopAltitude;
   };

   /**
    *  This represents analytic data from paragraph F.
    */
   struct Analytic
   {
      Analytic() :
         mUnits(' '),
         mHeight(0),
         mWindDirection(0),
         mWindSpeed(0),
         mTemperature(0),
         mHumidity(0.0),
         mDensity(0.0),
         mPressure(0.0)
      {
      }

      Analytic(const Analytic& b) :
         mUnits(b.mUnits),
         mHeight(b.mHeight),
         mWindDirection(b.mWindDirection),
         mWindSpeed(b.mWindSpeed),
         mTemperature(b.mTemperature),
         mHumidity(b.mHumidity),
         mDensity(b.mDensity),
         mPressure(b.mPressure)
      {
      }

      char mUnits;
      float mHeight;
      float mWindDirection;
      float mWindSpeed;
      float mTemperature;
      float mHumidity;
      float mDensity;
      float mPressure;
   };

   /**
    *  This represents surface aerosol data from paragraph H.
    */
   struct Aerosol
   {
      Aerosol() :
         mHeight(0),
         mPressure(0),
         mTemperature(0),
         mWaterVaporDensity(0),
         mAlternateTemperature(0),
         mAlternateWaterVaporDensity(0),
         mLatitude(0),
         mLongtitude(0),
         mOzoneRatio(0.0)
      {
      }

      Aerosol(const Aerosol& b) :
         mHeight(b.mHeight),
         mPressure(b.mPressure),
         mTemperature(b.mTemperature),
         mWaterVaporDensity(b.mWaterVaporDensity),
         mAlternateTemperature(b.mAlternateTemperature),
         mAlternateWaterVaporDensity(b.mAlternateWaterVaporDensity),
         mLatitude(b.mLatitude),
         mLongtitude(b.mLongtitude),
         mOzoneRatio(b.mOzoneRatio)
      {
      }

      int mHeight;
      unsigned int mPressure;
      int mTemperature;
      unsigned int mWaterVaporDensity;
      int mAlternateTemperature;
      unsigned int mAlternateWaterVaporDensity;
      int mLatitude;
      unsigned int mLongtitude;
      float mOzoneRatio;
   };

   /**
    *  This represents surface weather data from paragraph J.
    */
   struct SurfaceWeather
   {
      unsigned int mYear;
      unsigned int mJulianDay;
      unsigned int mHour;
      unsigned int mMinutes;
      unsigned int mCloudBase1;
      unsigned int mCloudCoverage1;
      unsigned int mCloudThickness1;
      unsigned int mCloudBase2;
      unsigned int mCloudCoverage2;
      unsigned int mCloudThickness2;
      unsigned int mCloudBase3;
      unsigned int mCloudCoverage3;
      unsigned int mCloudThickness3;
      unsigned int mCloudBase4;
      unsigned int mCloudCoverage4;
      unsigned int mCloudThickness4;
      unsigned int mTotalCoverage;
      unsigned int mVisibility;
      std::string mPrecipitationType;
      std::string mObscuration;
      float mPressure;
      float mTemperature;
      float mDewpoint;
      unsigned int mWindSpeed;
      unsigned int mWindDirection;
      unsigned int mAlternateWindSpeed;
      SurfaceWeather() :   mYear(0),
                           mJulianDay(0),
                           mHour(0),
                           mMinutes(0),
                           mCloudBase1(0),
                           mCloudCoverage1(0),
                           mCloudThickness1(0),
                           mCloudBase2(0),
                           mCloudCoverage2(0),
                           mCloudThickness2(0),
                           mCloudBase3(0),
                           mCloudCoverage3(0),
                           mCloudThickness3(0),
                           mCloudBase4(0),
                           mCloudCoverage4(0),
                           mCloudThickness4(0),
                           mTotalCoverage(0),
                           mVisibility(0),
                           mPressure(0.0),
                           mTemperature(0.0),
                           mDewpoint(0.0),
                           mWindSpeed(0),
                           mWindDirection(0),
                           mAlternateWindSpeed(0)
      {}
      SurfaceWeather(const SurfaceWeather& b) :
                           mYear(b.mYear),
                           mJulianDay(b.mJulianDay),
                           mHour(b.mHour),
                           mMinutes(b.mMinutes),
                           mCloudBase1(b.mCloudBase1),
                           mCloudCoverage1(b.mCloudCoverage1),
                           mCloudThickness1(b.mCloudThickness1),
                           mCloudBase2(b.mCloudBase2),
                           mCloudCoverage2(b.mCloudCoverage2),
                           mCloudThickness2(b.mCloudThickness2),
                           mCloudBase3(b.mCloudBase3),
                           mCloudCoverage3(b.mCloudCoverage3),
                           mCloudThickness3(b.mCloudThickness3),
                           mCloudBase4(b.mCloudBase4),
                           mCloudCoverage4(b.mCloudCoverage4),
                           mCloudThickness4(b.mCloudThickness4),
                           mTotalCoverage(b.mTotalCoverage),
                           mVisibility(b.mVisibility),
                           mPrecipitationType(b.mPrecipitationType),
                           mObscuration(b.mObscuration),
                           mPressure(b.mPressure),
                           mTemperature(b.mTemperature),
                           mDewpoint(b.mDewpoint),
                           mWindSpeed(b.mWindSpeed),
                           mWindDirection(b.mWindDirection),
                           mAlternateWindSpeed(b.mAlternateWindSpeed)
      {}
   };

   /**
    *  This represents data from ASPAM/PAR paragraph A.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphA
   {
      LocationType mSiteId;
      bool mLoaded;

      ParagraphA() : mSiteId(0.0, 0.0), mLoaded(false) {}
      ParagraphA(const ParagraphA& b) : mSiteId(b.mSiteId), mLoaded(b.mLoaded) {}
   };

   /**
    *  This represents data from ASPAM/PAR paragraph B.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphB
   {
      FactoryResource<DateTime> mpDateTime;
      bool mLoaded;
      ParagraphB() : mLoaded(false) {}
      ParagraphB(const ParagraphB& other)
      {
         mpDateTime->setStructured(other.mpDateTime->getStructured());
      }
   };

   /**
    *  This represents data from ASPAM/PAR paragraph C.
    *  This is currently an empty data structure since paragraph C
    *  is not parsed.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphC
   {
      // empty
      bool mLoaded;
      ParagraphC() : mLoaded(false) {}
   };

   /**
    *  This represents data from ASPAM/PAR paragraph D.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphD
   {
      ParagraphD() :
         mSurfaceVisibility(0.0),
         mUnits(""),
         mTotalCoverage(0),
         mWindDirection(0),
         mWindSpeed(0),
         mGustSpeed(0),
         mLoaded(false)
      {
      }

      ParagraphD(const ParagraphD& b) :
         mSurfaceVisibility(b.mSurfaceVisibility),
         mUnits(b.mUnits),
         mCloudLayers(b.mCloudLayers),
         mTotalCoverage(b.mTotalCoverage),
         mWindDirection(b.mWindDirection),
         mWindSpeed(b.mWindSpeed),
         mGustSpeed(b.mGustSpeed),
         mRemark(b.mRemark),
         mLoaded(b.mLoaded)
      {
      }

      double mSurfaceVisibility;
      std::string mUnits;
      std::vector<CloudLayer> mCloudLayers;
      unsigned int mTotalCoverage;

      unsigned int mWindDirection;
      unsigned int mWindSpeed;
      unsigned int mGustSpeed;
      std::string mRemark;
      bool mLoaded;
   };


   /**
    *  This represents data from ASPAM/PAR paragraph E.
    *  This is currently an empty data structure since paragraph E
    *  is not parsed.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphE
   {
      // empty
      bool mLoaded;
      ParagraphE() : mLoaded(false) {}
   };

   /**
    *  This represents data from ASPAM/PAR paragraph F.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphF
   {
      ParagraphF() :
         mLoaded(false)
      {
      }

      ParagraphF(const ParagraphF& b) :
         mLevel(b.mLevel),
         mAnalytic(b.mAnalytic),
         mLoaded(b.mLoaded)
      {
      }

      std::string mLevel;
      std::vector<Analytic> mAnalytic;
      bool mLoaded;
   };

   /**
    *  This represents data from ASPAM/PAR paragraph G.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphG
   {
      std::string mRemarks;
      bool mLoaded;

      ParagraphG() : mLoaded(false) {}
      ParagraphG(const ParagraphG& b) : mRemarks(b.mRemarks), mLoaded(b.mLoaded) {}
   };

   /**
    *  This represents data from ASPAM/PAR paragraph H.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphH
   {
      ParagraphH() :
         mLevels(0),
         mPrimaryBoundaryLayerAerosolParameter(0),
         mAirParcelType(0),
         mSeasonalDependence(0),
         mStratosphericAerosol(0),
         mSurfaceVisibility(0),
         mOzoneProfile(0),
         mBoundaryLayerParameterQualityIndex(0),
         mAlternateSurfaceVisibility(0),
         mAlternateBoundaryLayerAerosolParameter(0),
         mAlternateAirParcelType(0),
         mLoaded(false)
      {
      }

      ParagraphH(const ParagraphH& b) :
         mLevels(b.mLevels),
         mPrimaryBoundaryLayerAerosolParameter(b.mPrimaryBoundaryLayerAerosolParameter),
         mAirParcelType(b.mAirParcelType),
         mSeasonalDependence(b.mSeasonalDependence),
         mStratosphericAerosol(b.mStratosphericAerosol),
         mSurfaceVisibility(b.mSurfaceVisibility),
         mOzoneProfile(b.mOzoneProfile),
         mBoundaryLayerParameterQualityIndex(b.mBoundaryLayerParameterQualityIndex),
         mAlternateSurfaceVisibility(b.mAlternateSurfaceVisibility),
         mAlternateBoundaryLayerAerosolParameter(b.mAlternateBoundaryLayerAerosolParameter),
         mAlternateAirParcelType(b.mAlternateAirParcelType),
         mAerosol(b.mAerosol),
         mLoaded(b.mLoaded)
      {
      }

      unsigned int mLevels;
      unsigned int mPrimaryBoundaryLayerAerosolParameter;
      unsigned int mAirParcelType;
      unsigned int mSeasonalDependence;
      unsigned int mStratosphericAerosol;
      unsigned int mSurfaceVisibility;
      unsigned int mOzoneProfile;
      unsigned int mBoundaryLayerParameterQualityIndex;
      unsigned int mAlternateSurfaceVisibility;
      unsigned int mAlternateBoundaryLayerAerosolParameter;
      unsigned int mAlternateAirParcelType;
      std::vector<Aerosol> mAerosol;
      bool mLoaded;
   };

   /**
    *  This represents data from ASPAM/PAR paragraph I.
    *  Paragraph I is identical to paragraph F. It is not
    *  currently parsed.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   typedef ParagraphF ParagraphI;

   /**
    *  This represents data from ASPAM/PAR paragraph J.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphJ
   {
      ParagraphJ() :
         mMaxTemperature(0.0),
         mMinTemperature(0.0),
         mSnowDepth(0.0),
         mLoaded(false)
      {
      }

      ParagraphJ(const ParagraphJ& b) :
         mMaxTemperature(b.mMaxTemperature),
         mMinTemperature(b.mMinTemperature),
         mSnowDepth(b.mSnowDepth),
         mLoaded(b.mLoaded)
      {
      }

      std::vector<SurfaceWeather> mSurfaceWeather;
      float mMaxTemperature;
      float mMinTemperature;
      float mSnowDepth;
      bool mLoaded;
   };

   /**
    *  This represents data from ASPAM/PAR paragraph K.
    *  This is currently an empty data structure since paragraph K
    *  is not parsed.
    *
    *  @see ASPAM Quick Reference Users Handbook
    */
   struct ParagraphK
   {
      // empty
      bool mLoaded;
      ParagraphK() : mLoaded(false) {}
   };

public: // Signals
   /**
    *  Emitted with any<Aspam::ParagraphA> when paragraph A is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphAModified)

   /**
    *  Emitted with any<Aspam::ParagraphB> when paragraph B is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphBModified)

   /**
    *  Emitted with any<Aspam::ParagraphC> when paragraph C is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphCModified)

   /**
    *  Emitted with any<Aspam::ParagraphD> when paragraph D is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphDModified)

   /**
    *  Emitted with any<Aspam::ParagraphE> when paragraph E is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphEModified)

   /**
    *  Emitted with any<Aspam::ParagraphF> when paragraph F is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphFModified)

   /**
    *  Emitted with any<Aspam::ParagraphG> when paragraph G is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphGModified)

   /**
    *  Emitted with any<Aspam::ParagraphH> when paragraph H is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphHModified)

   /**
    *  Emitted with any<Aspam::ParagraphI> when paragraph I is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphIModified)

   /**
    *  Emitted with any<Aspam::ParagraphJ> when paragraph J is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphJModified)

   /**
    *  Emitted with any<Aspam::ParagraphK> when paragraph K is modified.
    */
   SIGNAL_METHOD(Aspam, ParagraphKModified)

public: // Accessors
   /**
    *  Gets the data for paragraph A.
    *
    *  @return  A read-only reference to the paragraph A data
    */
   virtual const ParagraphA& getParagraphA() const = 0;

   /**
    *  Gets the data for paragraph B.
    *
    *  @return  A read-only reference to the paragraph B data
    */
   virtual const ParagraphB& getParagraphB() const = 0;

   /**
    *  Gets the data for paragraph C.
    *  Paragraph C is not currently parsed, this is for future expansion.
    *
    *  @return  A read-only reference to the paragraph C data
    */
   virtual const ParagraphC& getParagraphC() const = 0;

   /**
    *  Gets the data for paragraph D.
    *
    *  @return  A read-only reference to the paragraph D data
    */
   virtual const ParagraphD& getParagraphD() const = 0;

   /**
    *  Gets the data for paragraph E.
    *  Paragraph E is not currently parsed, this is for future expansion.
    *
    *  @return  A read-only reference to the paragraph E data
    */
   virtual const ParagraphE& getParagraphE() const = 0;

   /**
    *  Gets the data for paragraph F.
    *
    *  @return  A read-only reference to the paragraph F data
    */
   virtual const ParagraphF& getParagraphF() const = 0;

   /**
    *  Gets the data for paragraph G.
    *
    *  @return  A read-only reference to the paragraph G data
    */
   virtual const ParagraphG& getParagraphG() const = 0;

   /**
    *  Gets the data for paragraph H.
    *
    *  @return  A read-only reference to the paragraph H data
    */
   virtual const ParagraphH& getParagraphH() const = 0;

   /**
    *  Gets the data for paragraph I.
    *  Paragraph I is not currently parsed, this is for future expansion.
    *
    *  @return  A read-only reference to the paragraph I data
    */
   virtual const ParagraphI& getParagraphI() const = 0;

   /**
    *  Gets the data for paragraph J.
    *
    *  @return  A read-only reference to the paragraph J data
    */
   virtual const ParagraphJ& getParagraphJ() const = 0;

   /**
    *  Gets the data for paragraph K.
    *  Paragraph K is not currently parsed, this is for future expansion.
    *
    *  @return  A read-only reference to the paragraph K data
    */
   virtual const ParagraphK& getParagraphK() const = 0;

   /**
    *  Sets the data for paragraph A.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphAModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphA>.
    */
   virtual void setParagraphA(const Aspam::ParagraphA& val) = 0;
   
   /**
    *  Sets the data for paragraph B.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphBModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphB>
    */
   virtual void setParagraphB(const Aspam::ParagraphB& val) = 0;
   
   /**
    *  Sets the data for paragraph C.
    *  Paragraph C is not currently supported, this is for future expansion.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphCModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphC>
    */
   virtual void setParagraphC(const Aspam::ParagraphC& val) = 0;
   
   /**
    *  Sets the data for paragraph D.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphDModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphD>
    */
   virtual void setParagraphD(const Aspam::ParagraphD& val) = 0;
   
   /**
    *  Sets the data for paragraph E.
    *  Paragraph E is not currently supported, this is for future expansion.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphEModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphE>
    */
   virtual void setParagraphE(const Aspam::ParagraphE& val) = 0;
   
   /**
    *  Sets the data for paragraph F.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphFModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphF>
    */
   virtual void setParagraphF(const Aspam::ParagraphF& val) = 0;
   
   /**
    *  Sets the data for paragraph G.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphGModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphG>
    */
   virtual void setParagraphG(const Aspam::ParagraphG& val) = 0;
   
   /**
    *  Sets the data for paragraph H.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphHModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphH>
    */
   virtual void setParagraphH(const Aspam::ParagraphH& val) = 0;
   
   /**
    *  Sets the data for paragraph I.
    *  Paragraph I is not currently supported, this is for future expansion.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphIModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphI>
    */
   virtual void setParagraphI(const Aspam::ParagraphI& val) = 0;
   
   /**
    *  Sets the data for paragraph J.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphJModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphJ>
    */
   virtual void setParagraphJ(const Aspam::ParagraphJ& val) = 0;
   
   /**
    *  Sets the data for paragraph K.
    *  Paragraph K is not currently supported, this is for future expansion.
    *
    *  @param  val
    *          The data to set.
    *
    *  @notify This will notify signalParagraphKModified() after
    *          the paragraph value is set. The optional argument
    *          is boost::any<ParagraphK>
    */
   virtual void setParagraphK(const Aspam::ParagraphK& val) = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~Aspam() {}
};

#endif
