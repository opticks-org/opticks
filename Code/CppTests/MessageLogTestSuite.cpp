/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "assert.h"
#include "DesktopServicesImp.h"
#include "DynamicObject.h"
#include "GraphicGroup.h"
#include "LayerList.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"
#include "MessageLogResource.h"
#include "ModelServicesImp.h"
#include "Observer.h"
#include "RasterElement.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "xmlwriter.h"

using namespace std;

vector<string> actionVector;
vector<string> updateVector;

const std::string attachedAction = "Attached";
const std::string detachedAction = "Detached";

class SubjectObserver
{
public:
   // A compile-time error occurs if a class w/ no virtual methods is used as a slot object
   virtual ~SubjectObserver() {}
   void update( Subject& subject, const string &signal, const boost::any& v)
   {      
      cout << "\tNotified of " << subject.getObjectType() << " after attaching." << endl;
   }
};

class MessageLogObserver : public Observer
{
public:
   void update( Subject& subject, const string &signal, const boost::any& v)
   {
      //if ( signal == SIGNAL_NAME(Message, MessageModified) ||
      if ( signal == SIGNAL_NAME(Subject, Modified) ||
         signal == SIGNAL_NAME(MessageLog, MessageAdded) ||
         signal == SIGNAL_NAME(MessageLog, MessageModified))
      {
         Message* pMessage = boost::any_cast<Message*>( v );
         actionVector.push_back( "Created Message with action = " + pMessage->getAction() );
      }
      updateVector.push_back( signal );
   }
   void MessageLogObserver::attached(Subject &subject, const string &signal, const Slot &slot)
   {
      updateVector.push_back( "Attached" );
   }
   void MessageLogObserver::detached(Subject &subject, const string &signal, const Slot &slot)
   {
      updateVector.push_back( signal );
   }
};

class MessageLogManagerObserver
{
public:
   // A compile-time error occurs if a class w/ no virtual methods is used as a slot object
   virtual ~MessageLogManagerObserver() {}
   void update( Subject& subject, const string &signal, const boost::any& v)
   {
      cout << "\tNotified of " << subject.getObjectType() << " with signal " << signal << endl;
   }
};

class MessageLogTest : public TestCase
{
public:
   MessageLogTest() : TestCase("Main") {}
   bool run()
   {
      bool success = true;
      string logName = "cppTestLog";
      bool ok = false;

      Service<MessageLogMgr> pLogMgr;
      issearf(pLogMgr.get() != NULL);

      MessageLogManagerObserver *pManagerObserver = new MessageLogManagerObserver();
      pLogMgr->attach( Subject::signalModified(), Slot(pManagerObserver, &MessageLogManagerObserver::update) );

      MessageLog* pLog = pLogMgr->getLog(logName);
      if (pLog == NULL)
      {
         pLog = pLogMgr->createLog(logName);
      }

      issearf(pLog != NULL);

      MessageLogObserver* pObserver = new MessageLogObserver();
      pLog->attach( SIGNAL_NAME(MessageLog, MessageAdded), Slot(pObserver, &MessageLogObserver::update) );
      pLog->attach( SIGNAL_NAME(MessageLog, MessageModified), Slot(pObserver, &MessageLogObserver::update) );
      pLog->attach( SIGNAL_NAME(MessageLog, MessageHidden), Slot(pObserver, &MessageLogObserver::update) );

      // try to log a list of strings
      vector<string> stringVector;
      stringVector.push_back( "This" );
      stringVector.push_back( "test" );
      stringVector.push_back( "list" );
      stringVector.push_back( "should" );
      stringVector.push_back( "log" );
      stringVector.push_back( "just" );
      stringVector.push_back( "fine" );

      Step *pStep = NULL;
      pStep = pLog->createStep( "try to log a list of strings", "cppTests", "637A87EC-565D-402F-9A5B-E6CAA7E124AF" );
      issea( pStep != NULL );

      ok = pStep->addProperty( "stringVector", stringVector );
      issea( ok == true );
      ok = pStep->finalize( Message::Success );
      issea( ok == true );

      // try to log a list of ints
      vector<float> intVector;
      intVector.push_back( 10 );
      intVector.push_back( 20 );
      intVector.push_back( 30 );
      intVector.push_back( 40 );
      intVector.push_back( 50 );
      intVector.push_back( 60 );

      Step *pStep2 = NULL;
      pStep2 = pLog->createStep( "try to log a list of ints", "cppTests", "C61FE788-EE2F-40F2-A0A8-6B9BA4B2BF16" );
      issea( pStep2 != NULL );

      ok = pStep2->addProperty( "intVector", intVector );
      issea( ok == true );
      ok = pStep2->finalize( Message::Success );
      issea( ok == true );

      string tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 4 );
      issea( updateVector.size() == 9 );

