/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ArcProxy.h"
#include "ConnectionParameters.h"
#include "FeatureClassProperties.h"
#include "FormatStringProcessor.h"
#include "LicenseUtilities.h"
#include "ProcessStatusChecker.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <boost/lexical_cast.hpp>
#include <string>

#ifndef ESRI_WINDOWS 
class _com_error
{
};
#endif

namespace
{
   BSTR to_BSTR(const QString &str)
   {
      CComBSTR com_bstr(str.size());
      for(int i = 0; i < str.size(); i++)
      {
         com_bstr[i] = str[i].unicode();
      }
      return com_bstr.Detach();
   }

   std::string toString(const CComBSTR &str)
   {
//#pragma message("Write this method for Solaris")
#if defined(ESRI_WINDOWS)
      _bstr_t moreBstr(str, true);
      return LPCSTR(moreBstr);
#else
      return "";
#endif
   }

   std::string toString(const VARIANT &variant)
   {
      CComVariant strVariant;
      if (FAILED(strVariant.ChangeType(VT_BSTR, &variant)))
      {
         return "[Error: Not stringable]";
      }
      return toString(strVariant.bstrVal);
   }

   class ArcFormatStringPreprocessor : public ArcProxyLib::FormatStringPreprocessor
   {
   public:
      ArcFormatStringPreprocessor(IFeatureClassPtr pFeatureClass, std::string &preprocessedString) : 
         FormatStringPreprocessor(preprocessedString), mpFeatureClass(pFeatureClass)
      {
      }

      int getFieldIndex(const std::string &fieldName) const
      {
         long index;
         mpFeatureClass->FindField(to_BSTR(QString::fromStdString(fieldName)), &index);
         return index;
      }

   private:
      IFeatureClassPtr mpFeatureClass;

   };

   class ArcFormatStringProcessor : public ArcProxyLib::FormatStringProcessor
   {
   public:
      ArcFormatStringProcessor(IFeaturePtr pFeature) : mpFeature(pFeature)
      {
      }

      std::string getFieldValue(int fieldNumber) const
      {
         VARIANT variant;
         if (!FAILED(mpFeature->get_Value(fieldNumber-1, &variant)))
         {
            return toString(variant);
         }
         else
         {
            return "[Error: Field not found]";
         }
     }

   private:
      IFeaturePtr mpFeature;
   };

}

ArcProxy::ArcProxy(QObject *pParent) :
   QObject(pParent),
   mArcLicense(false),
   mState(INITIALIZE)
{
   QTimer::singleShot(0, this, SLOT(connectToServer()));

   int idx = QCoreApplication::arguments().indexOf("-pid");
   if (idx != -1)
   {
      long pid = QCoreApplication::arguments()[idx+1].toLong();
      ProcessStatusChecker *pChecker = new ProcessStatusChecker(this, pid, 30000);
      connect(pChecker, SIGNAL(processExited()), this, SLOT(serverExited()));
      
   }
}

ArcProxy::~ArcProxy()
{
   if (mArcLicense)
   {
      ShutdownApp();
   }
}

void ArcProxy::connectToServer()
{
   int idx = QCoreApplication::arguments().indexOf("-s");
   if(idx == -1)
   {
      QCoreApplication::exit(1);
   }
   int port = QCoreApplication::arguments()[idx+1].toInt();
   QTcpSocket *pSocket = new QTcpSocket(this);
   pSocket->connectToHost(QHostAddress(QHostAddress::LocalHost), port);
   if(!pSocket->waitForConnected())
   {
      QCoreApplication::exit(1);
   }
   connect(pSocket, SIGNAL(readyRead()), this, SLOT(processData()));
   mStream.setDevice(pSocket);
   mStream.setRealNumberPrecision(16);
}

