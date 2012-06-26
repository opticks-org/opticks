/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLLADASTREAMWRITER_H
#define COLLADASTREAMWRITER_H

#include "AppConfig.h"
#if defined(OPENCOLLADA_SUPPORT)

#include "Location.h"

#include <COLLADASWLibraryGeometries.h>
#include <COLLADASWLibraryVisualScenes.h>
#include <COLLADASWStreamWriter.h>

class GraphicObject;

class ColladaStreamWriter
{
public:
   /**
    * Default constructor.
    */
   ColladaStreamWriter();
   /**
    * Constructor that sets the desired file to be written to.
    *
    * @param filename
    *   Location on disk where the COLLADA document should be written.
    */
   ColladaStreamWriter(std::string filename);
   /**
    * Default destructor.
    */
   virtual ~ColladaStreamWriter();

   /**
    * Writes the passed list of GraphicObjects to disk.
    *
    * @param objects
    *   List of GraphicObjects to be written to disk.
    */
   void writeGraphicObjects(std::list<GraphicObject*> objects);

private:
   void beginDocument(COLLADASW::StreamWriter* pStreamWriter);
   void endDocument(COLLADASW::StreamWriter* pStreamWriter);

   std::string mFilename;
};

/*
 * OpenCOLLADA requires implementation of interfaces for its Library* classes, which when instantiated are objects that
 * are used to write out the various elements that can be stored in a COLLADA document.
 * Opticks support currently exists for writing:
 * - Geometry
 * - Scene
 */
class ColladaGeometryLibrary : public COLLADASW::LibraryGeometries
{
public:
   /**
    * Constructor that takes a COLLADASW::StreamWriter.
    *
    * @param pStreamWriter
    *   Pointer to a COLLLADASW::StreamWriter that is used to write
    *   all of the geometry elements to disk.
    */
   ColladaGeometryLibrary(COLLADASW::StreamWriter* pStreamWriter);
   /**
    * Default destructor.
    */
   ~ColladaGeometryLibrary();

   /**
    * Writes an n-sided polygon to a COLLADA file.
    *
    * @param geometryId
    *   ID of the geometry to be written.
    * @param vertices
    *   Vector of vertices that define the polygon.
    */
   void writeArbitraryPolygon(std::string geometryId, const std::vector<Opticks::Location<float, 3>>& vertices);

   /**
    * Generates a URI for elements based on the given ID and element semantic type
    *
    * @param geometryId
    *   Base geometry identifier
    * @param type
    *   Type of attribute or element that a URI needs to be generated for.
    * @param other_suffix
    *   Optional additional suffix for the attribute/element.
    * @return A URI object with the geometry ID and any required suffixes.
    */
   COLLADASW::URI getUriBySemantics(std::string geometryId, COLLADASW::InputSemantic::Semantics type,
      std::string other_suffix = "");

   /**
    * Generates an ID for an element/attribute based on COLLADA semantics.
    *
    * @param geometryId
    *   Base geometry identifier
    * @param type
    *   Type of attribute or element that an ID needs to be generated for. The Library class has a method which will
    *   generate a string based on the enum passed here.
    * @param other_suffix
    *   Optional additional suffix for the attribute/element
    * @return A string with the geometry ID and any required suffixes.
    */
   std::string getIdBySemantics(std::string geometryId, COLLADASW::InputSemantic::Semantics type,
      std::string other_suffix = "");

private:
   std::string mGeometryId;
};

class ColladaSceneLibrary : public COLLADASW::LibraryVisualScenes
{
public:
   /**
    * Constructor that takes a COLLADASW::StreamWriter.
    *
    * @param pStreamWriter
    *   Pointer to a COLLLADASW::StreamWriter that is used to write
    *   all of the scene elements to disk.
    */
   ColladaSceneLibrary(COLLADASW::StreamWriter* pStreamWriter);
   /**
    * Default destructor.
    */
   ~ColladaSceneLibrary();

   /**
    * Prepares the document to begin writing scene information.
    *
    * @param sceneId
    *   ID of the scene that will be written.
    */
   void startScene(std::string sceneId);
   /**
    * Finalizes the scene section of the document for the current scene element.
    */
   void endScene();

   /**
    * Writes a basic scene, composed of only a single node.
    */
   void writeSimpleGeometryNode(std::string nodeId, double rotation);
};

#endif
#endif
