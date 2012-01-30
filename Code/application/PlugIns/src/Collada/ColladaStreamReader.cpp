/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined(OPENCOLLADA_SUPPORT)

#include <COLLADAFWGeometry.h>
#include <COLLADAFWPointerArray.h>
#include <COLLADAFWPolygons.h>
#include <COLLADAFWRoot.h>
#include <COLLADAFWRotate.h>
#include <COLLADAFWTransformation.h>
#include <COLLADAFWVisualScene.h>

#include <COLLADASaxFWLLoader.h>

#include "ColladaStreamReader.h"
#include "ColladaUtil.h"

using namespace std;

ColladaStreamReader::ColladaStreamReader() :
   mFirstPass(true),
   mCurrentElement(string()),
   mFilename(string())
{}

ColladaStreamReader::ColladaStreamReader(std::string filename) :
   mFirstPass(true),
   mCurrentElement(string()),
   mFilename(filename)
{}

ColladaStreamReader::~ColladaStreamReader()
{}

void ColladaStreamReader::populateGeometryIds(vector<string>& geometryIds)
{
   mFirstPass = true;
   mCurrentElement.clear();
   COLLADASaxFWL::Loader loader;
   COLLADAFW::Root root(&loader, this);

   if (root.loadDocument(mFilename))
   {
      getGeometryIds(geometryIds);
   }
}

bool ColladaStreamReader::read(string element)
{
   mFirstPass = false;
   mCurrentElement = element;
   COLLADASaxFWL::Loader loader;
   COLLADAFW::Root root(&loader, this);

   mVertices.clear();
   mPolygons.clear();

   if (!root.loadDocument(mFilename))
   {
      return false;
   }
   return true;
}

bool ColladaStreamReader::writeGeometry(const COLLADAFW::Geometry* pGeometry)
{
   if (pGeometry == NULL)
   {
      return false;
   }

   if (mFirstPass)
   {
      string geomId = pGeometry->getName();
      mGeometryIds.push_back(geomId);
      return true;
   }

   if (mCurrentElement == pGeometry->getName())
   {
      if (pGeometry->getType() == COLLADAFW::Geometry::GEO_TYPE_MESH)
      {
         const COLLADAFW::Mesh* pMesh = static_cast<const COLLADAFW::Mesh*>(pGeometry);
         if (!readVertices(pMesh))
         {
            return false;
         }
         if (!readPolygons(pMesh))
         {
            return false;
         }
      }
   }
   return true;
}

// Implementation of start() is required to fully implement IWriter interface.
// Since we currently have nothing to perform here, the body is left empty.
void ColladaStreamReader::start()
{}

void ColladaStreamReader::finish()
{
   if (!mFirstPass)
   {
      for (std::vector<const COLLADAFW::VisualScene>::iterator it = mScenes.begin(); it != mScenes.end(); ++it)
      {
         const COLLADAFW::NodePointerArray& rootNodes = (*it).getRootNodes();
         for (unsigned int i = 0; i < rootNodes.getCount(); ++i)
         {
            readSceneNodesRecursive(rootNodes[i]);
         }
      }
   }
}

void ColladaStreamReader::getPolygons(vector<vector<unsigned int>>& polygons)
{
   polygons.clear();
   for (vector<vector<unsigned int>>::iterator outerIt = mPolygons.begin(); outerIt != mPolygons.end(); ++outerIt)
   {
      vector<unsigned int> values;
      for (vector<unsigned int>::iterator innerIt = (*outerIt).begin(); innerIt != (*outerIt).end(); ++innerIt)
      {
         values.push_back(*innerIt);
      }
      polygons.push_back(values);
   }
}

void ColladaStreamReader::getVertices(vector<Opticks::Location<float, 3>>& vertices)
{
   vertices = vector<Opticks::Location<float, 3>>(mVertices.begin(), mVertices.end());
}

void ColladaStreamReader::getGeometryIds(vector<string>& geometryIds)
{
   geometryIds = vector<string>(mGeometryIds.begin(), mGeometryIds.end());
}