void ArcProxy::initialize(const QString &data)
{
   if(data == APP_VERSION_NUMBER)
   {
      mArcLicense = InitializeApp();
      ISpatialReferenceFactory2Ptr pSpatialFactory(CLSID_SpatialReferenceEnvironment);
      IGeographicCoordinateSystemPtr pSystem;
      pSpatialFactory->CreateGeographicCoordinateSystem(esriSRGeoCS_WGS1984, &pSystem);
      mpDestinationReference = pSystem;
      mStream << APP_VERSION_NUMBER << endl;
   }
   else
   {
      mStream << "ERROR Invalid version number. Expected " << APP_VERSION_NUMBER << endl;
      QCoreApplication::instance()->quit();
   }
}

void ArcProxy::handleRequest(const QString &request, QStringList &args)
{
   if(request == "END")
   {
      QCoreApplication::quit();
   }
   else if(request == "OPEN")
   {
      // parse connection parameters
      std::string errorMessage;
      ArcProxyLib::ConnectionParameters connection;
      connection.fromString(args.join(" ").toStdString());
      ComWrap<IFeatureClassPtr> pFeatureClass(getFeatureClass(connection, errorMessage));
      if(!pFeatureClass)
      {
         mStream << "ERROR " << QString::fromStdString(errorMessage) << endl;
      }
      else
      {
         QUuid uid = QUuid::createUuid();
         if(uid.isNull())
         {
            mStream << "ERROR Unable to generate unique ID" << endl;
         }
         else
         {
            mFeatureClasses[uid] = pFeatureClass;
            mStream << "SUCCESS " << uid << endl;
         }
      }
   }
   else if(request == "CLOSE")
   {
      QUuid uid(args.front());
      if(uid.isNull() || !mFeatureClasses.contains(uid))
      {
         mStream << "ERROR ID unknown" << endl;
      }
      else
      {
         mFeatureClasses.remove(uid);
         mStream << "SUCCESS " << uid << endl;
      }
   }
   else if(request == "GETFEATURECLASSPROPERTIES")
   {
      QUuid uid(args.front());
      if(uid.isNull() || !mFeatureClasses.contains(uid))
      {
         mStream << "ERROR ID unknown" << endl;
         return;
      }

      ArcProxyLib::FeatureClassProperties properties;

      IFeatureClassPtr pFeatures = mFeatureClasses[uid];
      long numFeatures=0;
      pFeatures->FeatureCount(IQueryFilterPtr(), &numFeatures);
      
      properties.setFeatureCount(numFeatures);

      esriGeometryType shapeType;
      pFeatures->get_ShapeType(&shapeType);
      ArcProxyLib::FeatureType featureType = ArcProxyLib::UNKNOWN;
      switch (shapeType)
      {
      case esriGeometryPoint:
         featureType = ArcProxyLib::POINT;
         break;
      case esriGeometryMultipoint:
         featureType = ArcProxyLib::MULTIPOINT;
         break;
      case esriGeometryPolyline:
         featureType = ArcProxyLib::POLYLINE;
         break;
      case esriGeometryPolygon:
         featureType = ArcProxyLib::POLYGON;
         break;
      default:
         break;
      }
      
      properties.setFeatureType(featureType);

      std::vector<std::string> fields;
      std::vector<std::string> types;
      std::vector<std::string> sampleValues;

      bool hasSampleValues = false;
      IFeaturePtr pFeature;

      IFeatureCursorPtr pFeatureCursor;
      pFeatures->Search(NULL, true, &pFeatureCursor);
      if (pFeatureCursor != NULL)
      {
         pFeatureCursor->NextFeature(&pFeature);
         if (pFeature != NULL)
         {
            hasSampleValues = true;
         }
      }

      ArcFormatStringProcessor formatProcessor(pFeature);

      IFieldsPtr pFields;
      pFeatures->get_Fields(&pFields);
      if (pFields != NULL)
      {
         long numFields;
         pFields->get_FieldCount(&numFields);

         for (long fieldIndex = 0; fieldIndex < numFields; ++fieldIndex)
         {
            IFieldPtr pField;
            pFields->get_Field(fieldIndex, &pField);
            if (pField != NULL)
            {
               esriFieldType type;
               std::string typeString;
               pField->get_Type(&type);

               switch (type)
               {
               case esriFieldTypeSmallInteger:
                  typeString = "Small Integer";
                  break;
               case esriFieldTypeInteger:
                  typeString = "Integer";
                  break;
               case esriFieldTypeSingle:
                  typeString = "Single";
                  break;
               case esriFieldTypeDouble:
                  typeString = "Double";
                  break;
               case esriFieldTypeString:
                  typeString = "String";
                  break;
               case esriFieldTypeDate:
                  typeString = "Date/Time";
                  break;
               default:
                  // skipping ObjectID, Geometry, Blob, Raster, GUID, and GlobalID
                  // since they can't be usefully manipulated by the user
                  continue;
               }

               types.push_back(typeString);

               CComBSTR nameBstr(10000); // I hope this is big enough
               pField->get_Name(&nameBstr);
               fields.push_back(toString(nameBstr));;

               if (hasSampleValues)
               {
                  sampleValues.push_back(formatProcessor.getFieldValue(fieldIndex+1)); // format processor expects 1-based indicies
               }
            }
         }
      }

      properties.setFields(fields);
      properties.setTypes(types);
      properties.setSampleValues(sampleValues);

      mStream << "SUCCESS " << QString::fromStdString(properties.toString()) << endl;
      return;
   }
   else if(request == "QUERY")
   {
      QUuid uid(args.front());
      if(uid.isNull() || !mFeatureClasses.contains(uid))
      {
         mStream << "ERROR ID unknown" << endl;
         return;
      }
      IFeatureClassPtr pFeatures = mFeatureClasses[uid];
      if(pFeatures == 0)
      {
         mStream << "ERROR Invalid feature class" << endl;
         return;
      }
      args.pop_front();
      queryFeatureClass(pFeatures, args);
   }
   else
   {
      mStream << "ERROR Invalid request: " << request << " is an unknown command." << endl;
      return;
   }
}

