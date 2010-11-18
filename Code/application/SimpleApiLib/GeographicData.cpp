/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpList.h"
#include "GeographicData.h"
#include "SimpleApiErrors.h"

extern "C"
{
   uint32_t getGcpCount(DataElement* pList)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(pList);
      if (pGcpList == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pGcpList->getCount();
   }

   struct Gcp getGcpPoint(DataElement* pList, uint32_t index)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(pList);
      if (pGcpList == NULL || index >= static_cast<uint32_t>(pGcpList->getCount()))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return Gcp();
      }
      const std::list<GcpPoint>& points = pGcpList->getSelectedPoints();
      std::list<GcpPoint>::const_iterator point = points.begin();
      while (index-- > 0) ++point;
      struct Gcp rval;
      memcpy(&rval, &*point, sizeof(struct Gcp));
      setLastError(SIMPLE_NO_ERROR);
      return rval;
   }

   uint32_t getGcpPoints(DataElement* pList, struct Gcp* pPoints)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(pList);
      if (pGcpList == NULL || pPoints == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      const std::list<GcpPoint>& points = pGcpList->getSelectedPoints();
      uint32_t index = 0;
      for (std::list<GcpPoint>::const_iterator point = points.begin(); point != points.end(); ++point)
      {
         memcpy(&pPoints[index++], &*point, sizeof(struct Gcp));
      }
      setLastError(SIMPLE_NO_ERROR);
      return points.size();
   }

   uint32_t setGcpPoints(DataElement* pList, uint32_t count, struct Gcp* pPoints)
   {
      GcpList* pGcpList = dynamic_cast<GcpList*>(pList);
      if (pGcpList == NULL || (count > 0 && pPoints == NULL))
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      pGcpList->clearPoints();
      if (count == 0)
      {
         setLastError(SIMPLE_NO_ERROR);
         return 0;
      }
      std::list<GcpPoint> points;
      for (uint32_t index = 0; index < count; ++index)
      {
         GcpPoint point;
         memcpy(&point, &pPoints[index], sizeof(struct Gcp));
         points.push_back(point);
      }
      pGcpList->addPoints(points);
      setLastError(SIMPLE_NO_ERROR);
      return pGcpList->getCount();
   }
}
