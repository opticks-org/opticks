/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVersion.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "GraphicGroup.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "LocationType.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "PseudocolorLayer.h"
#include "QtCluster.h"
#include "QtClusterGui.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtGui/QApplication>
#include <limits>
#include <math.h>
#if !defined(DEBUG)
// boost optimizations on
#define NDEBUG
#endif
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/symmetric.hpp>

using namespace boost::numeric;

REGISTER_PLUGIN_BASIC(OpticksObjectFinding, QtCluster);

namespace
{
typedef QList<QPoint> PointsType;
typedef ublas::symmetric_matrix<int> MatrixType;
}

QtCluster::QtCluster()
{
   setName("QT Cluster");
   setDescription("Cluster points in an AOI using the quality threshold (QT) cluster algorithm.");
   setDescriptorId("{FADAA529-8A37-4ebb-898D-8326C86220F0}");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
   setMenuLocation("[General Algorithms]/QT Cluster");
}

QtCluster::~QtCluster()
{
}

bool QtCluster::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   if (isBatch())
   {
      VERIFY(pInArgList->addArg<AoiElement>(Executable::DataElementArg(), "AOI element to perform clustering over."));
   }
   else
   {
      VERIFY(pInArgList->addArg<AoiLayer>(Executable::LayerArg(), "AOI layer to perform clustering over."));
      VERIFY(pInArgList->addArg<SpatialDataView>(Executable::ViewArg(), "View containing the AOI to perform clustering over."));
   }
//#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Might be useful to allow real-world cluster " \
//                                                            "sizes. (feet, meters, DMS, etc.) (tclarke)")
   VERIFY(pInArgList->addArg<double>("Cluster Size", 15.0, "Maximum cluster size in pixels. Default is 15."));
   std::string typeDesc = "Type of display created. Valid values are: ";
   std::vector<std::string> vals = StringUtilities::getAllEnumValuesAsDisplayString<DisplayType>();
   for (std::vector<std::string>::const_iterator val = vals.begin(); val != vals.end(); ++val)
   {
      if (val != vals.begin())
      {
         typeDesc += ", \"" + *val + "\"";
      }
      else
      {
         typeDesc += "\"" + *val + "\"";
      }
   }
   typeDesc += ". Default is " + StringUtilities::toDisplayString<DisplayType>(BOUNDARY) + ".";
   VERIFY(pInArgList->addArg<std::string>("Display Type",
      StringUtilities::toDisplayString<DisplayType>(BOUNDARY), typeDesc));
   VERIFY(pInArgList->addArg<std::string>("Result Name", reinterpret_cast<std::string*>(NULL),
      "Name of the result layer."));
   return true;
}

bool QtCluster::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pOutArgList->addArg<DataElement>("Result Element", "Data element resulting from the clustering operation."));
   if (!isBatch())
   {
      VERIFY(pOutArgList->addArg<Layer>("Result Layer", "Layer where the result of the clustering operation is placed."));
   }
   return true;
}