void ArcProxy::processData()
{
   QString data = mPartialRequest + mStream.readAll();
   QStringList lines = data.split("\n", QString::SkipEmptyParts);
   if (data[data.size()-1] != '\n')
   {
      mPartialRequest = lines.takeLast();
   }
   else
   {
      mPartialRequest.clear();
   }

   while (!lines.empty())
   {
      QString line = lines.takeFirst();

      if(mState == INITIALIZE)
      {
         initialize(line);
         mState = REQUEST;
      }
      else if(mState == REQUEST)
      {
         QStringList args = line.split(" ", QString::SkipEmptyParts);
         if(!args.empty())
         {
            QString request = args.front();
            args.pop_front();
            handleRequest(request, args);
         }
      }
      else
      {
         mStream << "ERROR invalid request" << endl;
      }
   }
}

void ArcProxy::queryFeatureClass(IFeatureClassPtr pFeatures, QStringList &args)
{
   // generate the query
   IQueryFilterPtr pQueryFilter;
   std::string labelFormat;
   while(!args.empty())
   {
      QString queryType = args.takeFirst();
      if(queryType == "CLIP")
      {
         // create a clip envelope
         double xMin = args.takeFirst().toDouble();
         double yMin = args.takeFirst().toDouble();
         double xMax = args.takeFirst().toDouble();
         double yMax = args.takeFirst().toDouble();
         IEnvelopePtr pEnv(CLSID_Envelope);  
         pEnv->PutCoords(xMin, yMin, xMax, yMax);

         ISpatialFilterPtr pFilter(CLSID_SpatialFilter);
         IEnvelopePtr pNewEnv;
         pEnv->get_Envelope(&pNewEnv);
         pFilter->putref_Geometry(IGeometryPtr(pNewEnv));

         CComBSTR bstrName;
         pFeatures->get_ShapeFieldName(&bstrName);
         pFilter->put_GeometryField(bstrName);
         pFilter->put_SpatialRel(esriSpatialRelIntersects);
         pQueryFilter = pFilter;
      }
      else if (queryType == "WHERE")
      {
         if (pQueryFilter == 0)
         {
            IQueryFilterPtr pFilter(CLSID_QueryFilter);
            pQueryFilter = pFilter;
         }

         // create a where-clause
         QString whereClause = convertWhereClause(pFeatures, QUrl::fromPercentEncoding(args.takeFirst().toAscii()));
         pQueryFilter->put_WhereClause(to_BSTR(whereClause));
         
      }
      else if (queryType == "LABELFORMAT")
      {
         std::string labelFormatRaw = QUrl::fromPercentEncoding(args.takeFirst().toAscii()).toStdString();
         ArcFormatStringPreprocessor preprocessor(pFeatures, labelFormat);
         std::for_each(labelFormatRaw.begin(), labelFormatRaw.end(), preprocessor);
      }
   }

   try
   {
      // loop the query and return the data
      IFeatureCursorPtr pCursor;
      pFeatures->Search(pQueryFilter, VARIANT_TRUE, &pCursor);

      IFeaturePtr pFeature;
      while(pCursor->NextFeature(&pFeature) == S_OK)
      {
         if(!writeFeature(pFeature, labelFormat))
         {
            break;
         }
      }
      mStream << "END" << endl;
   }
   catch (_com_error e)
   {
      mStream << "ERROR Invalid query" << endl;
   }
}

