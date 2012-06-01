/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "GcpGeoreference.h"
#include "GcpGui.h"
#include "GcpList.h"
#include "Georeference.h"
#include "GeoreferenceDescriptor.h"
#include "GeoreferenceUtilities.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "TypeConverter.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <list>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksGeoreference, GcpGeoreference);

static double sAnchorFractions[] = {-0.25, 0.25, 0.75, 1.25};
static int sNumAnchorFractions = sizeof(sAnchorFractions) / sizeof(sAnchorFractions[0]);
static int sNumAnchors = sNumAnchorFractions * sNumAnchorFractions;

GcpGeoreference::GcpGeoreference() :
   mpGui(NULL),
   mpRaster(NULL),
   mOrder(1),
   mReverseOrder(0),
   mNumRows(0),
   mNumColumns(0),
   mpProgress(NULL)
{
   setName("GCP Georeference");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("GCP Georeferencing plugin");
   setDescriptorId("{424C2028-1072-4e8a-933F-56B9730EE301}");
   allowMultipleInstances(true);
   executeOnStartup(false);
   destroyAfterExecute(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GcpGeoreference::~GcpGeoreference()
{
   delete mpGui;
}

bool GcpGeoreference::getInputSpecification(PlugInArgList*& pArgList)
{
   if (GeoreferenceShell::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   // Do not set a default value in the input args so that the georeference
   // descriptor values will be used if the arg value is not set
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<GcpList>(Georeference::GcpListArg(), "The GCPs to use for the georeferencing.  If not "
      "specified, the GCP list contained in the " + Executable::DataElementArg() + " input will be used.") == true);
   VERIFY(pArgList->addArg<int>("Order", "Polynomial order for the georeferencing calculations.  If not specified, "
      "the polynomial order contained in the " + Executable::DataElementArg() + " input will be used.") == true);

   return true;
}

bool GcpGeoreference::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run GCP Georeference", "app", "296120A0-1CD5-467E-A501-934BCA7775EA");

   // Get the values from the input arg list
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   mpProgress = pProgress;

   FAIL_IF(!isBatch(), "Interactive mode is not supported.", return false);

   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   FAIL_IF(mpRaster == NULL, "Unable to find raster element input", return false);

   GcpList* pGcpList = pInArgList->getPlugInArgValue<GcpList>(Georeference::GcpListArg());
   bool gcpListArgSet = (pGcpList != NULL);
   bool orderArgSet = pInArgList->getPlugInArgValue<int>("Order", mOrder);

   // If the polynomial order or GCP list were not contained in the arg list,
   // get the values from the georeference descriptor
   GeoreferenceDescriptor* pGeorefDescriptor = NULL;

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   }

   if (pGeorefDescriptor != NULL)
   {
      if (gcpListArgSet == false)
      {
         string gcpListName =
            dv_cast<string>(pGeorefDescriptor->getAttributeByPath("GCP Georeference/GcpListName"), string());
         if (gcpListName.empty() == false)
         {
            GcpList* pAttributeGcpList = static_cast<GcpList*>(mpDataModel->getElement(gcpListName,
               TypeConverter::toString<GcpList>(), mpRaster));
            if (pAttributeGcpList != NULL)
            {
               pGcpList = pAttributeGcpList;
            }
         }
      }

      if (orderArgSet == false)
      {
         mOrder = dv_cast<int>(pGeorefDescriptor->getAttributeByPath("GCP Georeference/PolynomialOrder"), mOrder);
      }
   }

   FAIL_IF(pGcpList == NULL, "Unable to find the GCP list.", return false);

   if ((mOrder <= 0) || (mOrder > MAX_ORDER))
   {
      if (mpProgress)
      {
         stringstream buffer;
         buffer << "Invalid polynomial order: " << mOrder << "\nThe order must be between 1 and " <<
            MAX_ORDER << " (inclusive)";
         mpProgress->updateProgress(buffer.str(), 0, ERRORS);
      }
      pStep->addProperty("Max Polynomial Order", MAX_ORDER);
      pStep->finalize(Message::Failure, "Invalid polynomial order");
      return false;
   }

   pStep->addProperty("gcpList", pGcpList->getName());
   pStep->addProperty("polynomialOrder", mOrder);

   mReverseOrder = min(MAX_ORDER, mOrder+1);

   int numCoeffs = COEFFS_FOR_ORDER(mOrder);
   int numPoints = pGcpList->getSelectedPoints().size();
   if (numPoints < numCoeffs)
   {
      if (mpProgress)
      {
         stringstream buffer;
         buffer << "Too few ground control points.\n" << "A polynomial fit of order " << mOrder <<
            "\nrequires at least " << numCoeffs << " GCPs.";
         mpProgress->updateProgress(buffer.str(), 0, ERRORS);
      }
      pStep->addProperty("Required GCPs", numCoeffs);
      pStep->finalize(Message::Failure, "Too few ground control points");
      return false;
   }

   int numReverseCoeffs = COEFFS_FOR_ORDER(mReverseOrder);
   mLatCoefficients.resize(numCoeffs, 0.0);
   mLonCoefficients.resize(numCoeffs, 0.0);
   mXCoefficients.resize(numReverseCoeffs, 0.0);
   mYCoefficients.resize(numReverseCoeffs, 0.0);
   vector<LocationType> latlonValues(numPoints);
   vector<LocationType> pixelValues(numPoints);
   double maxLonSeparation(0.0);

   list<GcpPoint>::const_iterator it;
   unsigned int i;
   unsigned int j;
   for (i = 0, it = pGcpList->getSelectedPoints().begin(); 
               it != pGcpList->getSelectedPoints().end();
               ++it, ++i)
   {
      pixelValues[i] = it->mPixel;
      latlonValues[i] = it->mCoordinate;
   }

   // Find the maximum separation to determine if it is the antimeridian or the poles
   for (i = 0; i < pGcpList->getSelectedPoints().size(); ++i)
   {
      for (j = i + 1; j < pGcpList->getSelectedPoints().size(); ++j)
      {
         if (fabs(latlonValues[i].mY - latlonValues[j].mY) > maxLonSeparation)
         {
            maxLonSeparation = fabs(latlonValues[i].mY - latlonValues[j].mY);
         }
      }
   }

   bool badValues = true;
   bool badPixelValues = true;
   for (i = 0; i < pGcpList->getSelectedPoints().size(); ++i)
   {
      for (j = i + 1; j < pGcpList->getSelectedPoints().size(); ++j)
      {
         if (fabs(latlonValues[i].mX - latlonValues[j].mX) > 1e-20)
         {
            badValues = false;
            break;
         }
         if (fabs(latlonValues[i].mY - latlonValues[j].mY) > 1e-20)
         {
            badValues = false;
            break;
         }
      }
   }

   //#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This is a short term solution " \
   //"the draw method in LatLonLayer needs to be changed! (mconsidi)")
   // Special lon cases of the Antimeridian and poles
   // A value of more than 180.0 in maxLonSeparation indicates a special condition
   if (maxLonSeparation > 180.0)
   {
      for (i = 0; i < pGcpList->getSelectedPoints().size(); ++i)
      {
         if (latlonValues[i].mY < 0.0)
         {
            latlonValues[i].mY = 360.0 + latlonValues[i].mY;
         }
      }
   }

   if (badValues)
   {
      mMessageText = "All GCPs have the same value.";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, mMessageText);
      return false;
   }

   for (i = 0; i < pGcpList->getSelectedPoints().size(); ++i)
   {
      for (j = i + 1; j < pGcpList->getSelectedPoints().size(); ++j)
      {
         if (fabs(pixelValues[i].mX - pixelValues[j].mX) > 1e-20)
         {
            badPixelValues = false;
            break;
         }
         if (fabs(pixelValues[i].mY - pixelValues[j].mY) > 1e-20)
         {
            badPixelValues = false;
            break;
         }
      }
   }

   if (badPixelValues)
   {
      mMessageText = "All GCPs have the same pixel location value.";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessageText, 0, ERRORS);
      }
      pStep->finalize(Message::Failure, mMessageText);
      return false;
   }

   mMessageText = "GcpGeoreference started.";
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessageText, 0, NORMAL);
   }

   bool success = GeoreferenceUtilities::computeFit(pixelValues, latlonValues, 0, mLatCoefficients);
   if (success)
   {
      success = GeoreferenceUtilities::computeFit(pixelValues, latlonValues, 1, mLonCoefficients);
   }

   if (pDescriptor != NULL)
   {
      setCubeSize(pDescriptor->getRowCount(), pDescriptor->getColumnCount());
   }

   // Generate a gridSize x gridSize grid of GCPs calculated from the 
   // frontwards (pixel-to-lat/lon) polynomial. Then generate the reverse 
   // (lat/lon-to-pixel) polynomial from this grid of GCPs.
   const int gridSize = 30;
   pixelValues.clear();
   latlonValues.clear();
   for (i = 0; i < gridSize; ++i)
   {
      for (j = 0; j < gridSize; ++j)
      {
         LocationType pixel;
         LocationType latlon;
         pixel.mX = i * mNumColumns / gridSize;
         pixel.mY = j * mNumRows / gridSize;
         latlon.mX = GeoreferenceUtilities::computePolynomial(pixel, mOrder, mLatCoefficients);
         latlon.mY = GeoreferenceUtilities::computePolynomial(pixel, mOrder, mLonCoefficients);
         pixelValues.push_back(pixel);
         latlonValues.push_back(latlon);
      }
   }

   if (success)
   {
      success = GeoreferenceUtilities::computeFit(latlonValues, pixelValues, 0, mXCoefficients);
   }
   if (success)
   {
      success = GeoreferenceUtilities::computeFit(latlonValues, pixelValues, 1, mYCoefficients);
   }

   list<GcpPoint> newPoints;
   list<GcpPoint>::iterator npIter;
   for (i = 0, it = pGcpList->getSelectedPoints().begin(); it != pGcpList->getSelectedPoints().end(); ++it, ++i)
   {
      GcpPoint newPoint;
      newPoint.mPixel.mX = it->mPixel.mX;
      newPoint.mPixel.mY = it->mPixel.mY;
      newPoint.mCoordinate.mX = it->mCoordinate.mX;
      newPoint.mCoordinate.mY = it->mCoordinate.mY;
      // If maxLonSeparation > 180.0 then this is a special case
      if (maxLonSeparation > 180.0 && newPoint.mCoordinate.mY < 0.0)
      {
         newPoint.mCoordinate.mY = newPoint.mCoordinate.mY + 360.0;
      }
      LocationType newPixel = geoToPixel(newPoint.mCoordinate);
      newPoint.mRmsError.mX = fabs(newPixel.mX - newPoint.mPixel.mX);
      newPoint.mRmsError.mY = fabs(newPixel.mY - newPoint.mPixel.mY);
      newPoints.push_back(newPoint);
   }

   pGcpList->clearPoints();
   pGcpList->addPoints(newPoints);

   mpRaster->setGeoreferencePlugin(this);

   // Update the georeference descriptor with the current georeference parameters if necessary
   if (pGeorefDescriptor != NULL)
   {
      // Georeference plug-in
      const string& plugInName = getName();
      pGeorefDescriptor->setGeoreferencePlugInName(plugInName);

      // GCP list
      if ((gcpListArgSet == true) && (pGcpList != NULL))
      {
         const string& gcpListName = pGcpList->getName();
         pGeorefDescriptor->setAttributeByPath("GCP Georeference/GcpListName", gcpListName);
      }

      // Polynomial order
      if (orderArgSet == true)
      {
         pGeorefDescriptor->setAttributeByPath("GCP Georeference/PolynomialOrder", mOrder);
      }
   }

   pStep->finalize(Message::Success);
   if (mpProgress)
   {
      mpProgress->updateProgress("GcpGeoreference finished.", 0, NORMAL);
   }

   return true;
}

