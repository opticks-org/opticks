/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignaturePlotAdapter.h"

using namespace std;

SignaturePlotAdapter::SignaturePlotAdapter(const string& id, const string& viewName, QGLContext* pDrawContext,
                                           QWidget* pParent) :
   SignaturePlotImp(id, viewName, pDrawContext, pParent)
{
}

SignaturePlotAdapter::~SignaturePlotAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& SignaturePlotAdapter::getObjectType() const
{
   static string type("SignaturePlotAdapter");
   return type;
}

bool SignaturePlotAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignaturePlot"))
   {
      return true;
   }

   return SignaturePlotImp::isKindOf(className);
}
