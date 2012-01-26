/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "COLLADAFWIWriter.h"
#include "COLLADAFWMesh.h"
#include "COLLADAFWNode.h"

#include "Location.h"

#include <vector>

class FileInfo;
class Scene;
class VisualScene;
class Geometry;
class LibraryNodes;
class Material;
class Effect;
class Camera;
class Image;
class Light;
class Animation;
class AnimationList;
class SkinControllerData;
class Controller;
class Formulas;
class KinematicsScene;

class ColladaStreamReader : public COLLADAFW::IWriter
{
public:
   /**
    * Default constructor.
    */
   ColladaStreamReader();
   /**
    * Constructor that takes in a filename.
    */
   ColladaStreamReader(std::string filename);
   /**
    * Default destructor.
    */
   virtual ~ColladaStreamReader();

   /**
    * Reads through the file that was set in the constructor, retrieving the data for
    * the geometry ID that was passed in as a parameter.
    *
    * @param element
    *   ID of the geometry to be found in the file.
    * @return Returns true if the file read was successful, false if any errors occurred while reading the file.
    */
   bool read(std::string element);
   /**
    * Reads all of the geometry IDs from the file set in the constructor and places them
    * in the vector that is passed.
    *
    * @param geomIds
    *   Vector where all of the geometry IDs will be placed.
    */
   void populateGeometryIds(std::vector<std::string>& geomIds);

   /**
    * Sets the location of the file on disk that is to be read.
    *
    * @param filename
    *   Location of the COLLADA file on disk.
    */
   void setFilename(std::string filename);
   /**
    * Retrieves the currently set filename.
    *
    * @return The name of the file to be read.
    */
   std::string getFilename();
   /**
    * Used to set whether the reader is on the first pass through the file
    * and needs to retrieve all of the geometry IDs. Subsequent passes
    * are used to read in geometry data based on specific geometry IDs.
    *
    * @param firstPass
    *   Boolean value to set the first pass status of the reader.
    */
   void setFirstPass(bool firstPass);
   /**
    * Returns whether the reader is on its first pass through the file or
    * is on a subsequent pass.
    *
    * @return Returns true if the reader is on its first pass through the file, false
    * if on a subsequent pass.
    */
   bool getFirstPass();

   /**
    * Populates the vector parameter with all of the geometry IDs from the file set in the constructor.
    *
    * @param geometryIds
    *   Vector to be filled with all of the geometry IDs in the file set in the constructor.
    */
   void getGeometryIds(std::vector<std::string>& geometryIds);
   /**
    * Populates the vector parameter with the vertices retrieved for the most recently
    * read geometry ID.
    *
    * @param vertices
    *   Vector to be populated with the geometry's vertex data.
    */
   void getVertices(std::vector<Opticks::Location<float, 3>>& vertices);
   /**
    * Populates the vector parameter with polygonal data that defines the order in which
    * the vertices should be drawn.
    *
    * @param polygons
    *   Vector to be populated with the geometry's polygonal data.
    */
   void getPolygons(std::vector<std::vector<unsigned int>>& polygons);

   /**
    * Returns the rotation for the current element.
    *
    * @return The rotation for the element from the file.
    */
   double getRotation();

   /*
    * OpenCOLLADA SAX interface methods
    */
   virtual void cancel(const std::string& errorMessage) {};
   virtual void start();
   virtual void finish();
   virtual bool writeGlobalAsset(const COLLADAFW::FileInfo* pAsset) { return true; }
   virtual bool writeScene(const COLLADAFW::Scene* pScene) { return true; }
   virtual bool writeVisualScene(const COLLADAFW::VisualScene* pVisualScene);
   virtual bool writeLibraryNodes(const COLLADAFW::LibraryNodes* pLibraryNodes) { return true; }
   virtual bool writeGeometry(const COLLADAFW::Geometry* pGeometry);
   virtual bool writeMaterial(const COLLADAFW::Material* pMaterial) { return true; }
   virtual bool writeEffect(const COLLADAFW::Effect* pEffect) { return true; }
   virtual bool writeCamera(const COLLADAFW::Camera* pCamera) { return true; }
   virtual bool writeImage(const COLLADAFW::Image* pImage) { return true; }
   virtual bool writeLight(const COLLADAFW::Light* pLight) { return true; }
   virtual bool writeAnimation(const COLLADAFW::Animation* pAnimation) { return true; }
   virtual bool writeAnimationList(const COLLADAFW::AnimationList* pAnimationList) { return true; }
   virtual bool writeSkinControllerData(const COLLADAFW::SkinControllerData* pSkinControllerData) { return true; }
   virtual bool writeController(const COLLADAFW::Controller* pController) { return true; }
   virtual bool writeFormulas(const COLLADAFW::Formulas* pFormulas) { return true; }
   virtual bool writeKinematicsScene(const COLLADAFW::KinematicsScene* pKinematicsScene) { return true; }

private:
   bool readVertices(const COLLADAFW::Mesh* pMesh);
   bool readPolygons(const COLLADAFW::Mesh* pMesh);
   void readSceneNodesRecursive(COLLADAFW::Node* pNode);

   std::vector<std::string> mGeometryIds;
   std::vector<Opticks::Location<float, 3>> mVertices;
   std::vector<std::vector<unsigned int>> mPolygons;
   std::vector<const COLLADAFW::VisualScene> mScenes;
   double mRotation;
   std::string mCurrentElement;
   std::string mFilename;
   bool mFirstPass;
};