const IFeatureClassPtr ArcProxy::getFeatureClass(const ArcProxyLib::ConnectionParameters &params, std::string &errorMessage)
{
   IFeatureClassPtr pFeatureClass(NULL);

   ArcProxyLib::ConnectionType connectionType = params.getConnectionType();

   if (connectionType == ArcProxyLib::SHP_CONNECTION)
   {
      IWorkspaceFactoryPtr pFactory(CLSID_ShapefileWorkspaceFactory);

      IWorkspacePtr pWorkspace;
      BSTR pWidePath = to_BSTR(QString::fromStdString(params.getDatabase()));
      HRESULT hr = pFactory->OpenFromFile(pWidePath, NULL, &pWorkspace);
      if (FAILED(hr))
      {
         errorMessage = "Could not open file (" + boost::lexical_cast<std::string>(hr) + ")";
         return IFeatureClassPtr();
      }
      IFeatureWorkspacePtr pFeatureWorkspace(pWorkspace);
      BSTR pWideShapefile = to_BSTR(QString::fromStdString(params.getFeatureClass()));
      hr = pFeatureWorkspace->OpenFeatureClass(pWideShapefile, &pFeatureClass);
      if (FAILED(hr))
      {
         errorMessage = "Could not open feature class (" + boost::lexical_cast<std::string>(hr) + ")";
         return IFeatureClassPtr();
      }

      ISQLSyntaxPtr pSqlSyntax(pWorkspace);
      if (pSqlSyntax != NULL)
      {
         CComBSTR special(10);
         pSqlSyntax->GetSpecialCharacter(esriSQL_DelimitedIdentifierPrefix, &special);
         std::string leftDelim = toString(special);
      }
   }
   else if (connectionType == ArcProxyLib::SDE_CONNECTION)
   {
      IWorkspaceFactoryPtr pFactory(CLSID_SdeWorkspaceFactory);

      // Some of these values must have some string or else Arc freaks out and presents a GUI instead of simply failing in Open()
      IPropertySetPtr pPropertySet(CLSID_PropertySet);
      pPropertySet->SetProperty(CComBSTR(L"SERVER"), CComVariant(CComBSTR(params.getServer().empty() ? "invalid" : params.getServer().c_str())));
      pPropertySet->SetProperty(CComBSTR(L"INSTANCE"), CComVariant(CComBSTR(params.getInstance().empty() ? "1" : params.getInstance().c_str())));
      pPropertySet->SetProperty(CComBSTR(L"DATABASE"), CComVariant(CComBSTR(params.getDatabase().empty() ? "invalid" : params.getDatabase().c_str())));
      pPropertySet->SetProperty(CComBSTR(L"USER"), CComVariant(CComBSTR(params.getUser().empty() ? "invalid" : params.getUser().c_str())));
      pPropertySet->SetProperty(CComBSTR(L"PASSWORD"), CComVariant(CComBSTR(params.getPassword().empty() ? "invalid" : params.getPassword().c_str())));
      pPropertySet->SetProperty(CComBSTR(L"VERSION"), CComVariant(CComBSTR(params.getVersion().c_str())));
      
      IWorkspacePtr pWorkspace;
      HRESULT hr = pFactory->Open(pPropertySet, 0, &pWorkspace);
      if (FAILED(hr))
      {
         errorMessage = "Could not open database (" + boost::lexical_cast<std::string>(hr) + ")";
         return IFeatureClassPtr();
      }
      IFeatureWorkspacePtr pFeatureWorkspace(pWorkspace);
      BSTR pFeatureClassName = to_BSTR(QString::fromStdString(params.getFeatureClass()));

      hr = pFeatureWorkspace->OpenFeatureClass(pFeatureClassName, &pFeatureClass);
      if (FAILED(hr))
      {
         errorMessage = "Could not open feature class (" + boost::lexical_cast<std::string>(hr) + ")";
         return IFeatureClassPtr();
      }
   }
   else
   {
      errorMessage = "Unknown connection type";
   }

   return pFeatureClass;
}


