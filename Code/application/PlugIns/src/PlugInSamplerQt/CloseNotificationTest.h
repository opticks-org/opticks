/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLOSENOTIFICATIONTEST_H
#define CLOSENOTIFICATIONTEST_H

#include "AlgorithmShell.h"
#include "AttachmentPtr.h"
#include <boost/any.hpp>

class ApplicationServices;

class CloseNotificationTest : public AlgorithmShell
{
public:
   CloseNotificationTest();
   ~CloseNotificationTest();

public:
   bool getInputSpecification(PlugInArgList *&pArgList)
   {
      pArgList = NULL;
      return true;
   }
   bool getOutputSpecification(PlugInArgList *&pArgList)
   {
      pArgList = NULL;
      return true;
   }
   bool execute( PlugInArgList *, PlugInArgList * );
   void processSessionClosed(Subject &pSubject, const std::string &signal, const boost::any &data);
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected:
   AttachmentPtr<ApplicationServices> mpAttachment;
};

#endif
