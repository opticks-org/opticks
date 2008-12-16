/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "GcpGeoreference.h"
#include "GcpGui.h"
#include "GcpList.h"
#include "MatrixFunctions.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <list>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

static double sAnchorFractions[] = {-0.25, 0.25, 0.75, 1.25};
static int sNumAnchorFractions = sizeof(sAnchorFractions) / sizeof(sAnchorFractions[0]);
static int sNumAnchors = sNumAnchorFractions * sNumAnchorFractions;

GcpGeoreference::GcpGeoreference() :
   mpGui(NULL),
   mpRaster(NULL),
   mOrder(0),
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

   fill(mLatCoefficients, &mLatCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)], 0.0);
   fill(mLonCoefficients, &mLonCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)], 0.0);
   fill(mXCoefficients, &mXCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)], 0.0);
   fill(mYCoefficients, &mYCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)], 0.0);
}

GcpGeoreference::~GcpGeoreference()
{
}

bool GcpGeoreference::setInteractive()
{
   ExecutableShell::setInteractive();
   return false;
}

bool GcpGeoreference::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = GeoreferenceShell::getInputSpecification(pArgList);
   success = success && pArgList->addArg<string>("GCP List", string("Corner Coordinates"));
   success = success && pArgList->addArg<int>("Order", 1);
   return success;
}

bool GcpGeoreference::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Run GCP Georeference", "app", "296120A0-1CD5-467E-A501-934BCA7775EA");

   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
   mpProgress = pProgress;

   FAIL_IF(!isBatch(), "Interactive mode is not supported.", return false);

   int numPoints;
   PlugInArg* pArg = NULL;

   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   FAIL_IF(mpRaster == NULL, "Unable to find raster element input", return false);

   string strGcpName;
   GcpList* pGcpList = NULL;

   if (mpGui != NULL)
   {
      mOrder = mpGui->getOrder();
      strGcpName = mpGui->getGcpListName();
   }
   else
   {
      pInArgList->getPlugInArgValue<int>("Order", mOrder);
      pInArgList->getPlugInArgValue<string>("GCP List", strGcpName);
   }

   pGcpList = static_cast<GcpList*>(mpDataModel->getElement(strGcpName, "GcpList", mpRaster));
   if (pGcpList != NULL)
   {
      pStep->addProperty("gcpList", pGcpList->getName());
   }
   pStep->addProperty("polynomialOrder", mOrder);

   FAIL_IF(pGcpList == NULL, "Unable to find GCP list.", return false);

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

   mReverseOrder = min(MAX_ORDER, mOrder+1);

   int numCoeffs = COEFFS_FOR_ORDER(mOrder);
   numPoints = pGcpList->getSelectedPoints().size();
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
   vector<double> latCoeffs(numCoeffs);
   vector<double> lonCoeffs(numCoeffs);
   vector<double> pXCoeffs(numReverseCoeffs);
   vector<double> pYCoeffs(numReverseCoeffs);
   vector<LocationType> latlonValues(numPoints);
   vector<LocationType> pixelValues(numPoints);

   list<GcpPoint>::const_iterator it;
   unsigned int i;
   unsigned int j;
   for (i = 0, it = pGcpList->getSelectedPoints().begin(); 
               it != pGcpList->getSelectedPoints().end();
               it++, i++)
   {
      pixelValues[i] = it->mPixel;
      latlonValues[i] = it->mCoordinate;
   }

   bool badValues = true;
   bool badPixelValues = true;
   for (i = 0; i < pGcpList->getSelectedPoints().size(); i++)
   {
      for (j = i + 1; j < pGcpList->getSelectedPoints().size(); j++)
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

   for (i = 0; i < pGcpList->getSelectedPoints().size(); i++)
   {
      for (j = i + 1; j < pGcpList->getSelectedPoints().size(); j++)
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

   bool success = computeFit(pixelValues, latlonValues, 0, latCoeffs);
   if (success)
   {
      success = computeFit(pixelValues, latlonValues, 1, lonCoeffs);
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor != NULL)
   {
      setCubeSize(pDescriptor->getRowCount(), pDescriptor->getColumnCount());
   }

   copy(latCoeffs.begin(), latCoeffs.end(), mLatCoefficients);
   copy(lonCoeffs.begin(), lonCoeffs.end(), mLonCoefficients);

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
         latlon.mX = computePolynomial(pixel, mOrder, latCoeffs);
         latlon.mY = computePolynomial(pixel, mOrder, lonCoeffs);
         pixelValues.push_back(pixel);
         latlonValues.push_back(latlon);
      }
   }

   if (success)
   {
      success = computeFit(latlonValues, pixelValues, 0, pXCoeffs);
   }
   if (success)
   {
      success = computeFit(latlonValues, pixelValues, 1, pYCoeffs);
   }

   copy(pXCoeffs.begin(), pXCoeffs.end(), mXCoefficients);
   copy(pYCoeffs.begin(), pYCoeffs.end(), mYCoefficients);

   list<GcpPoint> newPoints;
   list<GcpPoint>::iterator npIter;
   for (i = 0, it = pGcpList->getSelectedPoints().begin(); it != pGcpList->getSelectedPoints().end(); it++, i++)
   {
      GcpPoint newPoint;
      newPoint.mPixel.mX = it->mPixel.mX;
      newPoint.mPixel.mY = it->mPixel.mY;
      newPoint.mCoordinate.mX = it->mCoordinate.mX;
      newPoint.mCoordinate.mY = it->mCoordinate.mY;
      LocationType newPixel = geoToPixel(newPoint.mCoordinate);
      newPoint.mRmsError.mX = fabs(newPixel.mX - newPoint.mPixel.mX);
      newPoint.mRmsError.mY = fabs(newPixel.mY - newPoint.mPixel.mY);
      newPoints.push_back(newPoint);
   }

   pGcpList->clearPoints();
   pGcpList->addPoints(newPoints);

   mpGui = NULL;

   if (mpRaster != NULL)
   {
      mpRaster->setGeoreferencePlugin(this);
   }

   pStep->finalize(Message::Success);
   if (mpProgress)
   {
      mpProgress->updateProgress("GcpGeoreference finished.", 0, NORMAL);
   }

   return true;
}