      issea( updateVector.at( 0 ) == attachedAction );
      issea( updateVector.at( 1 ) == attachedAction );
      issea( updateVector.at( 2 ) == attachedAction );
      issea( updateVector.at( 3 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 4 ) == SIGNAL_NAME(MessageLog, MessageModified) );
      issea( updateVector.at( 5 ) == SIGNAL_NAME(MessageLog, MessageHidden) );
      issea( updateVector.at( 6 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 7 ) == SIGNAL_NAME(MessageLog, MessageModified) );
      issea( updateVector.at( 8 ) == SIGNAL_NAME(MessageLog, MessageHidden) );

      issea( actionVector.at( 0 ) == "Created Message with action = try to log a list of strings" );
      issea( actionVector.at( 1 ) == "Created Message with action = try to log a list of strings" );
      issea( actionVector.at( 2 ) == "Created Message with action = try to log a list of ints" );
      issea( actionVector.at( 3 ) == "Created Message with action = try to log a list of ints" );

      // now do a recursive test
      actionVector.clear();
      updateVector.clear();

      pStep = NULL;
      pStep = pLog->createStep( "try to log a list of strings again", "cppTests", "762D548F-0932-438C-927D-8EA9133DAAFA" );
      issea( pStep != NULL );

      ok = pStep->addProperty( "stringVector", stringVector );
      issea( ok == true );

      pStep2 = NULL;
      pStep2 = pLog->createStep( "try to log a list of ints again", "cppTests", "441B4821-02C0-456C-A3E2-1078A6B2E482" );
      issea( pStep2 != NULL );

