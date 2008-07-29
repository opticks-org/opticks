/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignaturePlotImp.h"
#include "SignaturePlotAdapter.h"
#include "Undo.h"
#include "xmlreader.h"

using namespace std;

SignaturePlotImp::SignaturePlotImp(const string& id, const string& viewName, QGLContext* pDrawContext,
                                   QWidget* pParent) :
   CartesianPlotImp(id, viewName, pDrawContext, pParent)
{
}

SignaturePlotImp::~SignaturePlotImp()
{
}

const string& SignaturePlotImp::getObjectType() const
{
   static string type("SignaturePlotImp");
   return type;
}

bool SignaturePlotImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignaturePlot"))
   {
      return true;
   }

   return CartesianPlotImp::isKindOf(className);
}

bool SignaturePlotImp::isKindOfView(const string& className)
{
   if ((className == "SignaturePlotImp") || (className == "SignaturePlot"))
   {
      return true;
   }

   return CartesianPlotImp::isKindOfView(className);
}

void SignaturePlotImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("SignaturePlot");
   CartesianPlotImp::getViewTypes(classList);
}

SignaturePlotImp& SignaturePlotImp::operator= (const SignaturePlotImp& signaturePlot)
{
   if (this != &signaturePlot)
   {
      CartesianPlotImp::operator= (signaturePlot);
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

View* SignaturePlotImp::copy(QGLContext* pDrawContext, QWidget* pParent) const
{
   string viewName = getName();

   SignaturePlotAdapter* pView = new SignaturePlotAdapter(SessionItemImp::generateUniqueId(), viewName,
      pDrawContext, pParent);
   if (pView != NULL)
   {
      UndoLock lock(pView);
      *(static_cast<SignaturePlotImp*>(pView)) = *this;
   }

   return pView;
}

bool SignaturePlotImp::copy(View *pView) const
{
   SignaturePlotImp *pViewImp = dynamic_cast<SignaturePlotImp*>(pView);
   if (pViewImp != NULL)
   {
      UndoLock lock(pView);
      *pViewImp = *this;
   }

   return pViewImp != NULL;
}

PlotType SignaturePlotImp::getPlotType() const
{
   return SIGNATURE_PLOT;
}
