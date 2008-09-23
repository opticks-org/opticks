/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>

#include "CurveCollection.h"
#include "CurveCollectionImp.h"
#include "CurveAdapter.h"
#include "PlotView.h"
#include "xmlreader.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

CurveCollectionImp::CurveCollectionImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mColor(Qt::black),
   mLineWidth(1),
   mLineStyle(SOLID_LINE)
{
   connect(this, SIGNAL(curveAdded(Curve*)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(pointsChanged()), this, SIGNAL(extentsChanged()));
}

CurveCollectionImp::~CurveCollectionImp()
{
}

CurveCollectionImp& CurveCollectionImp::operator= (const CurveCollectionImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mCurves.clear();

      vector<Curve*>::const_iterator iter = object.mCurves.begin();
      while (iter != object.mCurves.end())
      {
         Curve* pCurve = NULL;
         pCurve = *iter;
         if (pCurve != NULL)
         {
            Curve* pNewCurve = NULL;
            pNewCurve = addCurve();
            if (pNewCurve != NULL)
            {
               *(static_cast<CurveAdapter*> (pNewCurve)) = *(static_cast<CurveAdapter*> (pCurve));
            }
         }

         ++iter;
      }

      mColor = object.mColor;
      mLineWidth = object.mLineWidth;
      mLineStyle = object.mLineStyle;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& CurveCollectionImp::getObjectType() const
{
   static string type("CurveCollectionImp");
   return type;
}

bool CurveCollectionImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CurveCollection"))
   {
      return true;
   }

   return PlotObjectImp::isKindOf(className);
}

PlotObjectType CurveCollectionImp::getType() const
{
   return CURVE_COLLECTION;
}

void CurveCollectionImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   for (unsigned int i = 0; i < mCurves.size(); i++)
   {
      Curve* pCurve = NULL;
      pCurve = mCurves.at(i);
      if (pCurve != NULL)
      {
         ((CurveAdapter*) pCurve)->draw();
      }
   }
}

Curve* CurveCollectionImp::addCurve()
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return NULL;
   }

   bool bPrimary = false;
   bPrimary = isPrimary();

   Curve* pCurve = NULL;
   pCurve = new CurveAdapter(pPlot, bPrimary);
   if (pCurve != NULL)
   {
      ((CurveAdapter*) pCurve)->CurveImp::setColor(mColor);
      pCurve->setLineWidth(mLineWidth);
      pCurve->setLineStyle(mLineStyle);
      insertCurve(pCurve);
   }

   return pCurve;
}

bool CurveCollectionImp::insertCurve(Curve* pCurve)
{
   if (pCurve == NULL)
   {
      return false;
   }

   // Do not insert the curve if it is already in the vector
   vector<Curve*>::iterator iter = mCurves.begin();
   while (iter != mCurves.end())
   {
      Curve* pCurrentCurve = NULL;
      pCurrentCurve = *iter;
      if (pCurrentCurve == pCurve)
      {
         return false;
      }

      ++iter;
   }

   connect((CurveAdapter*) pCurve, SIGNAL(pointsChanged(const std::vector<LocationType>&)), this,
      SIGNAL(pointsChanged()));
   mCurves.push_back(pCurve);
   emit curveAdded(pCurve);
   notify(SIGNAL_NAME(CurveCollection, CurveAdded), boost::any(pCurve));

   return true;
}

const vector<Curve*>& CurveCollectionImp::getCurves() const
{
   return mCurves;
}

unsigned int CurveCollectionImp::getNumCurves() const
{
   return mCurves.size();
}

bool CurveCollectionImp::deleteCurve(Curve* pCurve)
{
   if (pCurve == NULL)
   {
      return false;
   }

   vector<Curve*>::iterator iter = mCurves.begin();
   while (iter != mCurves.end())
   {
      Curve* pCurrentCurve = NULL;
      pCurrentCurve = *iter;
      if (pCurrentCurve == pCurve)
      {
         mCurves.erase(iter);
         emit curveDeleted(pCurve);
         notify(SIGNAL_NAME(CurveCollection, CurveDeleted), boost::any(pCurve));
         delete (CurveAdapter*) pCurrentCurve;
         return true;
      }

      ++iter;
   }

   return false;
}

void CurveCollectionImp::clear()
{
   vector<Curve*>::iterator iter = mCurves.begin();
   while (iter != mCurves.end())
   {
      Curve* pCurve = NULL;
      pCurve = *iter;
      if (pCurve != NULL)
      {
         deleteCurve(pCurve);
      }

      iter = mCurves.begin();
   }
}

QColor CurveCollectionImp::getColor() const
{
   return mColor;
}

int CurveCollectionImp::getLineWidth() const
{
   return mLineWidth;
}

LineStyle CurveCollectionImp::getLineStyle() const
{
   return mLineStyle;
}

bool CurveCollectionImp::hit(LocationType point) const
{
   unsigned int numCurves = mCurves.size();
   for (unsigned int i = 0; i < numCurves; i++)
   {
      Curve* pCurve = NULL;
      pCurve = mCurves.at(i);
      if (pCurve != NULL)
      {
         bool bHit = false;
         bHit = ((CurveAdapter*) pCurve)->hit(point);
         if (bHit == true)
         {
            return true;
         }
      }
   }

   return false;
}

