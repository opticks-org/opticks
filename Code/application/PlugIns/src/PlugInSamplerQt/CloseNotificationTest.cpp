/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include "CloseNotificationTest.h"
#include "SessionItemSerializer.h"
#include "Slot.h"
#include <QtGui/QMessageBox>

CloseNotificationTest::CloseNotificationTest()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName( "Close Notification Test" );
   setDescription( "CloseNotificationTest" );
   setShortDescription( "CloseNotificationTest" );
   setMenuLocation( "[Tests]\\Close Notification Test" );
   destroyAfterExecute(false);
   setDescriptorId("{35F7F8F9-474B-42f8-9B2D-AFDBEC121C94}");

   mpAttachment.addSignal(SIGNAL_NAME(ApplicationServices, SessionClosed),
      Slot(this, &CloseNotificationTest::processSessionClosed));
}

CloseNotificationTest::~CloseNotificationTest()
{
}

bool CloseNotificationTest::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   // need to attach to ApplicationServices
   Service<ApplicationServices> pApplication;
   mpAttachment.reset(pApplication.get());

   QMessageBox::information( NULL, "Close Notification Test",
      "Now listening for an application close.\n\nClose the application when ready.", "OK" );   

   return true;
}

void CloseNotificationTest::processSessionClosed(Subject &pSubject, const std::string &signal, const boost::any &data)
{
   if (signal == ApplicationServices::signalSessionClosed())
   {
      QMessageBox::information(NULL, "Close Notification Test",
         "A Close Event was caught. This application is closing!", "OK");
   }
}

bool CloseNotificationTest::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool CloseNotificationTest::deserialize(SessionItemDeserializer &deserializer)
{
   Service<ApplicationServices> pApplication;
   mpAttachment.reset(pApplication.get());
   return true;
}
