/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ColladaStreamWriter.h"
#include "ColladaUtil.h"
#include "GraphicGroup.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "RectangleObject.h"
#include "StringUtilities.h"

#include "COLLADABUPlatform.h"
#include "COLLADASWAsset.h"
#include "COLLADASWConstants.h"
#include "COLLADASWInputList.h"
#include "COLLADASWInstanceGeometry.h"
#include "COLLADASWNode.h"
#include "COLLADASWSource.h"
#include "COLLADASaxFWLLoader.h"
#include "COLLADASWVertices.h"
#include "COLLADASWPrimitves.h"

using namespace std;

ColladaStreamWriter::ColladaStreamWriter()
{}

ColladaStreamWriter::ColladaStreamWriter(std::string filename) : mFilename(filename)
{}

ColladaStreamWriter::~ColladaStreamWriter()
{}

void ColladaStreamWriter::writeGraphicObjects(std::list<GraphicObject*> objects)
{
   COLLADABU::NativeString nativeFilename = COLLADABU::NativeString(mFilename);
   COLLADASW::StreamWriter streamWriter(nativeFilename);

   // Open document
   beginDocument(&streamWriter);

   ColladaGeometryLibrary geomLib(&streamWriter);
   // Iterate over all layer objects and have the geometry library write them to the file
   for (list<GraphicObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
   {
      vector<LocationType> vertices(0);
      switch ((*it)->getGraphicObjectType())
      {

      case RECTANGLE_OBJECT:
         // Get the extents 
         vertices.push_back(LocationType((*it)->getLlCorner().mX, (*it)->getUrCorner().mY));
         vertices.push_back(LocationType((*it)->getUrCorner().mX, (*it)->getUrCorner().mY));
         vertices.push_back(LocationType((*it)->getUrCorner().mX, (*it)->getLlCorner().mY));
         vertices.push_back(LocationType((*it)->getLlCorner().mX, (*it)->getLlCorner().mY));
         break;
      case TRIANGLE_OBJECT:
         // Triangle vertex retrieval here is based on how TriangleObjects are drawn in TriangleObjectImp::draw()
         vertices.push_back(LocationType((*it)->getLlCorner().mX, (*it)->getLlCorner().mY));
         vertices.push_back(LocationType((*it)->getUrCorner().mX, (*it)->getLlCorner().mY));
         vertices.push_back(LocationType((*it)->getLlCorner().mX + (*it)->getApex() * ((*it)->getUrCorner().mX -
            (*it)->getLlCorner().mX), (*it)->getUrCorner().mY));
         break;
      case POLYGON_OBJECT:
         vertices = dynamic_cast<PolygonObject*>(*it)->getVertices();
         break;
      case POLYLINE_OBJECT:         // fall through
      case LINE_OBJECT:             // fall through
      case TEXT_OBJECT:             // fall through
      case FRAME_LABEL_OBJECT:      // fall through
      case ARROW_OBJECT:            // fall through
      case ELLIPSE_OBJECT:          // fall through
      case ROUNDEDRECTANGLE_OBJECT: // fall through
      case ARC_OBJECT:              // fall through
      case MOVE_OBJECT:             // fall through
      case ROTATE_OBJECT:           // fall through
      case SCALEBAR_OBJECT:         // fall through
      case GROUP_OBJECT:            // fall through
      case CGM_OBJECT:              // fall through
      case RAW_IMAGE_OBJECT:        // fall through
      case FILE_IMAGE_OBJECT:       // fall through
      case WIDGET_IMAGE_OBJECT:     // fall through
      case LATLONINSERT_OBJECT:     // fall through
      case NORTHARROW_OBJECT:       // fall through
      case EASTARROW_OBJECT:        // fall through
      case VIEW_OBJECT:             // fall through
      case MULTIPOINT_OBJECT:       // fall through
      case MEASUREMENT_OBJECT:      // fall through
      case BITMASK_OBJECT:          // fall through
      case HLINE_OBJECT:            // fall through
      case VLINE_OBJECT:            // fall through
      case ROW_OBJECT:              // fall through
      case COLUMN_OBJECT:           // fall through
      case TRAIL_OBJECT:            // fall through
      default:
         break;
      }

      if (!vertices.empty())
      {
         string geometryId = (*it)->getName();
         vector<Opticks::Location<float, 3>> vertices3f(0);

         // The GraphicObject interface only returns vertices in terms of LocationType, which is strictly 2D.
         // To have a flexible interface, ColladaGeometryLibrary works on Location<float, 3> data, so
         // the polygon's vertices as LocationType objects are converted and passed.
         for (vector<LocationType>::iterator vertIt = vertices.begin(); vertIt != vertices.end(); ++vertIt)
         {
            Opticks::Location<float, 3> vert3f(static_cast<float>((*vertIt).mX), static_cast<float>((*vertIt).mY),
               static_cast<float>((*vertIt).mZ));
            vertices3f.push_back(vert3f);
         }
         geomLib.writeArbitraryPolygon(geometryId, vertices3f);
      }
   }

   ColladaSceneLibrary sceneLib(&streamWriter);
   sceneLib.startScene(getBasicSceneName());
   for (list<GraphicObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
   {
      sceneLib.writeSimpleGeometryNode((*it)->getName(), (*it)->getRotation());
   }
   endDocument(&streamWriter);
}

void ColladaStreamWriter::beginDocument(COLLADASW::StreamWriter* pStreamWriter)
{
   pStreamWriter->startDocument();
   COLLADASW::Asset asset(pStreamWriter);

   asset.setUpAxisType(COLLADASW::Asset::Y_UP);
   asset.getContributor().mAuthor = "Opticks User";
   asset.getContributor().mAuthoringTool = string(APP_NAME) + " " + string(APP_VERSION_NUMBER);
   asset.add();
}

void ColladaStreamWriter::endDocument(COLLADASW::StreamWriter* pStreamWriter)
{
   if (pStreamWriter == NULL)
   {
      return;
   }
   pStreamWriter->endDocument();
}

/*
 * ColladaGeometryLibrary methods
 */
ColladaGeometryLibrary::ColladaGeometryLibrary(COLLADASW::StreamWriter* pStreamWriter) :
   LibraryGeometries(pStreamWriter)
{}

ColladaGeometryLibrary::~ColladaGeometryLibrary()
{}

void ColladaGeometryLibrary::writeArbitraryPolygon(std::string geometryId,
   const vector<Opticks::Location<float, 3>>& vertices)
{
   // Begin library
   openLibrary();
   // Begin mesh element
   openMesh(COLLADASW::String(formatGeometryId(geometryId)));

   // Create FloatSource to set all the parameters for a mesh element, including vertex positions,
   // data types for axes, accessor stride, vertex counts
   COLLADASW::FloatSource colladaSource(mSW);
   colladaSource.setId(getIdBySemantics(geometryId, COLLADASW::InputSemantic::POSITION));
   colladaSource.setArrayId(getIdBySemantics(geometryId, COLLADASW::InputSemantic::POSITION) + ARRAY_ID_SUFFIX);
   colladaSource.setAccessorStride(3);
   colladaSource.setAccessorCount(vertices.size());
   colladaSource.getParameterNameList().push_back("X");
   colladaSource.getParameterNameList().push_back("Y");
   colladaSource.getParameterNameList().push_back("Z");

   // Write vertex data to the file
   colladaSource.prepareToAppendValues();
   for (unsigned int i = 0; i < vertices.size(); ++i)
   {
      colladaSource.appendValues(vertices.at(i).mX, vertices.at(i).mY, vertices.at(i).mZ);
   }
   colladaSource.finish();

   COLLADASW::Vertices colladaVertices(mSW);
   colladaVertices.setId(getIdBySemantics(geometryId, COLLADASW::InputSemantic::VERTEX));
   COLLADASW::InputList &colladaVerticesInputList = colladaVertices.getInputList();
   COLLADASW::Input colladaVerticesInput(COLLADASW::InputSemantic::POSITION,
      getUriBySemantics(geometryId, COLLADASW::InputSemantic::POSITION));
   colladaVerticesInputList.push_back(colladaVerticesInput);
   colladaVertices.add();

   // Begin the polylist
   COLLADASW::Polylist colladaPolylist(mSW);

   COLLADASW::InputList &colladaPolylistInputList = colladaPolylist.getInputList();
   COLLADASW::Input colladaPolylistInput(COLLADASW::InputSemantic::VERTEX,
      getUriBySemantics(geometryId, COLLADASW::InputSemantic::VERTEX), 0);
   colladaPolylistInputList.push_back(colladaPolylistInput);

   colladaPolylist.setCount(1);
   vector<unsigned long> vcounts;
   vcounts.push_back(vertices.size());
   colladaPolylist.setVCountList(vcounts);

   // Write the indices that define polygons based on previously written vertices
   // Since we will be writing an arbitrary, n-sided polygon, the indices for the
   // vertices will just be all of the index values of the vector
   colladaPolylist.prepareToAppendValues();
   for (unsigned int i = 0; i < vertices.size(); ++i)
   {
      colladaPolylist.appendValues(i);
   }
   colladaPolylist.finish();

   closeMesh();
   closeGeometry();
   closeLibrary();
}

COLLADASW::URI ColladaGeometryLibrary::getUriBySemantics(std::string geometryId,
   COLLADASW::InputSemantic::Semantics type, std::string other_suffix)
{
   std::string id = getIdBySemantics(geometryId, type, other_suffix);
   return COLLADASW::URI(COLLADABU::Utils::EMPTY_STRING, id);
}

string ColladaGeometryLibrary::getIdBySemantics(std::string geometryId,
   COLLADASW::InputSemantic::Semantics type, std::string other_suffix)
{
   return formatGeometryId(geometryId) + getSuffixBySemantic(type) + other_suffix;
}

/*
 * ColladaSceneLibrary methods
 */
ColladaSceneLibrary::ColladaSceneLibrary(COLLADASW::StreamWriter* pStreamWriter) : LibraryVisualScenes(pStreamWriter)
{}

ColladaSceneLibrary::~ColladaSceneLibrary()
{}

void ColladaSceneLibrary::startScene(string sceneId)
{
   openVisualScene(sceneId, sceneId);
}

void ColladaSceneLibrary::endScene()
{
   closeVisualScene();
}

void ColladaSceneLibrary::writeSimpleGeometryNode(string nodeId, double rotation)
{
   COLLADASW::Node node(mSW);
   node.setNodeId(nodeId);
   node.setType(COLLADASW::Node::NODE);

   node.start();

   node.addTranslate("location", 0.0, 0.0, 0.0);
   node.addRotateZ("rotationZ", rotation);
   node.addRotateY("rotationY", 0.0);
   node.addRotateX("rotationX", 0.0);
   node.addScale("scale", 1.0, 1.0, 1.0);

   COLLADASW::InstanceGeometry instGeom(mSW);
   instGeom.setUrl(COLLADASW::URI(COLLADABU::Utils::EMPTY_STRING, formatGeometryId(nodeId)));
   instGeom.add();

   node.end();
}