      pStep2->attach( SIGNAL_NAME(MessageLog, MessageAdded), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageModified), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageHidden), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver, &MessageLogObserver::update) );
      ok = pStep2->addProperty( "intVector", intVector );
      issea( ok == true );

      ok = pStep2->finalize( Message::Success );
      issea( ok == true );

      ok = pStep->finalize( Message::Success );
      issea( ok == true );

      tempName = "";
      tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 5 );
      issea( updateVector.size() == 10 );

      issea( updateVector.at( 0 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 1 ) == SIGNAL_NAME(MessageLog, MessageModified) );
      issea( updateVector.at( 2 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 3 ) == attachedAction );
      issea( updateVector.at( 4 ) == attachedAction );
      issea( updateVector.at( 5 ) == attachedAction );
      issea( updateVector.at( 6 ) == attachedAction );
      issea( updateVector.at( 7 ) == SIGNAL_NAME(Subject, Modified) );
      issea( updateVector.at( 8 ) == SIGNAL_NAME(Subject, Modified) );
      issea( updateVector.at( 9 ) == SIGNAL_NAME(MessageLog, MessageHidden) );

      issea( actionVector.at( 0 ) == "Created Message with action = try to log a list of strings again" );
      issea( actionVector.at( 1 ) == "Created Message with action = try to log a list of strings again" );
      issea( actionVector.at( 2 ) == "Created Message with action = try to log a list of ints again" );
      issea( actionVector.at( 3 ) == "Created Message with action = try to log a list of ints again" );
      issea( actionVector.at( 4 ) == "Created Message with action = try to log a list of ints again" );

      pStep2->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      // check to see if a parent finalize does a finalize on child messages
      actionVector.clear();
      updateVector.clear();

      pStep = NULL;
      pStep = pLog->createStep( "try to log a list of strings yet again", "cppTests",
         "A25CE0D6-3ABE-4EB5-B78C-7AA063B1EA2E" );
      issea( pStep != NULL );

      pStep2 = NULL;
      pStep2 = pLog->createStep( "try to log a list of ints yet again", "cppTests", "AC94714A-7862-4A9A-ACE8-B0668D0C6C88" );
      issea( pStep2 != NULL );

      pStep2->attach( SIGNAL_NAME(MessageLog, MessageAdded), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageModified), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageHidden), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      ok = pStep->finalize( Message::Success );
      issea( ok == true );

      issea( pStep2->getResult() == Message::Success );

      tempName = "";
      tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 3 );
      issea( updateVector.size() == 8 );

      issea( updateVector.at( 0 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 1 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 2 ) == attachedAction );
      issea( updateVector.at( 3 ) == attachedAction );
      issea( updateVector.at( 4 ) == attachedAction );
      issea( updateVector.at( 5 ) == attachedAction );
      issea( updateVector.at( 6 ) == SIGNAL_NAME(Subject, Modified) );
      issea( updateVector.at( 7 ) == SIGNAL_NAME(MessageLog, MessageHidden) );

      issea( actionVector.at( 0 ) == "Created Message with action = try to log a list of strings yet again" );
      issea( actionVector.at( 1 ) == "Created Message with action = try to log a list of ints yet again" );
      issea( actionVector.at( 2 ) == "Created Message with action = try to log a list of ints yet again" );

      pStep2->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      // check to see if a call to finalize() multiple times behaves correctly
      actionVector.clear();
      updateVector.clear();

      pStep = NULL;
      pStep = pLog->createStep( "try to log a list of strings once more", "cppTests",
         "5A2F673A-0346-49FC-B922-04DE8E49CE3F" );
      issea( pStep != NULL );

      pStep2 = NULL;
      pStep2 = pLog->createStep( "try to log a list of ints once more", "cppTests", "24D7FD15-8E59-4132-8605-6AB2B01BE1B7" );
      issea( pStep2 != NULL );

      pStep2->attach( SIGNAL_NAME(MessageLog, MessageAdded), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageModified), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(MessageLog, MessageHidden), Slot(pObserver, &MessageLogObserver::update) );
      pStep2->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      ok = pStep2->finalize( Message::Success );
      issea( ok == true );

      ok = pStep->finalize( Message::Success );
      issea( ok == true );

      ok = pStep2->finalize( Message::Failure );
      issea( ok == false );

      ok = pStep->finalize( Message::Failure );
      issea( ok == false );

      // they should both still be set to Success
      issea( pStep->getResult() == Message::Success );
      issea( pStep2->getResult() == Message::Success );

      tempName = "";
      tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 3 );
      issea( updateVector.size() == 8 );

      issea( updateVector.at( 0 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 1 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 2 ) == attachedAction );
      issea( updateVector.at( 3 ) == attachedAction );
      issea( updateVector.at( 4 ) == attachedAction );
      issea( updateVector.at( 5 ) == attachedAction );
      issea( updateVector.at( 6 ) == SIGNAL_NAME(Subject, Modified) );
      issea( updateVector.at( 7 ) == SIGNAL_NAME(MessageLog, MessageHidden) );

      issea( actionVector.at( 0 ) == "Created Message with action = try to log a list of strings once more" );
      issea( actionVector.at( 1 ) == "Created Message with action = try to log a list of ints once more" );
      issea( actionVector.at( 2 ) == "Created Message with action = try to log a list of ints once more" );

      pStep2->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      // check to see if an un-finalized message has a result of Unresolved
      actionVector.clear();
      updateVector.clear();

      pStep = NULL;
      pStep = pLog->createStep( "This message is unresolved.", "cppTests", "C7DF8761-95A3-4A69-A52E-F6F4A7A1799B" );
      issea( pStep != NULL );

      pStep2 = NULL;
      pStep2 = pLog->createStep( "So is this message.", "cppTests", "E7D201DA-1472-4802-AC75-75CF902DB253" );
      issea( pStep2 != NULL );

      // they should both still be set to Unresolved
      issea( pStep->getResult() == Message::Unresolved );
      issea( pStep2->getResult() == Message::Unresolved );

      tempName = "";
      tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 2 );
      issea( updateVector.size() == 2 );

      issea( updateVector.at( 0 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 1 ) == SIGNAL_NAME(MessageLog, MessageAdded) );

      issea( actionVector.at( 0 ) == "Created Message with action = This message is unresolved." );
      issea( actionVector.at( 1 ) == "Created Message with action = So is this message." );

      //finalize both of the steps we just created.
      pStep->finalize(Message::Success);

      // check to see if a property can be added after a finalize
      actionVector.clear();
      updateVector.clear();

      pStep = NULL;
      pStep = pLog->createStep( "This message is a test.", "cppTests", "2D64A0ED-FA56-429C-84DA-94355B661850" );
      issea( pStep != NULL );

      pStep2 = NULL;
      pStep2 = pLog->createStep( "So is this message.", "cppTests", "12CCFD42-4E89-4B9F-8391-6EFE48B5563C" );
      issea( pStep2 != NULL );

      pStep2->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));

      ok = pStep->finalize( Message::Success );  // finalize inner most step
      issea( ok == true );

      issea( pStep2->getResult() == Message::Success );  // this should get finalized automatically

      // now try to add a property to each
      ok = pStep->addProperty( "stringVector", stringVector );
      issea( ok == false );

      ok = pStep2->addProperty( "intVector", intVector );
      issea( ok == false );

      tempName = "";
      tempName = pLog->getLogName();
      issea( tempName == "cppTestLog" );

      issea( actionVector.size() == 3 );
      issea( updateVector.size() == 5 );

      issea( updateVector.at( 0 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 1 ) == SIGNAL_NAME(MessageLog, MessageAdded) );
      issea( updateVector.at( 2 ) == attachedAction );
      issea( updateVector.at( 3 ) == SIGNAL_NAME(Subject, Modified) );
      issea( updateVector.at( 4 ) == SIGNAL_NAME(MessageLog, MessageHidden) );

      issea( actionVector.at( 0 ) == "Created Message with action = This message is a test." );
      issea( actionVector.at( 1 ) == "Created Message with action = So is this message." );
      issea( actionVector.at( 2 ) == "Created Message with action = So is this message." );

      pStep2->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &MessageLogObserver::update));
      pLogMgr->detach( SIGNAL_NAME(Subject, Modified), Slot(pManagerObserver,  &MessageLogManagerObserver::update));

      return success;
   }
};

