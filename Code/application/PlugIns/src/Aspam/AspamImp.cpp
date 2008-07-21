/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AspamAdapter.h"
#include "AspamImp.h"
#include "MessageLogResource.h"

#include <boost/any.hpp>
#include <string>

using namespace std;

AspamImp::AspamImp()
{
}

AspamImp::~AspamImp()
{
}

void AspamImp::setParagraphA(const Aspam::ParagraphA &val)
{
   mParagraphA = val;
   notify(SIGNAL_NAME(Aspam, ParagraphAModified), boost::any(mParagraphA));
}

void AspamImp::setParagraphB(const Aspam::ParagraphB &val)
{
   mParagraphB = val;
   notify(SIGNAL_NAME(Aspam, ParagraphBModified), boost::any(mParagraphB));
}

void AspamImp::setParagraphC(const Aspam::ParagraphC &val)
{
   mParagraphC = val;
   notify(SIGNAL_NAME(Aspam, ParagraphCModified), boost::any(mParagraphC));
}

void AspamImp::setParagraphD(const Aspam::ParagraphD &val)
{
   mParagraphD = val;
   notify(SIGNAL_NAME(Aspam, ParagraphDModified), boost::any(mParagraphD));
}

void AspamImp::setParagraphE(const Aspam::ParagraphE &val)
{
   mParagraphE = val;
   notify(SIGNAL_NAME(Aspam, ParagraphEModified), boost::any(mParagraphE));
}

void AspamImp::setParagraphF(const Aspam::ParagraphF &val)
{
   mParagraphF = val;
   notify(SIGNAL_NAME(Aspam, ParagraphFModified), boost::any(mParagraphF));
}

void AspamImp::setParagraphG(const Aspam::ParagraphG &val)
{
   mParagraphG = val;
   notify(SIGNAL_NAME(Aspam, ParagraphGModified), boost::any(mParagraphG));
}

void AspamImp::setParagraphH(const Aspam::ParagraphH &val)
{
   mParagraphH = val;
   notify(SIGNAL_NAME(Aspam, ParagraphHModified), boost::any(mParagraphH));
}

void AspamImp::setParagraphI(const Aspam::ParagraphI &val)
{
   mParagraphI = val;
   notify(SIGNAL_NAME(Aspam, ParagraphIModified), boost::any(mParagraphI));
}

void AspamImp::setParagraphJ(const Aspam::ParagraphJ &val)
{
   mParagraphJ = val;
   notify(SIGNAL_NAME(Aspam, ParagraphJModified), boost::any(mParagraphJ));
}

void AspamImp::setParagraphK(const Aspam::ParagraphK &val)
{
   mParagraphK = val;
   notify(SIGNAL_NAME(Aspam, ParagraphKModified), boost::any(mParagraphK));
}

AnyData* AspamImp::copy() const
{
   Aspam *pAspam = dynamic_cast<Aspam*>(new AspamAdapter);
   if(pAspam != NULL)
   {
      pAspam->setParagraphA(mParagraphA);
      pAspam->setParagraphB(mParagraphB);
      pAspam->setParagraphD(mParagraphD);
      pAspam->setParagraphF(mParagraphF);
      pAspam->setParagraphG(mParagraphG);
      pAspam->setParagraphH(mParagraphH);
      pAspam->setParagraphJ(mParagraphJ);
   }

   return pAspam;
}

const string& AspamImp::getObjectType() const
{
   static string type("AspamImp");
   return type;
}

bool AspamImp::isKindOf(const string& className) const
{
   if((className == getObjectType()) || (className == "Aspam"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