bool ArcProxy::writeFeature(IFeaturePtr pFeature, const std::string &labelFormat)
{
   IGeometryPtr pShape;
   pFeature->get_Shape(&pShape);

   esriGeometryType shapeType;
   pShape->get_GeometryType(&shapeType);
   switch(shapeType)
   {
   case esriGeometryPoint:
      mStream << "POINT ";
      break;
   case esriGeometryMultipoint:
      mStream << "MULTIPOINT ";
      break;
   case esriGeometryPolyline:
      mStream << "POLYLINE ";
      break;
   case esriGeometryPolygon:
      mStream << "POLYGON ";
      break;
   case esriGeometryMultiPatch:
      mStream << "ERROR Multi patch shapes are not supported" << endl;
      return false;
   case esriGeometryTriangles:     // fall-through
   case esriGeometrySphere:        // fall-through
   case esriGeometryRay:           // fall-through
   case esriGeometryTriangleFan:   // fall-through
   case esriGeometryTriangleStrip: // fall-through
   case esriGeometryBag:           // fall-through
   case esriGeometryAny:           // fall-through
   case esriGeometryEnvelope:      // fall-through
   case esriGeometryRing:          // fall-through
   case esriGeometryPath:          // fall-through
   case esriGeometryBezier3Curve:  // fall-through
   case esriGeometryEllipticArc:   // fall-through
   case esriGeometryCircularArc:   // fall-through
   case esriGeometryLine:          // fall-through
   case esriGeometryNull:          // fall-through
      mStream << "ERROR Unsupported type encountered" << endl;
      return false;
   }

   // generate label
   std::string processedLabel = std::for_each(labelFormat.begin(), labelFormat.end(), 
      ArcFormatStringProcessor(pFeature)).getProcessedString();
   mStream << "LABEL=" << QUrl::toPercentEncoding(QString::fromStdString(processedLabel)).constData() << " ";

   IGeometryCollectionPtr pCollection(pShape);
   IPointCollectionPtr pPoints(pShape);
   if((pCollection == 0) && (pPoints == 0))
   {
      // Single point
      IPointPtr pPoint(pShape);
      pPoint->Project(mpDestinationReference);
      
      // Arc stores these in a different order so we switch them here
      double x, y;
      pPoint->get_X(&y);
      pPoint->get_Y(&x);
      mStream << x << "," << y << endl;
   }
   else if((shapeType == esriGeometryMultipoint) ||
      ((pCollection == 0) && (pPoints != 0)))
   {
      // Multi-point or polygon/polyline with a single segment
      // loop through all the vertices
      long numVertices;
      pPoints->get_PointCount(&numVertices);
      for(long vertex = 0; vertex < numVertices; vertex++)
      {
         // correct the geo reference frame and add the point to the vertex list
         IPointPtr pOutputPoint;
         pPoints->get_Point(vertex, &pOutputPoint);
         pOutputPoint->Project(mpDestinationReference);

         // Arc stores these in a different order so we switch them here
         double x, y;
         pOutputPoint->get_X(&y);
         pOutputPoint->get_Y(&x);
         mStream << x << "," << y << " ";
      }
      mStream << endl;
   }
   else
   {
      // Polygon/polyline with multiple segments or rings
      long numPaths = 0;
      pCollection->get_GeometryCount(&numPaths);

      // loop through all the paths/rings
      for(long path = 0; path < numPaths; path++)
      {
         // Get the path/ring...any other geometry type is an error
         IGeometryPtr pGeometry;
         pCollection->get_Geometry(path, &pGeometry);
         esriGeometryType type;
         pGeometry->get_GeometryType(&type);
         if(type == esriGeometryPath || type == esriGeometryRing)
         {
            // if we are past the first path, mark the start of the new path
            if(path > 0)
            {
               mStream << "PATH ";
            }

            // loop through all the vertices in this path
            IPointCollectionPtr pPoints(pGeometry);
            long numVertices;
            pPoints->get_PointCount(&numVertices);
            for(int vertex = 0; vertex < numVertices; vertex++)
            {
               // correct the geo reference frame and add the point to the vertex list
               IPointPtr pOutputPoint;
               pPoints->get_Point(vertex, &pOutputPoint);
               pOutputPoint->Project(mpDestinationReference);
               
               // Arc stores these in a different order so we switch them here
               double x, y;
               pOutputPoint->get_X(&y);
               pOutputPoint->get_Y(&x);
               mStream << x << "," << y << " ";
            }
         }
      }
      mStream << endl;
   }
   return true;
}

