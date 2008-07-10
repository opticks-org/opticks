/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
  * The documentation for this class is in GcpList.h
  */
#include "AppConfig.h"
#include "GcpListImp.h"
#include "RasterElement.h"
#include "MessageLogResource.h"

#include <algorithm>
#include <boost/any.hpp>
#include <iterator>
#include <string>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

GcpListImp::GcpListImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id)
{
}

GcpListImp::~GcpListImp()
{
}

int GcpListImp::getCount() const
{
   int iCount = 0;
   iCount = selected.size();

   return iCount;
}

const list<GcpPoint>& GcpListImp::getSelectedPoints() const
{
   return selected;
}

void GcpListImp::addPoints(const list<GcpPoint>& points)
{
   StepResource pStep("Change GcpList", "app", "47DD0968-DD03-4BE9-B7F3-E1F5440A22B1");
   pStep->addProperty("action", "addPoints");
   pStep->addProperty("name", string(getName()));

   list<GcpPoint>::const_iterator it;
   for (it=points.begin(); it!=points.end(); it++)
   {
      selected.push_back(*it);
   }
   notify(SIGNAL_NAME(GcpList, PointsAdded), boost::any(points));

   pStep->finalize(Message::Success);
}

void GcpListImp::addPoint(const GcpPoint& point)
{
   StepResource pStep("Change GcpList", "app", "B4EB4D13-63E3-47F3-A19D-F88FCB075957");
   pStep->addProperty("action", "addPoint");
   pStep->addProperty("name", string(getName()));

   selected.push_back (point);
   notify(SIGNAL_NAME(GcpList, PointAdded), boost::any(point));

   pStep->finalize(Message::Success);
}

void GcpListImp::removePoints(const list<GcpPoint>& points)
{
   StepResource pStep("Change GcpList", "app", "B18AB1F0-BB98-40B4-B6F1-E3289EEA4D25");
   pStep->addProperty("action", "removePoints");
   pStep->addProperty("name", string(getName()));

   bool changed = false;
   int i = 0;
   int iCount = 0;
   GcpPoint point;
   list<GcpPoint>::iterator it;
   list<GcpPoint>::const_iterator newit;
   for (newit=points.begin(); newit!=points.end(); newit++)
   {
      for (it=selected.begin(); it!=selected.end(); it++)
         if (it->mCoordinate.mX == newit->mCoordinate.mX &&
             it->mCoordinate.mY == newit->mCoordinate.mY &&
             it->mPixel.mX == newit->mPixel.mX &&
             it->mPixel.mY == newit->mPixel.mY)
            break;
//      it = find (selected.begin(), selected.end(), *newit);
      if (it != selected.end())
      {
         point = *it;
         selected.erase (it);
         changed =  true;
      }
   }
   if (changed) 
   {
      notify(SIGNAL_NAME(GcpList, PointsRemoved), boost::any(points));
   }

   pStep->finalize(Message::Success);
}

void GcpListImp::removePoint(const GcpPoint& point)
{
   StepResource pStep("Change GcpList", "app", "D6AC526B-1646-40E2-B3AF-1276909F1B9E");
   pStep->addProperty("action", "removePoint");
   pStep->addProperty("name", string(getName()));

   list<GcpPoint>::iterator it;
   for (it=selected.begin(); it!=selected.end(); it++)
      if (it->mCoordinate.mX == point.mCoordinate.mX &&
          it->mCoordinate.mY == point.mCoordinate.mY &&
          it->mPixel.mX == point.mPixel.mX &&
          it->mPixel.mY == point.mPixel.mY)
         break;
   if (it != selected.end())
   {
      selected.erase(it);
      notify(SIGNAL_NAME(GcpList, PointRemoved), boost::any(point));
   }

   pStep->finalize(Message::Success);
}

