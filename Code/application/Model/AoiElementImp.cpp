/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElementImp.h"
#include "AoiLayer.h"
#include "BitMask.h"
#include "BitMaskObject.h"
#include "BitMaskObjectImp.h"
#include "AppVerify.h"
#include "GraphicGroupImp.h"
#include "GraphicObjectFactory.h"
#include "GraphicLayer.h"
#include "MessageLogResource.h"
#include "PixelObjectImp.h"
#include "RasterElement.h"

#include <list>
using namespace std;

AoiElementImp::AoiElementImp(const DataDescriptorImp& descriptor, const string& id) :
   GraphicElementImp(descriptor, id),
   mBitMaskDirty(true),
   mToggledAllPoints(false)
{}

AoiElementImp::~AoiElementImp()
{}

void AoiElementImp::clearPoints()
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Clear Points");
   pStep->addProperty("name", getName());

   GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
   if (pGroup != NULL)
   {
      pGroup->removeAllObjects(true);
      GraphicLayer* pLayer = pGroup->getLayer();
      if (pLayer != NULL)
      {
         pLayer->deselectAllObjects();
      }
   }

   pStep->finalize(Message::Success);
}

void AoiElementImp::toggleAllPoints()
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Toggle All Points");
   pStep->addProperty("name", getName());

   mToggledAllPoints = !mToggledAllPoints;

   Subject* pGroup = dynamic_cast<Subject*>(getGroup());
   VERIFYNRV(pGroup != NULL);
   groupModified(*pGroup, SIGNAL_NAME(Subject, Modified), boost::any());

   pStep->finalize(Message::Success);
}

ModeType AoiElementImp::correctedDrawMode(ModeType mode)
{
   if (getAllPointsToggled())
   {
      switch (mode)
      {
      case DRAW:
         mode = ERASE;
         break;
      case ERASE:
         mode = DRAW;
         break;
      default:
         break;
      }
   }
   return mode;
}

size_t AoiElementImp::getPixelCount() const
{
   const BitMask* pMask = getSelectedPoints();
   if (pMask != NULL)
   {
      return pMask->getCount();
   }
   return 0;
}

const BitMask* AoiElementImp::getSelectedPoints() const
{
   if (mBitMaskDirty)
   {
      mpBitMask->clear();
      const GraphicGroup* pGroup = getGroup();
      VERIFYRV(pGroup != NULL, NULL);
      const list<GraphicObject*>& objects = pGroup->getObjects();
      for (list<GraphicObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
      {
         GraphicObject* pObj = *iter;
         VERIFYRV(pObj != NULL, NULL);
         const BitMask* pMask = pObj->getPixels();
         BitMaskImp maskDuplicate;
         if (pMask == NULL)
         {
            continue;
         }
         switch (pObj->getDrawMode())
         {
         case DRAW:
            mpBitMask->merge(*pMask);
            break;
         case ERASE:
            maskDuplicate = *static_cast<const BitMaskImp*>(pMask);
            maskDuplicate.invert();
            mpBitMask->intersect(maskDuplicate);
            break;
         case TOGGLE:
            mpBitMask->toggle(*pMask);
            break;
         default:
            break;
         }
      }
      if (mToggledAllPoints)
      {
         mpBitMask->invert();
      }
      mBitMaskDirty = false;
   }

   return mpBitMask.get();
}

bool AoiElementImp::getAllPointsToggled() const
{
   return mToggledAllPoints;
}

GraphicObject* AoiElementImp::addPoints(const vector<LocationType>& points)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Add Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);
   GraphicObject* pObj = pGroup->addObject(MULTIPOINT_OBJECT);

   pObj->setDrawMode(correctedDrawMode(DRAW));
   pObj->addVertices(points);

   pStep->finalize(Message::Success);

   return pObj;
}

GraphicObject* AoiElementImp::addPoint(LocationType point)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Add Point");
   pStep->addProperty("name", getName());

   vector<LocationType> points(1, point);

   pStep->finalize(Message::Success);

   return addPoints(points);
}