bool ColladaStreamReader::readVertices(const COLLADAFW::Mesh* pMesh)
{
   if (pMesh == NULL)
   {
      return false;
   }
   const COLLADAFW::MeshVertexData& meshData = pMesh->getPositions();
   unsigned int numVerts = 0;
   if (meshData.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_FLOAT)
   {
      const COLLADAFW::ArrayPrimitiveType<float>* values = meshData.getFloatValues();
      numVerts = values->getCount();
      for (unsigned int i = 0; i < numVerts; i += 3)
      {
         Opticks::Location<float, 3> vertexAsLocation((*values)[i], (*values)[i+1], (*values)[i+2]);
         mVertices.push_back(vertexAsLocation);
      }
   }
   else if (meshData.getType() == COLLADAFW::MeshVertexData::DATA_TYPE_DOUBLE)
   {
      const COLLADAFW::ArrayPrimitiveType<double>* values = meshData.getDoubleValues();
      numVerts = values->getCount();
      for (unsigned int i = 0; i < numVerts; i += 3)
      {
         Opticks::Location<float, 3> vertexAsLocation(static_cast<float>((*values)[i]), static_cast<float>((*values)[i+1]),
            static_cast<float>((*values)[i+2]));
         mVertices.push_back(vertexAsLocation);
      }
   }
   return true;
}

bool ColladaStreamReader::readPolygons(const COLLADAFW::Mesh* pMesh)
{
   if (pMesh == NULL)
   {
      return false;
   }
   const COLLADAFW::MeshPrimitiveArray& meshPrimitiveArray = pMesh->getMeshPrimitives();

   // Iterate over all mesh primitives that belong to the mesh
   for (unsigned int i = 0; i < meshPrimitiveArray.getCount(); ++i)
   {
      COLLADAFW::MeshPrimitive* pMeshPrimitive = meshPrimitiveArray[i];
      unsigned int* pVertexIndices = pMeshPrimitive->getPositionIndices().getData();
      int primitiveType = pMeshPrimitive->getPrimitiveType();

      if (primitiveType == COLLADAFW::MeshPrimitive::POLYLIST ||
         primitiveType == COLLADAFW::MeshPrimitive::POLYGONS)
      {
         COLLADAFW::Polygons* pPolygonPrimitive = static_cast<COLLADAFW::Polygons*>(pMeshPrimitive);
         if (pPolygonPrimitive == NULL)
         {
            return false;
         }

         COLLADAFW::Polygons::VertexCountArray& vertexCounts =
            pPolygonPrimitive->getGroupedVerticesVertexCountArray();

         // Iterate over all faces within the current primitive
         for (unsigned int j = 0; j < pMeshPrimitive->getFaceCount(); ++j)
         {
            int vertexCount = vertexCounts[j];
            vector<unsigned int> vertexIndices;
            for (int k = 0; k < vertexCount; ++k)
            {
               vertexIndices.push_back(pVertexIndices[k]);
            }
            mPolygons.push_back(vertexIndices);
            pVertexIndices += vertexCount;
         }
      }
   }
   return true;
}

bool ColladaStreamReader::writeVisualScene(const COLLADAFW::VisualScene* pScene)
{
   if (mFirstPass)
   {
      const COLLADAFW::VisualScene scene(*pScene);
      mScenes.push_back(scene);
   }
   return true;
}

void ColladaStreamReader::readSceneNodesRecursive(COLLADAFW::Node* pNode)
{
   if (pNode == NULL)
   {
      return;
   }
   if (ColladaUtilities::formatGeometryId(pNode->getName()) == mCurrentElement)
   {
      const COLLADAFW::TransformationPointerArray& transformations = pNode->getTransformations();
      for (unsigned int i = 0; i < transformations.getCount(); ++i)
      {
         COLLADAFW::Rotate* pRotation = dynamic_cast<COLLADAFW::Rotate*>(transformations[i]);
         // Get the rotation about Z, stored in the rotation axis list at position 2
         if (pRotation != NULL && pRotation->getRotationAxis()[2] == 1)
         {
            mRotation = dynamic_cast<COLLADAFW::Rotate*>(transformations[i])->getRotationAngle();
            return;
         }
      }
   }
   COLLADAFW::NodePointerArray& children = pNode->getChildNodes();
   for (unsigned int i = 0; i < children.getCount(); ++i)
   {
      readSceneNodesRecursive(children[i]);
   }
}

void ColladaStreamReader::setFilename(std::string filename)
{
   mFilename = filename;
}

std::string ColladaStreamReader::getFilename()
{
   return mFilename;
}

void ColladaStreamReader::setFirstPass(bool firstPass)
{
   mFirstPass = firstPass;
}

bool ColladaStreamReader::getFirstPass()
{
   return mFirstPass;
}

double ColladaStreamReader::getRotation()
{
   return mRotation;
}

#endif