void GcpListImp::clearPoints()
{
   StepResource pStep("Change Gcp List", "app", "9C141193-5985-43E4-A9EA-FB1B1626301A");
   pStep->addProperty("action", "clearPoints");
   pStep->addProperty("name", string(getName()));

   selected.clear();
   notify(SIGNAL_NAME(GcpList, Cleared), boost::any());

   pStep->finalize(Message::Success);
}

DataElement* GcpListImp::copy(const string& name, DataElement* pParent) const
{
   DataElement* pElement = DataElementImp::copy(name, pParent);

   GcpListImp* pGcpList = dynamic_cast<GcpListImp*>(pElement);
   if (pGcpList != NULL)
   {
      pGcpList->selected = selected;
   }

   return pElement;
}

bool GcpListImp::toXml(XMLWriter* pXml) const
{
   if(!DataElementImp::toXml(pXml))
   {
      return false;
   }
   return gcpsToXml(selected.begin(), selected.end(), pXml);
}

bool GcpListImp::gcpsToXml(list<GcpPoint>::const_iterator first, list<GcpPoint>::const_iterator last, XMLWriter* pXml)
{
   for(list<GcpPoint>::const_iterator it = first; it != last; ++it)
   {
      pXml->pushAddPoint(pXml->addElement("gcpPoint"));

      stringstream buf;
      buf.precision(12);

      pXml->pushAddPoint(pXml->addElement("pixel"));
      buf << (*it).mPixel.mX << ' ' << (*it).mPixel.mY;
      pXml->addText(buf.str().c_str());
      pXml->popAddPoint();

      pXml->pushAddPoint(pXml->addElement("coordinate"));
      buf.str("");
      buf << (*it).mCoordinate.mX << ' ' << (*it).mCoordinate.mY;
      pXml->addText(buf.str().c_str());
      pXml->popAddPoint();

      pXml->pushAddPoint(pXml->addElement("rmsError"));
      buf.str("");
      buf << (*it).mRmsError.mX << ' ' << (*it).mRmsError.mY;
      pXml->addText(buf.str().c_str());
      pXml->popAddPoint();

      pXml->popAddPoint();
   }
   return true;
}

bool GcpListImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DataElementImp::fromXml(pDocument, version);
   bool success = xmlToGcps(back_insert_iterator<list<GcpPoint> >(selected), pDocument, version);
   notify(SIGNAL_NAME(Subject, Modified));
   return success;
}

bool GcpListImp::xmlToGcps(back_insert_iterator<list<GcpPoint> > first, DOMNode* pDoc, unsigned int version)
{
   for(DOMNode *pChld = pDoc->getFirstChild();
                pChld != NULL;
                pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("gcpPoint")))
      {
         GcpPoint point;
         for(DOMNode *pGchld = pChld->getFirstChild();
                      pGchld != NULL;
                      pGchld = pGchld->getNextSibling())
         {
            if(XMLString::equals(pGchld->getNodeName(), X("pixel")))
            {
               DOMNode *pGgchld(pGchld->getFirstChild());
               XmlReader::StrToLocation(pGgchld->getNodeValue(), point.mPixel);
            }
            else if(XMLString::equals(pGchld->getNodeName(), X("coordinate")))
            {
               DOMNode *pGgchld(pGchld->getFirstChild());
               XmlReader::StrToLocation(pGgchld->getNodeValue(), point.mCoordinate);
            }
            else if(XMLString::equals(pGchld->getNodeName(), X("rmsError")))
            {
               DOMNode *pGgchld(pGchld->getFirstChild());
               XmlReader::StrToLocation(pGgchld->getNodeValue(), point.mRmsError);
            }
         }
         *first++ = point;
      }
   }
   return true;
}

const string& GcpListImp::getObjectType() const
{
   static string type("GcpListImp");
   return type;
}

bool GcpListImp::isKindOf(const string& className) const
{
   if (className == getObjectType() || (className == "GcpList"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool GcpListImp::isKindOfElement(const string& className)
{
   if ((className == "GcpListImp") || (className == "GcpList"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void GcpListImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("GcpList");
   DataElementImp::getElementTypes(classList);
}