unsigned char GcpGeoreference::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return Georeference::CAN_NOT_GEOREFERENCE;
   }

   // Check if the raster data is already loaded in the data model
   RasterElement* pRaster = dynamic_cast<RasterElement*>(mpDataModel->getElement(pDescriptor));
   if (pRaster != NULL)
   {
      // Raster data is loaded, so check for GCP lists with the raster element as a parent
      vector<string> elementNames = mpDataModel->getElementNames(pRaster, TypeConverter::toString<GcpList>());
      if (elementNames.empty() == false)
      {
         return Georeference::CAN_GEOREFERENCE;
      }
   }
   else
   {
      // Raster data is not loaded, so check for valid GCPs in the file descriptor
      const RasterFileDescriptor* pFileDescriptor =
         dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      if (pFileDescriptor != NULL)
      {
         const list<GcpPoint>& gcps = pFileDescriptor->getGcps();
         if (gcps.empty() == false)
         {
            return Georeference::CAN_GEOREFERENCE;
         }
      }
   }

   return CAN_NOT_GEOREFERENCE;
}

QWidget* GcpGeoreference::getWidget(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create the widget
   if (mpGui == NULL)
   {
      mpGui = new GcpGui(INTERACTIVE_MAX_ORDER);
   }

   // Set the georeference data into the widget for the given raster data
   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      RasterElement* pRaster = dynamic_cast<RasterElement*>(mpDataModel->getElement(pDescriptor));
      if (pRaster != NULL)
      {
         mpGui->setGeoreferenceData(pGeorefDescriptor, pRaster);
      }
      else
      {
         RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
         if (pFileDescriptor != NULL)
         {
            mpGui->setGeoreferenceData(pGeorefDescriptor, pFileDescriptor->getGcps());
         }
      }
   }

   return mpGui;
}