static LocationType solveLinearEquations(LocationType size1, LocationType size2, LocationType delta)
{
   LocationType distance(0.0, 0.0);

   double alpha = size2.mY*size1.mX - size2.mX*size1.mY;
   double beta = size1.mX*delta.mY - size1.mY*delta.mX;
   double gamma = size2.mX*delta.mY - size2.mY*delta.mX;

   if (fabs(alpha) > 1e-16)
   {
      distance.mY = beta / alpha;
      distance.mX = -gamma / alpha;
   }

   return distance;
}

LocationType GcpGeoreference::geoToPixel(LocationType geocoord) const
{
   return evaluatePolynomial(geocoord, mXCoefficients, mYCoefficients, mReverseOrder);
}

LocationType GcpGeoreference::geoToPixelQuick(LocationType geo) const
{
   return geoToPixel(geo);
}

LocationType GcpGeoreference::pixelToGeo(LocationType pixel) const
{
   return evaluatePolynomial(pixel, mLatCoefficients, mLonCoefficients, mOrder);
}

bool GcpGeoreference::canHandleRasterElement(RasterElement *pRaster) const
{
   vector<string> elementNames = mpDataModel->getElementNames(pRaster, "GcpList");
   if (elementNames.empty() == true)
   {
      return false;
   }

   return true;
}

LocationType GcpGeoreference::evaluatePolynomial(LocationType position,
                                                 const double pXCoeffs[],
                                                 const double pYCoeffs[],
                                                 int order) const
{
   double yValue;
   double xyValue;
   LocationType transformedPosition(0.0, 0.0);

   int count = 0;
   for (int i = 0; i <= order; ++i)          // y power
   {
      yValue = pow(position.mY, i);
      for (int j = 0; j <= order - i; ++j)   // x power
      {
         xyValue = pow(position.mX, j) * yValue;
         transformedPosition.mX += pXCoeffs[count] * xyValue;
         transformedPosition.mY += pYCoeffs[count] * xyValue;
         count++;
      }
   }

   return transformedPosition;
}

QWidget *GcpGeoreference::getGui(RasterElement *pRaster)
{
   if (pRaster == NULL)
   {
      return NULL;
   }

   vector<string> gcpNames = mpDataModel->getElementNames(pRaster, "GcpList");

   vector<string> gcpNameList;
   for (vector<string>::iterator iter = gcpNames.begin(); iter != gcpNames.end(); ++iter)
   {
      gcpNameList.push_back(*iter);
   }
   delete mpGui;

   mpGui = new GcpGui(INTERACTIVE_MAX_ORDER, gcpNameList, pRaster);
   return mpGui;
}