bool CurveCollectionImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   unsigned int numCurves = mCurves.size();
   if (numCurves == 0)
   {
      return false;
   }

   dMinX = 1e38;
   dMinY = 1e38;
   dMaxX = -1e38;
   dMaxY = -1e38;

   for (unsigned int i = 0; i < numCurves; i++)
   {
      Curve* pCurve = NULL;
      pCurve = mCurves.at(i);
      if (pCurve != NULL)
      {
         double dCurrentMinX = 0.0;
         double dCurrentMinY = 0.0;
         double dCurrentMaxX = 0.0;
         double dCurrentMaxY = 0.0;

         bool bSuccess = false;
         bSuccess = pCurve->getExtents(dCurrentMinX, dCurrentMinY, dCurrentMaxX, dCurrentMaxY);
         if (bSuccess == false)
         {
            dMinX = -1.0;
            dMinY = -1.0;
            dMaxX = 1.0;
            dMaxY = 1.0;

            return false;
         }

         if (dCurrentMinX < dMinX)
         {
            dMinX = dCurrentMinX;
         }

         if (dCurrentMinY < dMinY)
         {
            dMinY = dCurrentMinY;
         }

         if (dCurrentMaxX > dMaxX)
         {
            dMaxX = dCurrentMaxX;
         }

         if (dCurrentMaxY > dMaxY)
         {
            dMaxY = dCurrentMaxY;
         }
      }
   }

   return true;
}

const QPixmap& CurveCollectionImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QPixmap selectedPix(25, 15);
   static QColor pixColor;
   static QColor selectedPixColor;

   QColor currentColor = getColor();
   if (currentColor.isValid() == false)
   {
      currentColor = Qt::black;
   }

   if ((bSelected == true) && (selectedPix.isNull() == false))
   {
      if (selectedPixColor != currentColor)
      {
         selectedPixColor = currentColor;
         selectedPix.fill(Qt::white);

         QRect rcPixmap = selectedPix.rect();

         QPolygon points(4);
         points.setPoint(0, rcPixmap.center().x() - 4, rcPixmap.center().y());
         points.setPoint(1, rcPixmap.center().x(), rcPixmap.center().y() + 4);
         points.setPoint(2, rcPixmap.center().x() + 4, rcPixmap.center().y());
         points.setPoint(3, rcPixmap.center().x(), rcPixmap.center().y() - 4);

         QPainter p(&selectedPix);
         p.setPen(QPen(currentColor, 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.setBrush(Qt::black);
         p.setPen(QPen(Qt::black, 1));
         p.drawPolygon(points);
         p.end();
      }

      return selectedPix;
   }
   else if ((bSelected == false) && (pix.isNull() == false))
   {
      if (pixColor != currentColor)
      {
         pixColor = currentColor;
         pix.fill(Qt::white);

         QRect rcPixmap = pix.rect();

         QPainter p(&pix);
         p.setPen(QPen(currentColor, 1));
         p.drawLine(rcPixmap.left() + 2, rcPixmap.center().y(), rcPixmap.right() - 2, rcPixmap.center().y());
         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void CurveCollectionImp::setSelected(bool bSelect)
{
   if (isSelected() != bSelect)
   {
      unsigned int numCurves = mCurves.size();
      for (unsigned int i = 0; i < numCurves; i++)
      {
         Curve* pCurve = NULL;
         pCurve = mCurves.at(i);
         if (pCurve != NULL)
         {
            pCurve->setSelected(bSelect);
         }
      }
   }

   PlotObjectImp::setSelected(bSelect);
}

void CurveCollectionImp::setColor(const QColor& clrCurve)
{
   if (clrCurve.isValid() == false)
   {
      return;
   }

   if (clrCurve != mColor)
   {
      mColor = clrCurve;

      unsigned int numCurves = mCurves.size();
      for (unsigned int i = 0; i < numCurves; i++)
      {
         Curve* pCurve = NULL;
         pCurve = mCurves.at(i);
         if (pCurve != NULL)
         {
            ((CurveAdapter*) pCurve)->CurveImp::setColor(mColor);
         }
      }

      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void CurveCollectionImp::setLineWidth(int iWidth)
{
   if (iWidth < 1)
   {
      iWidth = 1;
   }

   if (iWidth != mLineWidth)
   {
      mLineWidth = iWidth;

      unsigned int numCurves = mCurves.size();
      for (unsigned int i = 0; i < numCurves; i++)
      {
         Curve* pCurve = NULL;
         pCurve = mCurves.at(i);
         if (pCurve != NULL)
         {
            pCurve->setLineWidth(iWidth);
         }
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void CurveCollectionImp::setLineStyle(LineStyle eStyle)
{
   if (eStyle != mLineStyle)
   {
      mLineStyle = eStyle;

      unsigned int numCurves = mCurves.size();
      for (unsigned int i = 0; i < numCurves; i++)
      {
         Curve* pCurve = NULL;
         pCurve = mCurves.at(i);
         if (pCurve != NULL)
         {
            pCurve->setLineStyle(eStyle);
         }
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool CurveCollectionImp::toXml(XMLWriter* pXml) const
{
   if(!PlotObjectImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("lineWidth", mLineWidth);
   pXml->addAttr("lineStyle", mLineStyle);
   for(vector<Curve*>::const_iterator it = mCurves.begin(); it != mCurves.end(); ++it)
   {
      const CurveImp* pCurve = dynamic_cast<CurveImp*>(*it);
      pXml->pushAddPoint(pXml->addElement("Curve"));
      if (pCurve == NULL || !pCurve->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }
   return true;
}

bool CurveCollectionImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if(pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   ColorType color = StringUtilities::fromXmlString<ColorType>(
      A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);
   mLineWidth = StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("lineWidth"))));
   mLineStyle = StringUtilities::fromXmlString<LineStyle>(
      A(pElem->getAttribute(X("lineStyle"))));
   for(DOMNode *pChld = pElem->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if(XMLString::equals(pChld->getNodeName(), X("Curve")))
      {
         CurveImp *pCurve = dynamic_cast<CurveImp*>(addCurve());
         if(pCurve == NULL || !pCurve->fromXml(pChld, version))
         {
            return false;
         }
      }
   }
   return true;
}