void ArcProxy::serverExited()
{
   QCoreApplication::quit();
}

QString ArcProxy::convertWhereClause(IFeatureClassPtr pFeatures, const QString &original)
{
   if (pFeatures == NULL)
   {
      return QString();
   }

   QString converted;
   QStringList originalList = original.split(",");

   QString combineStr = " " + originalList.takeFirst() + " ";

   if (originalList.count() % 3 != 0)
   {
      return QString();
   }

   IDatasetPtr pDataset(pFeatures);;
   if (pDataset == NULL)
   {
      return QString();
   }

   IWorkspacePtr pWorkspace;
   pDataset->get_Workspace(&pWorkspace);
   if (pWorkspace == NULL)
   {
      return QString();
   }

   ISQLSyntaxPtr pSqlRequest(pWorkspace);
   if (pSqlRequest == NULL)
   {
      return QString();
   }

   CComBSTR specialChar(10);
   pSqlRequest->GetSpecialCharacter(esriSQL_DelimitedIdentifierPrefix, &specialChar);
   QString startDelim = QString::fromStdString(toString(specialChar));
   pSqlRequest->GetSpecialCharacter(esriSQL_DelimitedIdentifierSuffix, &specialChar);
   QString stopDelim = QString::fromStdString(toString(specialChar));

   IFieldsPtr pFields;
   pFeatures->get_Fields(&pFields);
   if (pFields == NULL)
   {
      return QString();
   }

   QStringList::const_iterator iter = originalList.begin();
   while (iter != originalList.end())
   {
      if (iter != originalList.begin())
      {
         converted += combineStr;
      }

      QString fieldName = *iter++;
      QString operand = *iter++;
      QString parameter = *iter++;

      parameter = parameter.replace("%~", ",");

      long fieldIndex = 0;
      pFields->FindField(to_BSTR(fieldName), &fieldIndex);
      
      IFieldPtr pField;
      pFields->get_Field(fieldIndex, &pField);
      if (pField == NULL)
      {
         continue;
      }

      esriFieldType type;
      pField->get_Type(&type);

      converted += startDelim + fieldName + stopDelim;
      converted += operand;

      //do not force quotes around Date inputs that want to change the format
      if (type == esriFieldTypeString || (type == esriFieldTypeDate && !parameter.contains(QString("TO_DATE"), Qt::CaseInsensitive)) )
      {
         converted += "'" + parameter.replace('\'', "''") + "'"; 
      }
      else
      {
         converted += parameter;
      }
   }

   return converted;
}


int main(int argc, char *argv[])
{
   try
   {
      QCoreApplication app(argc, argv);
      ArcProxy proxy;
      return app.exec();
   }
   catch (_com_error e)
   {
      // Problem with COM.  Arc probably isn't installed
   }

   return 1;
}
