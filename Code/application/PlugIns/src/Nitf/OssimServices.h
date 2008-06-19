/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OSSIM_SERVICES_H
#define OSSIM_SERVICES_H

#include "AlgorithmShell.h"
#include "AppVersion.h"
#include "SessionItemSerializer.h"

#include <ossim/init/ossimInit.h>

namespace Nitf
{

   class OssimServices : public AlgorithmShell
   {
   public:
      OssimServices()
      {
         setName("OSSIM Services");
         destroyAfterExecute(false);
         executeOnStartup(true);
         setDescriptorId("{FE6F67B0-2DC1-48d1-85EC-18DC62BE9980}");
         setProductionStatus(APP_IS_PRODUCTION_RELEASE);

         ossimInit::instance()->initialize();
      }

      ~OssimServices()
      {
         ossimInit::instance()->finalize();
      }

      bool getInputSpecification(PlugInArgList*& pArgs)
      {
         pArgs = NULL;
         return true;
      }

      bool getOutputSpecification(PlugInArgList*& pArgs)
      {
         pArgs = NULL;
         return true;
      }

      bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
      {
         return true;
      }

      bool serialize( SessionItemSerializer &serializer )const
      {
         return serializer.serialize(NULL, 0);
      }

      bool deserialize( SessionItemDeserializer &deserializer )
      {
         return true;
      }

   };
}

#endif
