/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlotObjectImp.h"
#include "PlotObject.h"
#include "PlotViewImp.h"
#include "xmlreader.h"

#include <sstream>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

PlotObjectImp::PlotObjectImp(PlotViewImp* pPlot, bool bPrimary) :
   mpPlot(pPlot),
   mName(QString()),
   mbVisible(true),
   mbPrimary(bPrimary),
   mbSelected(false)
{
}

PlotObjectImp::~PlotObjectImp()
{
}

PlotObjectImp& PlotObjectImp::operator= (const PlotObjectImp& object)
{
   if (this != &object)
   {
      mName = object.mName;
      mbVisible = object.mbVisible;
      mbPrimary = object.mbPrimary;
      mbSelected = object.mbSelected;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& PlotObjectImp::getObjectType() const
{
   static string type("PlotObjectImp");
   return type;
}

bool PlotObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

PlotViewImp* PlotObjectImp::getPlot() const
{
   return mpPlot;
}

QString PlotObjectImp::getObjectName() const
{
   return mName;
}

bool PlotObjectImp::isVisible() const
{
   return mbVisible;
}

bool PlotObjectImp::isPrimary() const
{
   return mbPrimary;
}

bool PlotObjectImp::isSelected() const
{
   return mbSelected;
}

bool PlotObjectImp::hit(LocationType point) const
{
   return false;
}

bool PlotObjectImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   return false;
}

const QPixmap& PlotObjectImp::getLegendPixmap(bool bSelected) const
{
   // QPixmap must be destroyed before QApplication. This can't be guaranteed with
   // a static object. A heap object will leak but since the lifespan of this object
   // is the life of the application this is ok.
   static QPixmap* spNullPixmap(NULL);
   if (!spNullPixmap)
   {
      spNullPixmap = new QPixmap();
   }
   return *spNullPixmap;
}

void PlotObjectImp::setObjectName(const QString& strObjectName)
{
   if (strObjectName != mName)
   {
      mName = strObjectName;
      emit renamed(mName);
      notify(SIGNAL_NAME(PlotObject, Renamed), boost::any(mName));
   }
}

void PlotObjectImp::setVisible(bool bVisible)
{
   if (bVisible != mbVisible)
   {
      mbVisible = bVisible;
      emit visibilityChanged(mbVisible);
      notify(SIGNAL_NAME(PlotObject, VisibilityChanged), boost::any(mbVisible));
   }
}

void PlotObjectImp::setSelected(bool bSelect)
{
   if (bSelect != mbSelected)
   {
      mbSelected = bSelect;
      emit selected(mbSelected);
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(PlotObject, Selected), boost::any(mbSelected));
   }
}

bool PlotObjectImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", getObjectName().toStdString());
   pXml->addAttr("type", getType());
   pXml->addAttr("visible", isVisible());
   if (isPrimary())
   {
      pXml->addAttr("primary", true);
   }
   pXml->addAttr("selected", isSelected());
   return true;
}

bool PlotObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }
   DOMElement* pElem = static_cast<DOMElement*>(pDocument);

   setObjectName(A(pElem->getAttribute(X("name"))));
   setVisible(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("visible")))));
   setSelected(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("selected")))));
   mbPrimary = pElem->hasAttribute(X("primary"));

   return true;
}
