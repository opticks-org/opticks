/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DummyCustomImporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"

using namespace std;

DummyCustomImporter::DummyCustomImporter()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName("Dummy Custom Importer");
   setDescription("DummyCustomImporter");
   setDescriptorId("{B5BC70E6-8159-4475-BFDC-5B59F77AFAE4}");
}

DummyCustomImporter::~DummyCustomImporter()
{
}

bool DummyCustomImporter::getInputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;

   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   if( pArg != NULL )
   {
      pArg->setName( "Filename" );
      pArg->setType( "string" );
      pArgList->addArg( *pArg );
   }

   return true;
}

bool DummyCustomImporter::getOutputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;

   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   if( pArg != NULL )
   {
      mpModel->addElementType("DummyType");
      pArg->setName( "Custom Test Element" );
      pArg->setType( "DummyType" );
      pArgList->addArg( *pArg );
   }

   return true;
}

bool DummyCustomImporter::setBatch()
{
   return true;
}

bool DummyCustomImporter::setInteractive()
{
   return true;
}

bool DummyCustomImporter::hasAbort()
{
   return true;
}

bool DummyCustomImporter::abort()
{
   return true;
}

vector<ImportDescriptor*> DummyCustomImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> data;
   return data;
}

bool DummyCustomImporter::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   return true;
}

unsigned char DummyCustomImporter::getFileAffinity( const std::string& filename )
{
   return Importer::CAN_NOT_LOAD;
}
