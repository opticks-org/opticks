/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMMANAGER_H
#define ASPAMMANAGER_H

#include "AlgorithmShell.h"
#include "Subject.h"
#include "SubjectImp.h"

class Any;
class Aspam;

/**
 *  Memory manager plug-in for the Aspam data model.
 *
 *  This class is a stay resident plug-in which handles memory management
 *  for the Aspam data model. When instantiated, this plug-in registers
 *  the Aspam data type with ModelServices. When a new Aspam is created
 *  by ModelServices, an Any data element is created. The stay resident
 *  instance of this plug-in is used to populate the AnyData portion of the
 *  Aspam's Any element. An example can be seen in AspamImporter.
 *
 *  This plug-in also handles clean up of the AnyData portions when it is destroyed.
 *  Do not destroy the plug-in instance manually or any existing Aspam Any elements
 *  will have their AnyData portions deleted and set to NULL.
 *
 *  @see ModelServices, Any, AnyData, AspamImporter
 */
class AspamManager : public AlgorithmShell, public SubjectImp, public Subject
{
public:
   AspamManager();
   ~AspamManager();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   /**
    *  Initialize the AnyData portion of an Aspam Any element.
    *
    *  If the Any element already has an AnyData associated with it,
    *  this function will fail.
    *
    *  @param pAspamContainer
    *         The Any element for the Aspam. This should be created
    *         with model services.
    *
    *  @return True if successful, false otherwise.
    */
   bool initializeAspam(Any *pAspamContainer);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
   SUBJECTADAPTER_METHODS(SubjectImp);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

public: // Signals
   /**
    *  Emitted with any<Any*> when an Any is initialized with an Aspam object
    */
   SIGNAL_METHOD(AspamManager, AspamInitialized)
};

#endif