GraphicObject* AoiElementImp::addPoints(const BitMask* pPoints)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Add Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);

   BitMaskObject* pMaskObj = static_cast<BitMaskObject*>(pGroup->addObject(BITMASK_OBJECT));

   pMaskObj->setDrawMode(correctedDrawMode(DRAW));
   pMaskObj->setBitMask(pPoints, true);

   pStep->finalize(Message::Success);

   return pMaskObj;
}

GraphicObject* AoiElementImp::removePoints(const vector<LocationType>& points)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Remove Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);

   GraphicObject* pObj = pGroup->addObject(MULTIPOINT_OBJECT);

   pObj->setDrawMode(correctedDrawMode(ERASE));
   pObj->addVertices(points);

   pStep->finalize(Message::Success);

   return pObj;
}

GraphicObject* AoiElementImp::removePoint(LocationType point)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Remove Point");
   pStep->addProperty("name", getName());

   vector<LocationType> points(1, point);

   pStep->finalize(Message::Success);

   return removePoints(points);
}

GraphicObject* AoiElementImp::removePoints(const BitMask* pPoints)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Remove Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);

   BitMaskObject* pMaskObj = static_cast<BitMaskObject*>(pGroup->addObject(BITMASK_OBJECT));

   pMaskObj->setDrawMode(correctedDrawMode(ERASE));
   pMaskObj->setBitMask(pPoints, true);

   pStep->finalize(Message::Success);

   return pMaskObj;
}

GraphicObject* AoiElementImp::togglePoints(const vector<LocationType>& points)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Toggle Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);

   GraphicObject* pObj = pGroup->addObject(MULTIPOINT_OBJECT);

   pObj->setDrawMode(correctedDrawMode(TOGGLE));
   pObj->addVertices(points);

   pStep->finalize(Message::Success);

   return pObj;
}

GraphicObject* AoiElementImp::togglePoint(LocationType point)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Toggle Point");
   pStep->addProperty("name", getName());

   vector<LocationType> points(1, point);

   pStep->finalize(Message::Success);

   return togglePoints(points);
}

GraphicObject* AoiElementImp::togglePoints(const BitMask* pPoints)
{
   StepResource pStep("ChangeAOI", "app", "1AB6C122-2652-48B6-9421-AEFACAFD658C");
   pStep->addProperty("action", "Toggle Points");
   pStep->addProperty("name", getName());

   GraphicGroup* pGroup = getGroup();
   VERIFYRV(pGroup != NULL, NULL);

   BitMaskObject* pMaskObj = static_cast<BitMaskObject*>(pGroup->addObject(BITMASK_OBJECT));

   pMaskObj->setDrawMode(correctedDrawMode(TOGGLE));
   pMaskObj->setBitMask(pPoints, true);
   
   pStep->finalize(Message::Success);

   return pMaskObj;
}

DataElement* AoiElementImp::copy(const string& name, DataElement* pParent) const
{
   DataElement* pElement = GraphicElementImp::copy(name, pParent);

   AoiElementImp* pAoi = dynamic_cast<AoiElementImp*>(pElement);
   if (pAoi != NULL)
   {
      pAoi->mToggledAllPoints = mToggledAllPoints;
      pAoi->mBitMaskDirty = true;
   }

   return pElement;
}

void AoiElementImp::groupModified(Subject& subject, const string& signal, const boost::any& data)
{
   if (&subject == dynamic_cast<Subject*>(getGroup()))
   {
      mBitMaskDirty = true;
   }

   GraphicElementImp::groupModified(subject, signal, data);
}

const string& AoiElementImp::getObjectType() const
{
   static string sType("AoiElementImp");
   return sType;
}

bool AoiElementImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AoiElement"))
   {
      return true;
   }

   return GraphicElementImp::isKindOf(className);
}

void AoiElementImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("AoiElement");
   GraphicElementImp::getElementTypes(classList);
}

bool AoiElementImp::isKindOfElement(const string& className)
{
   if ((className == "AoiElementImp") || (className == "AoiElement"))
   {
      return true;
   }

   return GraphicElementImp::isKindOfElement(className);
}

bool AoiElementImp::setGeocentric(bool geocentric)
{
   // Geocentric is not supported on AoiElements.
   return false;
}