bool GcpGeoreference::validateGuiInput() const
{
   if (mpGui != NULL)
   {
      return mpGui->validateInput();
   }
   return false;
}

void GcpGeoreference::setCubeSize(unsigned int numRows, unsigned int numColumns)
{
   mNumRows = numRows;
   mNumColumns = numColumns;
}

bool GcpGeoreference::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter writer("GcpGeoreference");
   if (mpRaster != NULL)
   {
      writer.addAttr("rasterId", mpRaster->getId());
   }
   else
   {
      return false;
   }
   writer.addAttr("order", mOrder);
   writer.addAttr("reverseOrder", mReverseOrder);
   writer.addAttr("numRows", mNumRows);
   writer.addAttr("numCols", mNumColumns);
   XML_ADD_CONTAINER(writer, latCoefficients, &mLatCoefficients[0], &mLatCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)]);
   XML_ADD_CONTAINER(writer, lonCoefficients, &mLonCoefficients[0], &mLonCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)]);
   XML_ADD_CONTAINER(writer, xCoefficients, &mXCoefficients[0], &mXCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)]);
   XML_ADD_CONTAINER(writer, yCoefficients, &mYCoefficients[0], &mYCoefficients[COEFFS_FOR_ORDER(MAX_ORDER)]);
   return serializer.serialize(writer);
}

bool GcpGeoreference::deserialize(SessionItemDeserializer &deserializer)
{
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
      readContainerElements(pRootElement, "latCoefficients", mLatCoefficients, COEFFS_FOR_ORDER(MAX_ORDER));
      readContainerElements(pRootElement, "lonCoefficients", mLonCoefficients, COEFFS_FOR_ORDER(MAX_ORDER));
      readContainerElements(pRootElement, "xCoefficients", mXCoefficients, COEFFS_FOR_ORDER(MAX_ORDER));
      readContainerElements(pRootElement, "yCoefficients", mYCoefficients, COEFFS_FOR_ORDER(MAX_ORDER));
      return true;
   }
   return false;
}

double GcpGeoreference::computePolynomial(LocationType pixel, int order, vector<double> &coeffs)
{
   double yValue;

   int count = 0;
   double value = 0.0;
   for (int i = 0; i <= order; i++)          // y power
   {
      yValue = pow(pixel.mY, i);
      for (int j = 0; j <= order - i; j++)   // x power
      {
         value += coeffs[count] * pow (pixel.mX, j) * yValue;
         count++;
      }
   }

   return value;
}

bool GcpGeoreference::computeFit(const vector<LocationType> &points,
   const vector<LocationType> &values, int which, vector<double> &coefficients)
{
   // The number of GCP points represents the number of rows in the matrix to be solved.
   // The number of coefficients required for the calculation represents the number of columns in the matrix.
   const int numRows = values.size();
   const int numCols = coefficients.size();

   // Create and populate the matrix to be solved.
   MatrixFunctions::MatrixResource<double> pMatrix(numRows, numCols);
   VERIFY(pMatrix.get() != NULL);

   for (int row = 0; row < numRows; ++row)
   {
      basisFunction(points[row], pMatrix[row], numCols);
   }

   // Create a vector of latlon points corresponding to which (X or Y) set of values is to be used.
   // This vector represents the right-hand side of the equation to use when solving the equation.
   vector<double> latlon(numRows);
   for (int point = 0; point < numRows; ++point)
   {
      if (which == 0)
      {
         latlon[point] = values[point].mX;
      }
      else
      {
         latlon[point] = values[point].mY;
      }
   }

   // Solve the equation.
   return MatrixFunctions::solveLinearEquation(&coefficients.front(), pMatrix, &latlon.front(), numRows, numCols);
}

void GcpGeoreference::basisFunction(const LocationType& pixelCoord, double* pBasisValues, int numBasisValues)
{
   // numBasisValues = (order+1)*(order+2)/2, solve for order
   // add fudge factor to prevent x.9999999 being truncated to x
   int order = static_cast<int>((-3.0 + sqrt(9.0 + 8.0 *(numBasisValues-1)))/2.0 + 0.5);
   double yValue;

   int count = 0;
   for (int i = 0; i <= order; i++)          // y power
   {
      yValue = pow(pixelCoord.mY, i);
      for (int j = 0; j <= order - i; j++)   // x power
      {
         pBasisValues[count] = pow (pixelCoord.mX, j) * yValue;
         count++;
      }
   }
}