bool QtCluster::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Clustering data", "app", "{9E15CC5E-C286-4d23-8E14-644958AAC2EC}");

   AoiElement* pAoiElement = NULL;
   DataElement* pParentElement = NULL;
   SpatialDataView* pView = NULL;
   BitMaskIterator* pOrigMaskIt = NULL;
   if (isBatch())
   {
      pAoiElement = pInArgList->getPlugInArgValue<AoiElement>(Executable::DataElementArg());
   }
   else
   {
      pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
      if (pView == NULL)
      {
         progress.report("Invalid view", 0, ERRORS, true);
         return false;
      }
      pParentElement = pView->getLayerList()->getPrimaryRasterElement();
      AoiLayer* pAoi = pInArgList->getPlugInArgValue<AoiLayer>(Executable::LayerArg());
      if (pAoi == NULL) // try the active layer
      {
         pAoi = dynamic_cast<AoiLayer*>(pView->getActiveLayer());
      }
      if (pAoi == NULL) // get the topmost AOI layer
      {
         pAoi = dynamic_cast<AoiLayer*>(pView->getTopMostLayer(AOI_LAYER));
      }
      if (pAoi != NULL)
      {
         pAoiElement = static_cast<AoiElement*>(pAoi->getDataElement());
      }
   }
   if (pAoiElement == NULL)
   {
      progress.report("Invalid AOI", 0, ERRORS, true);
      return false;
   }
   const BitMask* pOrigMask = pAoiElement->getSelectedPoints();
   VERIFY(pOrigMask);
   RasterElement* pPrimaryRaster = dynamic_cast<RasterElement*>(pParentElement);
   if (pPrimaryRaster != NULL)
   {
      pOrigMaskIt = new BitMaskIterator(pOrigMask, pPrimaryRaster);
      VERIFY(pOrigMaskIt != NULL);
      if (pOrigMaskIt->getCount() == 0)
      {
         progress.report("No points in the AOI.", 0, ERRORS, true);
         return false;
      }
      if (pOrigMaskIt->getCount() > sqrt(static_cast<double>(std::numeric_limits<unsigned int>::max())))
      {
         progress.report("Too many points in the AOI.", 0, ERRORS, true);
         return false;
      }
   }
   else
   {
      if (pOrigMask->getCount() == 0)
      {
         progress.report("No points in the AOI.", 0, ERRORS, true);
         return false;
      }
      if (pOrigMask->getCount() > sqrt(static_cast<double>(std::numeric_limits<unsigned int>::max())))
      {
         progress.report("Too many points in the AOI.", 0, ERRORS, true);
         return false;
      }
   }
   if (!isBatch() && pOrigMask->getCount() > 10000)
   {
      if (Service<DesktopServices>()->showMessageBox("Warning", "The AOI contains a large number of points. "
         "Clustering may take a long time. Would you like to continue?", "Yes", "No") == 1)
      {
         progress.report("Aborted by user", 0, ABORT, true);
         return false;
      }
   }

   std::string resultName;
   pInArgList->getPlugInArgValue("Result Name", resultName);
   if (resultName.empty())
   {
      resultName = "clusters";
   }

   double clusterSize;
   VERIFY(pInArgList->getPlugInArgValue("Cluster Size", clusterSize));
   std::string displayTypeStr;
   VERIFY(pInArgList->getPlugInArgValue("Display Type", displayTypeStr));
   DisplayType displayType = StringUtilities::fromDisplayString<DisplayType>(displayTypeStr);

   if (!isBatch())
   {
      QtClusterGui gui(Service<DesktopServices>()->getMainWidget());
      gui.setClusterSize(clusterSize);
      gui.setResultName(QString::fromStdString(resultName));
      gui.setDisplayType(displayType);
      if (gui.exec() != QDialog::Accepted)
      {
         progress.report("Aborted by user.", 0, ABORT, true);
         return false;
      }
      clusterSize = gui.getClusterSize();
      resultName = gui.getResultName().toStdString();
      displayType = gui.getDisplayType();
   }
   if (!displayType.isValid())
   {
      progress.report("Invalid display type.", 0, ERRORS, true);
      return false;
   }

   /**********
    * Create the output element
    **********/
   if (Service<ModelServices>()->getElement(resultName, std::string(), pParentElement) != NULL)
   {
      if (resultName == pAoiElement->getName() && pParentElement == pAoiElement->getParent())
      {
         progress.report("Input AOI and result element are the same.", 0, ERRORS, true);
         return false;
      }
      if (isBatch())
      {
         progress.report("Result data element already exists.", 0, ERRORS, true);
         return false;
      }
      if (Service<DesktopServices>()->showMessageBox("Replace element",
               "The result data element exists. Would you like to replace it?", "Yes", "No") == 1)
      {
         progress.report("Result data element already exists.", 0, ABORT, true);
         return false;
      }
      Service<ModelServices>()->destroyElement(
         Service<ModelServices>()->getElement(resultName, std::string(), pParentElement));
   }
   ModelResource<RasterElement> pPseudo(reinterpret_cast<RasterElement*>(NULL));
   DataAccessor pPseudoAcc(NULL, NULL);
   ModelResource<AnnotationElement> pAnnotation(reinterpret_cast<AnnotationElement*>(NULL));
   GraphicGroup* pOut = NULL;
   if (displayType == PSEUDO)
   {
      RasterElement* pParentRaster = dynamic_cast<RasterElement*>(pParentElement);
      RasterDataDescriptor* pDesc = pParentRaster == NULL ? NULL :
         static_cast<RasterDataDescriptor*>(pParentRaster->getDataDescriptor());
      if (pDesc != NULL)
      {
         pPseudo = ModelResource<RasterElement>(RasterUtilities::createRasterElement(
            resultName, pDesc->getRowCount(), pDesc->getColumnCount(), INT1UBYTE, true, pParentElement));
      }
      if (pPseudo.get() == NULL)
      {
         progress.report("Unable to create pseudocolor element.", 0, ERRORS, true);
         return false;
      }
      pPseudoAcc = pPseudo->getDataAccessor();
      if (!pPseudoAcc.isValid())
      {
         progress.report("Unable to access pseudocolor layer.", 0, ERRORS, true);
         return false;
      }
      memset(pPseudo->getRawData(), 0, pDesc->getRowCount() * pDesc->getColumnCount());
   }
   else
   {
      pAnnotation = ModelResource<AnnotationElement>(resultName, pParentElement);
      pOut = pAnnotation->getGroup();
      VERIFY(pOut);
   }

   /**********
    * Calculate pairwise distances
    **********/
   PointsType points;
   int bx1, bx2, by1, by2;
   if (pView != NULL)
   {
      pOrigMaskIt->getBoundingBox(bx1, by1, bx2, by2);
   }
   else
   {
      pOrigMask->getMinimalBoundingBox(bx1, by1, bx2, by2);
   }
   int numToIncPercentage = (bx2-bx1+1) / 99;
   progress.report("Scanning AOI", 0, NORMAL);
   for (int x = bx1; x <= bx2; ++x)
   {
      if (numToIncPercentage <= 0 || (x-bx1) % numToIncPercentage == 0)
      {
         if (isAborted())
         {
            progress.report("User aborted", 0, ABORT, true);
            return false;
         }
         progress.report("Scanning AOI", 99 * (x-bx1) / (bx2-bx1+1), NORMAL);
      }
      for (int y = by1; y <= by2; ++y)
      {
         if (pView != NULL)
         {
            if (pOrigMaskIt->getPixel(x, y))
            {
               points.push_back(QPoint(x, y));
            }
         }
         else
         {
            if (pOrigMask->getPixel(x, y))
            {
               points.push_back(QPoint(x, y));
            }
         }
      }
   }
   delete pOrigMaskIt;
   /**********
    * Calculate in range points
    **********/
   MatrixType inRange(points.size(), points.size());
   numToIncPercentage = points.size() / 99;
   if (numToIncPercentage <= 0)
   {
      numToIncPercentage = 1;
   }
   else if (numToIncPercentage > 100)
   {
      numToIncPercentage = 100;
   }
   progress.report("Calculating distance matrix", 0, NORMAL);
   for (int start = 0; start < points.size(); ++start)
   {
      inRange(start, start) = 1;
      if (start % numToIncPercentage == 0)
      {
         if (isAborted())
         {
            progress.report("User aborted", 0, ABORT, true);
            return false;
         }
         progress.report("Calculating distance matrix", 99 * start / points.size(), NORMAL);
      }
      for (int end = start+1; end < points.size(); ++end)
      {
         QPoint a = points[start];
         QPoint b = points[end];
         QPoint c = b - a;
         double distance = sqrt(static_cast<double>(c.x()) * c.x() + c.y() * c.y());
         // if the pairwise distance is larger than the cluster size,
         // these two points will never be in a cluster together so
         // we throw them out
         inRange(start,end) = (distance <= clusterSize) ? 1 : 0;
      }
   }

   /**********
    * iterate until everything is clustered
    **********/
   int total = points.size();
   int pointsChosen = 0;
   int clusterNumber = 1;
   progress.report("Locating clusters", 0, NORMAL);
   while (pointsChosen < total)
   {
      if (isAborted())
      {
         progress.report("User aborted", 0, ABORT, true);
         return false;
      }
      progress.report(QString("Locating clusters. %1 clusters, %2 points remain unclustered.")
         .arg(clusterNumber-1).arg(total - pointsChosen).toStdString(),
         99 * pointsChosen / total, NORMAL);

      int largest = -1;
      int largestCount = 0;
      for (int row = 0; row < points.size(); ++row)
      {
         int tmpCount = sum(ublas::matrix_row<MatrixType>(inRange, row));
         if (tmpCount > largestCount)
         {
            largest = row;
            largestCount = tmpCount;
         }
      }
      if (largestCount == 0)
      {
         break;
      }

      LocationType centroid(0, 0);

      ublas::vector<int> row(inRange.size1());
      for (size_t col = 0; col < inRange.size1(); ++col)
      {
         row(col) = inRange(largest, col);
      }
      for (size_t col = 0; col < row.size(); ++col)
      {
         if (col % 100 == 0)
         {
            QApplication::processEvents();
         }
         if (row(col) != 0)
         {
            ++pointsChosen;
            centroid.mX += points[col].x();
            centroid.mY += points[col].y();

            for (size_t idx = 0; idx < inRange.size1(); ++idx)
            {
               inRange(col, idx) = 0;
            }

            if (displayType == PSEUDO)
            {
               pPseudoAcc->toPixel(points[col].y(), points[col].x());
               if (!pPseudoAcc.isValid())
               {
                  progress.report("Unable to access pseudocolor layer.", 0, ERRORS, true);
                  return false;
               }
               *reinterpret_cast<unsigned char*>(pPseudoAcc->getColumn()) = clusterNumber;
            }
         }
      }
      centroid.mX /= largestCount;
      centroid.mY /= largestCount;
      for (size_t idx = 0; idx < inRange.size1(); ++idx)
      {
         inRange(largest, idx) = 0;
      }

      // adjust the centroid to the center of a pixel
      centroid.mX += 0.5;
      centroid.mY += 0.5;
      if (displayType == CENTROID || displayType == CENTROID_AND_BOUNDARY)
      {
         std::vector<LocationType> vertices;
         vertices.push_back(centroid);
         GraphicObject* pCentroid = pOut->addObject(MULTIPOINT_OBJECT);
         VERIFY(pCentroid);
         pCentroid->addVertices(vertices);
         pCentroid->setName("Centroid " + StringUtilities::toDisplayString(clusterNumber));
      }
      if (displayType == BOUNDARY || displayType == CENTROID_AND_BOUNDARY)
      {
         GraphicObject* pCluster = pOut->addObject(ELLIPSE_OBJECT);
         VERIFY(pCluster);
         pCluster->setBoundingBox(centroid - LocationType(clusterSize, clusterSize),
                                  centroid + LocationType(clusterSize, clusterSize));
         pCluster->setFillState(false);
         pCluster->setName("Cluster " + StringUtilities::toDisplayString(clusterNumber));
      }
      ++clusterNumber;
   }
   if (pView != NULL)
   {
      if (displayType == PSEUDO)
      {
         PseudocolorLayer* pOutLayer = static_cast<PseudocolorLayer*>(
            pView->createLayer(PSEUDOCOLOR, pPseudo.get()));
         if (pOutLayer != NULL)
         {
            std::vector<ColorType> colors;
            std::vector<ColorType> excluded;
            excluded.push_back(ColorType(0, 0, 0));
            excluded.push_back(ColorType(255, 255, 255));
            ColorType::getUniqueColors(clusterNumber - 1, colors, excluded);
            for (int cl = 1; cl < clusterNumber; ++cl)
            {
               pOutLayer->addInitializedClass(QString("Cluster %1").arg(cl).toStdString(), cl, colors[cl-1]);
            }
            if (pOutArgList != NULL)
            {
               pOutArgList->setPlugInArgValue("Result Layer", pOutLayer);
            }
         }
      }
      else
      {
         AnnotationLayer* pOutLayer = static_cast<AnnotationLayer*>(
            pView->createLayer(ANNOTATION, pAnnotation.get()));
         if (pOutArgList != NULL)
         {
            pOutArgList->setPlugInArgValue("Result Layer", pOutLayer);
         }
      }
   }
   if (pOutArgList != NULL)
   {
      if (displayType == PSEUDO)
      {
         pOutArgList->setPlugInArgValue("Result Element", pPseudo.get());
      }
      else
      {
         pOutArgList->setPlugInArgValue("Result Element", pAnnotation.get());
      }
   }
   pPseudo.release();
   pAnnotation.release();

   progress.report("Clustering complete", 100, NORMAL);
   progress.upALevel();
   return true;
}
