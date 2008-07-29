/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Classification.h"
#include "DataDescriptor.h"
#include "MessageLogResource.h"
#include "RasterElement.h"
#include "TiePointListImp.h"

#include <sstream>
using namespace std;

XERCES_CPP_NAMESPACE_USE

TiePointListImp::TiePointListImp(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id)
{
}

TiePointListImp::~TiePointListImp()
{
}

void TiePointListImp::setMissionDatasetName(string missionName)
{
   mMissionName = missionName;
}

const string &TiePointListImp::getMissionDatasetName() const
{
   return mMissionName;
}

LocationType TiePointListImp::toMission(LocationType refPixel) const
{
   return LocationType (0.0,0.0);
}

const vector<TiePoint>& TiePointListImp::getTiePoints() const
{
   return mTiePoints;
}

void TiePointListImp::adoptTiePoints(vector<TiePoint>& points)
{
   if (mTiePoints == points)
   {
      return;
   }

   StepResource pStep("Change TiePointList", "app", "A46AF7DD-A063-494C-BC5C-7FFF41EB9490");
   pStep->addProperty("action", "setPoints");
   pStep->addProperty("name", string(getName()));

   mTiePoints.clear();
   mTiePoints.swap(points);
   notify(SIGNAL_NAME(Subject, Modified));

   pStep->finalize(Message::Success);
}

DataElement* TiePointListImp::copy(const string& name, DataElement* pParent) const
{
   DataElement* pElement = DataElementImp::copy(name, pParent);

   TiePointListImp* pTiePointList = dynamic_cast<TiePointListImp*>(pElement);
   if (pTiePointList != NULL)
   {
      pTiePointList->mMissionName = mMissionName;
      pTiePointList->mTiePoints = mTiePoints;
   }

   return pElement;
}

const string& TiePointListImp::getObjectType() const
{
   static string type("TiePointListImp");
   return type;
}

bool TiePointListImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "TiePointList"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}

bool TiePointListImp::toXml(XMLWriter* pWriter) const
{
   if (pWriter == NULL || DataElementImp::toXml(pWriter) == false)
   {
      return false;
   }

   DOMElement *pDomElement(pWriter->addElement("missionDataset"));
   if (pDomElement == NULL)
   {
      return false;
   }
   pWriter->pushAddPoint(pDomElement);
   pWriter->addAttr("name", XmlBase::PathToURL(getMissionDatasetName()));
   pWriter->popAddPoint();

   char buffer[1024];
   vector<TiePoint>::const_iterator it;
   for (it = mTiePoints.begin(); it!= mTiePoints.end(); it++)
   {
      pDomElement = pWriter->addElement("pt");
      if (pDomElement == NULL)
      {
         return false;
      }
      stringstream buf;
      sprintf(buffer, "%d %d %g %g %d %d", it->mReferencePoint.mX, it->mReferencePoint.mY,
         it->mMissionOffset.mX, it->mMissionOffset.mY, 
         it->mConfidence, it->mPhi);
      pWriter->addText(buffer, pDomElement);
   }
   return true;
}

bool TiePointListImp::fromXml(DOMNode* pNode, unsigned int version)
{
   if (pNode == NULL)
   {
      return false;
   }

   mTiePoints.clear();

   if (DataElementImp::fromXml(pNode, version) == false)
   {
      return false;
   }

   for(DOMNode *chld = pNode->getFirstChild();
                chld != NULL;
                chld = chld->getNextSibling())
   {
      if(XMLString::equals(chld->getNodeName(),X("missionDataset")))
      {
         setMissionDatasetName(XmlBase::URLtoPath(static_cast<DOMElement*>(chld)->getAttribute(X("name"))));
      }
      else if(XMLString::equals(chld->getNodeName(),X("pt")))
      {
         TiePoint point;
         DOMNode *pGchild(chld->getFirstChild());
         XERCES_CPP_NAMESPACE_QUALIFIER XMLStringTokenizer t(pGchild->getNodeValue());
         if(t.hasMoreTokens())
         {
            point.mReferencePoint.mX = atoi(A(t.nextToken()));
         }
         if(t.hasMoreTokens())
         {
            point.mReferencePoint.mY = atoi(A(t.nextToken()));
         }
         if(t.hasMoreTokens())
         {
            point.mMissionOffset.mX = static_cast<float>(atof(A(t.nextToken())));
         }
         if(t.hasMoreTokens())
         {
            point.mMissionOffset.mY = static_cast<float>(atof(A(t.nextToken())));
         }
         if(t.hasMoreTokens())
         {
            point.mConfidence = atoi(A(t.nextToken()));
         }
         if(t.hasMoreTokens())
         {
            point.mPhi = atoi(A(t.nextToken()));
         }
         mTiePoints.push_back (point);
      }
   }

   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

bool TiePointListImp::isKindOfElement(const string& className)
{
   if ((className == "TiePointListImp") || (className == "TiePointList"))
   {
      return true;
   }

   return DataElementImp::isKindOfElement(className);
}

void TiePointListImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("TiePointList");
   DataElementImp::getElementTypes(classList);
}