bool GcpGeoreference::validate(const RasterDataDescriptor* pDescriptor, string& errorMessage) const
{
   if (GeoreferenceShell::validate(pDescriptor, errorMessage) == false)
   {
      return false;
   }

   VERIFY(pDescriptor != NULL);

   const GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   VERIFY(pGeorefDescriptor != NULL);

   // Check if the raster data is already loaded in the data model
   GcpList* pGcpList = NULL;
   list<GcpPoint> gcps;

   RasterElement* pRaster = dynamic_cast<RasterElement*>(mpDataModel->getElement(pDescriptor));
   if (pRaster != NULL)
   {
      // Raster data is loaded, so check the GCP list name
      string gcpList = dv_cast<string>(pGeorefDescriptor->getAttributeByPath("GCP Georeference/GcpListName"),
         string());
      if (gcpList.empty() == true)
      {
         errorMessage = "The GCP list name is invalid.";
         return false;
      }

      // Check for a valid GCP list
      pGcpList = dynamic_cast<GcpList*>(Service<ModelServices>()->getElement(gcpList,
         TypeConverter::toString<GcpList>(), pRaster));
      if (pGcpList == NULL)
      {
         errorMessage = "Could not get the specified GCP list.";
         return false;
      }
   }
   else
   {
      // Raster data is not loaded, so check for valid GCPs in the file descriptor
      const RasterFileDescriptor* pFileDescriptor =
         dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      if (pFileDescriptor == NULL)
      {
         errorMessage = "The raster data set does not contain valid file information.";
         return false;
      }

      gcps = pFileDescriptor->getGcps();
      if (gcps.empty() == true)
      {
         errorMessage = "The raster data set does not contain valid GCPs.";
         return false;
      }
   }

   // The polynomial order cannot be zero or negative
   int order = dv_cast<int>(pGeorefDescriptor->getAttributeByPath("GCP Georeference/PolynomialOrder"), 1);
   if (order <= 0)
   {
      errorMessage = "The polynomial order is invalid.";
      return false;
   }

   // The number of GCPs must support the polynomial order
   int gcpsRequired = COEFFS_FOR_ORDER(order);
   int gcpsPresent = 0;

   if (pGcpList != NULL)
   {
      gcpsPresent = static_cast<int>(pGcpList->getSelectedPoints().size());
   }
   else
   {
      gcpsPresent = static_cast<int>(gcps.size());
   }

   if (gcpsPresent < gcpsRequired)
   {
      errorMessage = "There are not enough GCPs present to support the specified polynomial order.";
      return false;
   }

   return true;
}