class MessageLogSubscribeTest : public TestCase
{
public:
   MessageLogSubscribeTest() : TestCase("Subscribe") {}
   bool run()
   {
      bool success = true;

      Service<MessageLogMgr> pLogMgr;
      issearf(pLogMgr.get() != NULL);

      {
         //create a message in session log, so that we can find the log in the code below
         MessageResource msg("Test Step", "TestComponent", "TestKey");
      }

      vector<MessageLog*> pLogVector = pLogMgr->getLogs();
      string logName = "";

      issea( pLogVector.size() >= 2 ); // there should be at least 2 logs currently in the session

      bool foundProject = false;
      bool foundTestlog = false;

      for( unsigned int i = 0; i < pLogVector.size(); i++ )
      {
         logName = pLogVector.at( i )->getLogName();

         if( logName.find( Service<SessionManager>()->getName(), 0 ) != string::npos )  // the cube being used
         {
            foundProject = true;
         }
         else if( logName.compare( "cppTestLog" ) == 0 )  // the cpp test log created above
         {
            foundTestlog = true;
         }
      }
      issea( foundProject == true && foundTestlog == true );

      return success;
   }
};

class MessageLogKeyComponentTest : public TestCase
{
public:
   MessageLogKeyComponentTest() : TestCase("KeyComponent") {}
   bool run()
   {
      bool success = true;
      string logName = "cppTestLog";
      bool ok = false;

      Service<MessageLogMgr> pLogMgr;
      issearf(pLogMgr.get() != NULL);

      MessageLog* pLog = pLogMgr->getLog(logName);
      if (pLog == NULL)
      {
         pLog = pLogMgr->createLog(logName);
      }

      issearf(pLog != NULL);

      Step *pStep = NULL;
      pStep = pLog->createStep( "This is a new Step.", "cppTests", "newStep1" );
      issea( pStep != NULL );
      pStep->finalize( Step::Success );

      Message *pMessage = NULL;
      pMessage = pLog->createMessage( "This is a new Message.", "cppTests", "newMessage" );
      issea( pMessage != NULL );
      pMessage->finalize();

      Step *pStep2 = NULL;
      pStep2 = pLog->createStep( "This is a new Step also.", "cppTests", "newStep2" );
      issea( pStep2 != NULL );
      pStep2->finalize( Step::Success );

      Message *pMessage2 = NULL;
      pMessage2 = pLog->createMessage( "This is a new Message also.", "cppTests", "newMessage2" );
      issea( pMessage2 != NULL );
      pMessage2->finalize();

      Step *pStep3 = NULL;
      pStep3 = pLog->createStep( "This is a new Step as well.", "cppTests2", "newStep2" );
      issea( pStep3 != NULL );
      pStep3->finalize( Step::Success );

      Message *pMessage3 = NULL;
      pMessage3 = pLog->createMessage( "This is a new Message as well.", "cppTests", "newMessage3" );
      issea( pMessage3 != NULL );
      pMessage3->finalize();

      issea( pStep->getAction() == "This is a new Step." );
      issea( pStep->getComponent() == "cppTests" );
      issea( pStep->getKey() == "newStep1" );
      issea( pStep2->getAction() == "This is a new Step also." );
      issea( pStep2->getComponent() == "cppTests" );
      issea( pStep2->getKey() == "newStep2" );
      issea( pStep3->getAction() == "This is a new Step as well." );
      issea( pStep3->getComponent() == "cppTests2" );
      issea( pStep3->getKey() == "newStep2" );

      issea( pMessage->getAction() == "This is a new Message." );
      issea( pMessage->getComponent() == "cppTests" );
      issea( pMessage->getKey() == "newMessage" );
      issea( pMessage2->getAction() == "This is a new Message also." );
      issea( pMessage2->getComponent() == "cppTests" );
      issea( pMessage2->getKey() == "newMessage2" );
      issea( pMessage3->getAction() == "This is a new Message as well." );
      issea( pMessage3->getComponent() == "cppTests" );
      issea( pMessage3->getKey() == "newMessage3" );

      return success;
   }
};

