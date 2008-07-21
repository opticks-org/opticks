/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PixelAspectRatio.h"
#include "PixelAspectRatioGui.h"
#include "Service.h"
#include "SessionItemSerializer.h"

PixelAspectRatio::PixelAspectRatio()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   ViewerShell::setName( "Pixel Aspect Ratio Test" );
   setDescription( "Verifies MPR1 Requirement 180." );
   setMenuLocation( "[Tests]\\Pixel Aspect Ratio Test" );
   setDescriptorId("{626E12E9-41C0-489e-B92A-1E495083E69C}");
   mpGui = NULL;
   destroyAfterExecute( false );
}

PixelAspectRatio::~PixelAspectRatio()
{
}

bool PixelAspectRatio::showGui()
{
   Service<DesktopServices> pDesktop;
   Service<ModelServices> pModel;
   StepResource pStep( "Pixel Aspect Ratio Started.", "app", "5E4BCD48-E662-408b-93AF-F9127CE56C66" );

   std::vector<DataElement*> cubes = pModel->getElements( "RasterElement" );
   if( cubes.size() == 0 )
   {
      QMessageBox::critical( NULL, "Pixel Aspect Ratio Test", "No RasterElement input found!", "OK" );
      pStep->finalize( Message::Failure, "No RasterElement input found!" );
      return false;
   }

   mpGui = new PixelAspectRatioGui( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   mpGui->show();

   pStep->finalize( Message::Success );
   return true;
}

bool PixelAspectRatio::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   return showGui();
}

QWidget* PixelAspectRatio::getWidget() const
{
   return mpGui;
}

void PixelAspectRatio::dialogClosed()
{
   abort();
}

bool PixelAspectRatio::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool PixelAspectRatio::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}