LocationType GcpGeoreference::geoToPixel(LocationType geocoord, bool* pAccurate) const
{
   LocationType pixcoord = GeoreferenceUtilities::evaluatePolynomial(
      geocoord, mXCoefficients, mYCoefficients, mReverseOrder);
   if (pAccurate != NULL)
   {
      bool outsideCols = pixcoord.mX < 0.0 || pixcoord.mX > static_cast<double>(mNumColumns);
      bool outsideRows = pixcoord.mY < 0.0 || pixcoord.mY > static_cast<double>(mNumRows);
      *pAccurate = !(outsideCols || outsideRows);
   }

   return pixcoord;
}

LocationType GcpGeoreference::pixelToGeo(LocationType pixel, bool* pAccurate) const
{
   if (pAccurate != NULL)
   {
      bool outsideCols = pixel.mX < 0.0 || pixel.mX > static_cast<double>(mNumColumns);
      bool outsideRows = pixel.mY < 0.0 || pixel.mY > static_cast<double>(mNumRows);
      *pAccurate = !(outsideCols || outsideRows);
   }
  return GeoreferenceUtilities::evaluatePolynomial(pixel, mLatCoefficients, mLonCoefficients, mOrder);
}

void GcpGeoreference::setCubeSize(unsigned int numRows, unsigned int numColumns)
{
   mNumRows = numRows;
   mNumColumns = numColumns;
}