class MessageLogIteratorTest : public TestCase
{
public:
   MessageLogIteratorTest() : TestCase("MessageLogIterator") {}
   bool run()
   {
      bool success = true;
      string logName = "emptyCppTestLog";
      bool ok = false;

      Service<MessageLogMgr> pLogMgr;
      issearf(pLogMgr.get() != NULL);

      MessageLog* pLog = pLogMgr->getLog(logName);
      if (pLog == NULL)
      {
         pLog = pLogMgr->createLog(logName);
      }

      issearf(pLog != NULL);

      Step *pStep = NULL;
      pStep = pLog->createStep( "This is Step Number 1.", "cppTests1", "newStep1" );
      issea( pStep != NULL );
      pStep->addProperty( "stepProperty1", 1 );
      pStep->finalize( Step::Success );

      Message *pMessage = NULL;
      pMessage = pLog->createMessage( "This is Message Number 1.", "cppTests1", "newMessage1" );
      issea( pMessage != NULL );
      pMessage->addProperty( "MessageProperty1", 1 );
      pMessage->finalize();

      pStep = NULL;
      pStep = pLog->createStep( "This is Step Number 2.", "cppTests2", "newStep2" );
      issea( pStep != NULL );
      pStep->addProperty( "stepProperty2", 2 );
      pStep->finalize( Step::Success );

      pMessage = NULL;
      pMessage = pLog->createMessage( "This is Message Number 2.", "cppTests2", "newMessage2" );
      issea( pMessage != NULL );
      pMessage->addProperty( "MessageProperty2", 2 );
      pMessage->finalize();

      pStep = NULL;
      pStep = pLog->createStep( "This is Step Number 3.", "cppTests3", "newStep3" );
      issea( pStep != NULL );
      pStep->addProperty( "stepProperty3", 3 );
      pStep->finalize( Step::Success );

      pMessage = NULL;
      pMessage = pLog->createMessage( "This is Message Number 3.", "cppTests3", "newMessage3" );
      issea( pMessage != NULL );
      pMessage->addProperty( "MessageProperty3", 3 );
      pMessage->finalize();

      pStep = NULL;
      pStep = pLog->createStep( "This is Step Number 4.", "cppTests4", "newStep4" );
      issea( pStep != NULL );
      pStep->addProperty( "stepProperty4", 4 );
      pStep->finalize( Step::Success );

      pMessage = NULL;
      pMessage = pLog->createMessage( "This is Message Number 4.", "cppTests4", "newMessage4" );
      issea( pMessage != NULL );
      pMessage->addProperty( "MessageProperty4", 4 );
      pMessage->finalize();

      pStep = NULL;
      pStep = pLog->createStep( "This is Step Number 5.", "cppTests5", "newStep5" );
      issea( pStep != NULL );
      pStep->addProperty( "stepProperty5", 5 );
      pStep->finalize( Step::Success );

      pMessage = NULL;
      pMessage = pLog->createMessage( "This is Message Number 5.", "cppTests5", "newMessage5" );
      issea( pMessage != NULL );
      pMessage->addProperty( "MessageProperty5", 5 );
      pMessage->finalize();

      MessageLog::iterator myItr = pLog->begin();
      int entryCount = 0;
      Message *pTemp = *myItr;

      // verify the contents of the log
      if( pTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         issea( pProperties->getNumAttributes() == 0 );
         issea( pTemp->getStringId() == "1" );
         issea( pTemp->getAction() == "Log Opened" );
         issea( pTemp->getComponent() == "app" );
         issea( pTemp->getKey() == "EC355E3E-03CA-4081-9006-5F45D6A488B3" );
         issea( pTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      Step *pTempStep = dynamic_cast<Step*>( pTemp );
      if( pTempStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty1" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 1 );
         issea( pTempStep->getStringId() == "2" );
         issea( pTempStep->getAction() == "This is Step Number 1." );
         issea( pTempStep->getComponent() == "cppTests1" );
         issea( pTempStep->getKey() == "newStep1" );
         issea( pTempStep->getResult() == Message::Success );
         issea( pTempStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      if( pTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty1" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 1 );
         issea( pTemp->getStringId() == "3" );
         issea( pTemp->getAction() == "This is Message Number 1." );
         issea( pTemp->getComponent() == "cppTests1" );
         issea( pTemp->getKey() == "newMessage1" );
         issea( pTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      pTempStep = dynamic_cast<Step*>( pTemp );
      if( pTempStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty2" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 2 );
         issea( pTempStep->getStringId() == "4" );
         issea( pTempStep->getAction() == "This is Step Number 2." );
         issea( pTempStep->getComponent() == "cppTests2" );
         issea( pTempStep->getKey() == "newStep2" );
         issea( pTempStep->getResult() == Message::Success );
         issea( pTempStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      if( pTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty2" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 2 );
         issea( pTemp->getStringId() == "5" );
         issea( pTemp->getAction() == "This is Message Number 2." );
         issea( pTemp->getComponent() == "cppTests2" );
         issea( pTemp->getKey() == "newMessage2" );
         issea( pTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      pTempStep = dynamic_cast<Step*>( pTemp );
      if( pTempStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty3" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 3 );
         issea( pTempStep->getStringId() == "6" );
         issea( pTempStep->getAction() == "This is Step Number 3." );
         issea( pTempStep->getComponent() == "cppTests3" );
         issea( pTempStep->getKey() == "newStep3" );
         issea( pTempStep->getResult() == Message::Success );
         issea( pTempStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      if( pTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty3" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 3 );
         issea( pTemp->getStringId() == "7" );
         issea( pTemp->getAction() == "This is Message Number 3." );
         issea( pTemp->getComponent() == "cppTests3" );
         issea( pTemp->getKey() == "newMessage3" );
         issea( pTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      pTempStep = dynamic_cast<Step*>( pTemp );
      if( pTempStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty4" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 4 );
         issea( pTempStep->getStringId() == "8" );
         issea( pTempStep->getAction() == "This is Step Number 4." );
         issea( pTempStep->getComponent() == "cppTests4" );
         issea( pTempStep->getKey() == "newStep4" );
         issea( pTempStep->getResult() == Message::Success );
         issea( pTempStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      if( pTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty4" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 4 );
         issea( pTemp->getStringId() == "9" );
         issea( pTemp->getAction() == "This is Message Number 4." );
         issea( pTemp->getComponent() == "cppTests4" );
         issea( pTemp->getKey() == "newMessage4" );
         issea( pTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      pTempStep = dynamic_cast<Step*>( pTemp );
      if( pTempStep != NULL )
      {
         char myVal = 42;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty5" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 5 );
         issea( pTempStep->getStringId() == "10" );
         issea( pTempStep->getAction() == "This is Step Number 5." );
         issea( pTempStep->getComponent() == "cppTests5" );
         issea( pTempStep->getKey() == "newStep5" );
         issea( pTempStep->getResult() == Message::Success );
         issea( pTempStep->propertyToString( "char", pVoidPtr ) == "42" );
      }
      myItr++;
      pTemp = *myItr;
      entryCount++;

      if( pTemp != NULL )
      {
         unsigned char myVal = 93;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty5" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 5 );
         issea( pTemp->getStringId() == "11" );
         issea( pTemp->getAction() == "This is Message Number 5." );
         issea( pTemp->getComponent() == "cppTests5" );
         issea( pTemp->getKey() == "newMessage5" );
         issea( pTemp->propertyToString( "unsigned char", pVoidPtr ) == "93" );
      }
      myItr++;
      entryCount++;

      issea( myItr == pLog->end() );
      issea( entryCount == 11 );




      // verify the contents of the log using a const iterator
      MessageLog::const_iterator myConstItr = pLog->begin();
      entryCount = 0;
      Message *pConstTemp = *myConstItr;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 0 );
         issea( pConstTemp->getStringId() == "1" );
         issea( pConstTemp->getAction() == "Log Opened" );
         issea( pConstTemp->getComponent() == "app" );
         issea( pConstTemp->getKey() == "EC355E3E-03CA-4081-9006-5F45D6A488B3" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      Step *pTempConstStep = dynamic_cast<Step*>( pConstTemp );
      if( pTempConstStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempConstStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty1" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 1 );
         issea( pTempConstStep->getStringId() == "2" );
         issea( pTempConstStep->getAction() == "This is Step Number 1." );
         issea( pTempConstStep->getComponent() == "cppTests1" );
         issea( pTempConstStep->getKey() == "newStep1" );
         issea( pTempConstStep->getResult() == Message::Success );
         issea( pTempConstStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty1" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 1 );
         issea( pConstTemp->getStringId() == "3" );
         issea( pConstTemp->getAction() == "This is Message Number 1." );
         issea( pConstTemp->getComponent() == "cppTests1" );
         issea( pConstTemp->getKey() == "newMessage1" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      pTempConstStep = dynamic_cast<Step*>( pConstTemp );
      if( pTempConstStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempConstStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty2" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 2 );
         issea( pTempConstStep->getStringId() == "4" );
         issea( pTempConstStep->getAction() == "This is Step Number 2." );
         issea( pTempConstStep->getComponent() == "cppTests2" );
         issea( pTempConstStep->getKey() == "newStep2" );
         issea( pTempConstStep->getResult() == Message::Success );
         issea( pTempConstStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty2" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 2 );
         issea( pConstTemp->getStringId() == "5" );
         issea( pConstTemp->getAction() == "This is Message Number 2." );
         issea( pConstTemp->getComponent() == "cppTests2" );
         issea( pConstTemp->getKey() == "newMessage2" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      pTempConstStep = dynamic_cast<Step*>( pConstTemp );
      if( pTempConstStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempConstStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty3" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 3 );
         issea( pTempConstStep->getStringId() == "6" );
         issea( pTempConstStep->getAction() == "This is Step Number 3." );
         issea( pTempConstStep->getComponent() == "cppTests3" );
         issea( pTempConstStep->getKey() == "newStep3" );
         issea( pTempConstStep->getResult() == Message::Success );
         issea( pTempConstStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty3" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 3 );
         issea( pConstTemp->getStringId() == "7" );
         issea( pConstTemp->getAction() == "This is Message Number 3." );
         issea( pConstTemp->getComponent() == "cppTests3" );
         issea( pConstTemp->getKey() == "newMessage3" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      pTempConstStep = dynamic_cast<Step*>( pConstTemp );
      if( pTempConstStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempConstStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty4" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 4 );
         issea( pTempConstStep->getStringId() == "8" );
         issea( pTempConstStep->getAction() == "This is Step Number 4." );
         issea( pTempConstStep->getComponent() == "cppTests4" );
         issea( pTempConstStep->getKey() == "newStep4" );
         issea( pTempConstStep->getResult() == Message::Success );
         issea( pTempConstStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty4" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 4 );
         issea( pConstTemp->getStringId() == "9" );
         issea( pConstTemp->getAction() == "This is Message Number 4." );
         issea( pConstTemp->getComponent() == "cppTests4" );
         issea( pConstTemp->getKey() == "newMessage4" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      pTempConstStep = dynamic_cast<Step*>( pConstTemp );
      if( pTempConstStep != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pTempConstStep->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "stepProperty5" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 5 );
         issea( pTempConstStep->getStringId() == "10" );
         issea( pTempConstStep->getAction() == "This is Step Number 5." );
         issea( pTempConstStep->getComponent() == "cppTests5" );
         issea( pTempConstStep->getKey() == "newStep5" );
         issea( pTempConstStep->getResult() == Message::Success );
         issea( pTempConstStep->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      pConstTemp = *myConstItr;
      entryCount++;

      if( pConstTemp != NULL )
      {
         double myVal = 939393.14159;
         void *pVoidPtr = &myVal;
         const DynamicObject *pProperties = pConstTemp->getProperties();
         vector<string> attributeNames;
         pProperties->getAttributeNames( attributeNames );
         issea( attributeNames.size() == 1 );
         issea( attributeNames.front() == "MessageProperty5" );
         DataVariant var = pProperties->getAttribute( attributeNames[0] );
         issea( var.isValid() && *var.getPointerToValue<int>() == 5 );
         issea( pConstTemp->getStringId() == "11" );
         issea( pConstTemp->getAction() == "This is Message Number 5." );
         issea( pConstTemp->getComponent() == "cppTests5" );
         issea( pConstTemp->getKey() == "newMessage5" );
         issea( pConstTemp->propertyToString( "double", pVoidPtr ) == "939393.14159" );
      }
      myConstItr++;
      entryCount++;

      issea( myConstItr == pLog->end() );
      issea( entryCount == 11 );

      return success;
   }
};

class MessageSubjectObserverTest : public TestCase
{
public:
   MessageSubjectObserverTest() : TestCase("SubjectObserver") {}
   bool run()
   {
      bool success = true;
      string logName = "cppTestLog";
      bool ok = false;

      Service<MessageLogMgr> pLogMgr;
      issearf(pLogMgr.get() != NULL);

      MessageLog* pLog = pLogMgr->getLog(logName);
      if (pLog == NULL)
      {
         pLog = pLogMgr->createLog(logName);
      }

      issearf(pLog != NULL);

      Message *pMessage = NULL;
      pMessage = pLog->createMessage( "This is the SubjectObserver test case.", "cppTests",
         "74C3D023-FFB2-4D9C-BB26-29C8EB5B3AB3" );
      issea( pMessage != NULL );
      pMessage->finalize();

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>( ModelServicesImp::instance()->createElement( "testAoi", "AoiElement", pRasterElement ) );
      issea( pAoi != NULL );

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "testAoi" ) );
      issea( pProperties != NULL );

      GraphicObject *pRect = pAoi->getGroup()->addObject( RECTANGLE_OBJECT );
      issea( pRect != NULL && pRect->getGraphicObjectType() == RECTANGLE_OBJECT );
      pRect->setBoundingBox( LocationType( 50, 50 ), LocationType( 60, 70 ) );

      // get all AOIs in the session
      vector<DataElement*> aoiVector;
      aoiVector = ModelServicesImp::instance()->getElements( pRasterElement, "AoiElement" );
      issea( aoiVector.size() > 0 );

      SubjectObserver *pObserver = new SubjectObserver();
      pView->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      pList->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      for( unsigned int i = 0; i < aoiVector.size(); i++ )
      {
         aoiVector.at( i )->attach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      }

      pView->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      pList->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      for( unsigned int i = 0; i < aoiVector.size(); i++ )
      {
         aoiVector.at( i )->detach( SIGNAL_NAME(Subject, Modified), Slot(pObserver,  &SubjectObserver::update));
      }

      return success;
   }
};

class MessageToXmlTest : public TestCase
{
public:
   MessageToXmlTest() : TestCase("ToXml") {}
   bool run()
   {
      bool success = true;

      {
         XMLWriter* pWriter = new XMLWriter("test-serialize");
         StepResource step("Test Step", "TEST-COMPONENT", "TEST-KEY", "", "TEST-LOG");
         string stepDate, stepTime;
         step->serializeDate(stepDate, stepTime);
         step->toXml(pWriter);
         string firstSerialize = pWriter->writeToString();
         string firstSerializeStandard = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<test-serialize xmlns=\"https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd\">\n\n  <step component=\"TEST-COMPONENT\" date=\"" + stepDate + "\" id=\"2\" key=\"TEST-KEY\" name=\"Test Step\" result=\"Unresolved\" time=\"" + stepTime + "\"/>\n\n</test-serialize>\n";
         issea(firstSerialize == firstSerializeStandard);
         delete pWriter;

         pWriter = new XMLWriter("test-serialize");
         XMLWriter* pWriter2 = new XMLWriter("test-serialize");
         step->addProperty("test-prop", 3);
         vector<string> testProp;
         testProp.push_back("str1");
         testProp.push_back("str2");
         step->addProperty("test-prop2", testProp);
         step->addProperty("test-prop3", "A multi-line \n value with \" < > special chars");
         step->finalize(Message::Success);
         step->toXml(pWriter);
         step->toXml(pWriter2);
         string secondSerializeStandard = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<test-serialize xmlns=\"https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd\">\n\n  <step component=\"TEST-COMPONENT\" date=\"" + stepDate + "\" id=\"2\" key=\"TEST-KEY\" name=\"Test Step\" result=\"Success\" time=\"" + stepTime + "\">\n    <property name=\"test-prop\" type=\"int\" value=\"3\"/>\n    <property name=\"test-prop2\" type=\"vector&lt;string>\" value=\"str1, str2\"/>\n    <property name=\"test-prop3\" type=\"string\" value=\"A multi-line &#xA; value with &quot; &lt; > special chars\"/>\n  </step>\n\n</test-serialize>\n";
         string secondSerialize = pWriter->writeToString();
         issea(secondSerialize == secondSerializeStandard);
         string thirdSerialize = pWriter2->writeToString();
         issea(secondSerialize == thirdSerialize);
         delete pWriter;
         delete pWriter2;
      }

      {
         XMLWriter* pWriter = new XMLWriter("test-serialize");
         MessageResource msg("Test Message", "TEST-COMPONENT", "TEST-KEY", "TEST-LOG");
         string msgDate, msgTime;
         msg->serializeDate(msgDate, msgTime);
         msg->toXml(pWriter);
         string firstSerialize = pWriter->writeToString();
         string firstSerializeStandard = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<test-serialize xmlns=\"https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd\">\n\n  <message component=\"TEST-COMPONENT\" date=\"" + msgDate + "\" id=\"3\" key=\"TEST-KEY\" name=\"Test Message\" time=\"" + msgTime + "\"/>\n\n</test-serialize>\n";
         issea(firstSerialize == firstSerializeStandard);
         delete pWriter;

         pWriter = new XMLWriter("test-serialize");
         XMLWriter* pWriter2 = new XMLWriter("test-serialize");
         msg->addProperty("test-prop", 3);
         vector<string> testProp;
         testProp.push_back("str1");
         testProp.push_back("str2");
         msg->addProperty("test-prop2", testProp);
         msg->addProperty("test-prop3", "A multi-line \n value with \" < > special chars");
         msg->toXml(pWriter);
         msg->toXml(pWriter2);
         string secondSerializeStandard = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<test-serialize xmlns=\"https://comet.balldayton.com/standards/namespaces/2005/v1/comet.xsd\">\n\n  <message component=\"TEST-COMPONENT\" date=\"" + msgDate + "\" id=\"3\" key=\"TEST-KEY\" name=\"Test Message\" time=\"" + msgTime + "\">\n    <property name=\"test-prop\" type=\"int\" value=\"3\"/>\n    <property name=\"test-prop2\" type=\"vector&lt;string>\" value=\"str1, str2\"/>\n    <property name=\"test-prop3\" type=\"string\" value=\"A multi-line &#xA; value with &quot; &lt; > special chars\"/>\n  </message>\n\n</test-serialize>\n";
         string secondSerialize = pWriter->writeToString();
         issea(secondSerialize == secondSerializeStandard);
         string thirdSerialize = pWriter2->writeToString();
         issea(secondSerialize == thirdSerialize);
         delete pWriter;
         delete pWriter2;
      }

      {
         XMLWriter* pWriter = new XMLWriter("test-serialize");
         XMLWriter* pWriter2 = new XMLWriter("test-serialize");
         StepResource step("Test Step", "TEST-COMPONENT", "TEST-KEY", "", "TEST-LOG");
         step->addProperty("prop1", "TestProp");
         StepResource subStep("Test Sub Step", "TEST-COMPONENT", "TEST-KEY", "", "TEST-LOG");
         subStep->addProperty("prop2", "TestProp2");
         MessageResource msg("Test Message", "TEST-COMPONENT", "TEST-KEY", "TEST-LOG");
         msg->addProperty("prop3", "TestProp3");
         step->toXml(pWriter);
         step->toXml(pWriter2);
         string firstSerialize = pWriter->writeToString();
         string secondSerialize = pWriter2->writeToString();
         cout << firstSerialize << secondSerialize << endl;
         issea(firstSerialize == secondSerialize);
      }

      return success;
   }
};

class MessageLogTestSuite : public TestSuiteNewSession
{
public:
   MessageLogTestSuite() : TestSuiteNewSession( "MessageLog" )
   {
      addTestCase( new MessageLogTest );
      addTestCase( new MessageLogSubscribeTest );
      addTestCase( new MessageLogKeyComponentTest );
      addTestCase( new MessageLogIteratorTest );
      addTestCase( new MessageSubjectObserverTest );
      addTestCase( new MessageToXmlTest );
   }
};

REGISTER_SUITE( MessageLogTestSuite )
