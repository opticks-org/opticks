/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"

#include <errno.h>
#include <sstream>

void LogVerificationErrorProc(const char *pExpression, const char *pFilename, unsigned int line, const char *pMsg)
{
   Service<MessageLogMgr> pMessageLogMgr;

   MessageLog* pLog = pMessageLogMgr->getLog();
   if (pLog != NULL)
   {
      Message* pMessage = pLog->createMessage(std::string("Internal ") + APP_NAME + std::string(" Error"), "app",
         "94B15D95-4EBC-4ccd-9045-2F89D91CD5A7");
      if (pMessage != NULL)
      {
         if (pMsg != NULL)
         {
            pMessage->addProperty("Message", pMsg);
         }
         else
         {
            pMessage->addProperty("Verify", pExpression);
            pMessage->addProperty("File", pFilename);
            pMessage->addProperty("Line", line);
         }
      }
   }

#if !defined(DEBUG)
   // In release mode, check for the presence of the OPTICKS_SHOW_VERIFY environment variable
   const char* pValue = getenv("OPTICKS_SHOW_VERIFY");
   if (pValue != NULL)
   {
#endif
      std::stringstream text;
      if (pMsg != NULL)
      {
         text << pMsg;
      }
      else
      {
         text << "Internal " << APP_NAME << " Error: Verification Failed\n\nVerify(" << pExpression
            << ")\n   File: " << pFilename << "\n   Line: " << line;
      }

      Service<DesktopServices> pDesktop;
      pDesktop->showMessageBox(APP_NAME, text.str());
#if !defined(DEBUG)
   }
#endif
}