bool GcpGeoreference::serialize(SessionItemSerializer& serializer) const
{
   if (mpRaster == NULL)
   {
      // execute() has not yet been called so there is no need to serialize any parameters
      return true;
   }

   XMLWriter writer("GcpGeoreference");
   writer.addAttr("rasterId", mpRaster->getId());
   writer.addAttr("order", mOrder);
   writer.addAttr("reverseOrder", mReverseOrder);
   writer.addAttr("numRows", mNumRows);
   writer.addAttr("numCols", mNumColumns);
   writer.addAttr("latCoefficients", mLatCoefficients);
   writer.addAttr("lonCoefficients", mLonCoefficients);
   writer.addAttr("xCoefficients", mXCoefficients);
   writer.addAttr("yCoefficients", mYCoefficients);
   return serializer.serialize(writer);
}

bool GcpGeoreference::deserialize(SessionItemDeserializer& deserializer)
{
   if (deserializer.getBlockSizes().empty() == true)
   {
      // The plug-in was serialized before execute() was called
      return true;
   }

   XmlReader::StringStreamAssigner<int> toInt;
   XmlReader::StringStreamAssigner<unsigned short> toUShort;
   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "GcpGeoreference");
   if (pRootElement)
   {
      string rasterId = A(pRootElement->getAttribute(X("rasterId")));
      mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(rasterId));
      if (mpRaster == NULL)
      {
         return false;
      }
      mOrder = toInt(A(pRootElement->getAttribute(X("order"))));
      mReverseOrder = toUShort(A(pRootElement->getAttribute(X("reverseOrder"))));
      mNumRows = toInt(A(pRootElement->getAttribute(X("numRows"))));
      mNumColumns = toInt(A(pRootElement->getAttribute(X("numCols"))));
      mLatCoefficients = *reinterpret_cast<std::vector<double>*>(
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(
            pRootElement->getAttribute(X("latCoefficients"))));
      mLonCoefficients = *reinterpret_cast<std::vector<double>*>(
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(
            pRootElement->getAttribute(X("lonCoefficients"))));
      mXCoefficients = *reinterpret_cast<std::vector<double>*>(
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(
            pRootElement->getAttribute(X("xCoefficients"))));
      mYCoefficients = *reinterpret_cast<std::vector<double>*>(
         XmlReader::StrToVector<double, XmlReader::StringStreamAssigner<double> >(
            pRootElement->getAttribute(X("yCoefficients"))));
      return true;
   }
   return false;
}
