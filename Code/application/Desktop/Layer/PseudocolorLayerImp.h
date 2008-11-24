/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PSEUDOCOLORLAYERIMP_H
#define PSEUDOCOLORLAYERIMP_H

#include <QtCore/QMap>

#include "BitMask.h"
#include "ColorType.h"
#include "LayerImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PseudocolorClass.h"

#include <vector>

class Image;

/**
 * This class displays a RasterElement as an layer. It maintains a pointer to
 * the RasterElement and all of the display attributes, including colors, symbol
 * style and layer names and values.  Only the first band of the RasterElement
 * is used.
 */
class PseudocolorLayerImp : public LayerImp
{
#ifdef CPPTESTS // allow testing of image rendering
   friend class PseudocolorSubsetCreationTest;
#endif

   Q_OBJECT

public:
   PseudocolorLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~PseudocolorLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
   void rasterElementDataModified(Subject& subject, const std::string& signal, const boost::any& v);

   PseudocolorLayerImp& operator= (const PseudocolorLayerImp& pseudocolorLayer);

   LayerType getLayerType() const;
   using LayerImp::setName;

   std::vector<ColorType> getColors() const;
   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);

   PseudocolorClass* addClass();
   PseudocolorClass* addClass(const QString& strClass, int iValue, const QColor& clrClass, bool bDisplayed);
   void insertClass(PseudocolorClass* pClass, int iID = -1);
   PseudocolorClass* getClass(int iValue) const;
   PseudocolorClass* getClassById(int iID) const;
   int getClassID(PseudocolorClass* pClass) const;
   std::vector<PseudocolorClass*> getAllClasses() const;
   unsigned int getClassCount() const;
   bool removeClass(PseudocolorClass* pClass);
   void clear();

   virtual void getBoundingBox(int& x1, int& y1, int& x2, int& y2) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   void setSymbol(SymbolType symbol);
   SymbolType getSymbol() const;

   /**
    * Get the pixels within the threshold as a bitmask.
    *
    * The Layer owns the returned BitMask.
    */
   virtual const BitMask* getSelectedPixels() const;

public slots:
   void reset();

protected:
   bool canRenderAsImage() const;
   std::pair<int, int> getValueRange(bool onlyDisplayed) const;
   void generateImage();
   bool isGpuImageSupported() const;

protected slots:
   void invalidateImage();

private:
   QMap<int, PseudocolorClass*> mClasses;

   SymbolType mSymbol;

   mutable FactoryResource<BitMask> mpMask;
   int mNextID;
   Image* mpImage;
};

#define PSEUDOCOLORLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define PSEUDOCOLORLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   int addClass() \
   { \
      PseudocolorClass* pClass = impClass::addClass(); \
      if (pClass != NULL) \
      { \
         return impClass::getClassID(pClass); \
      } \
      \
      return -1; \
   } \
   int addInitializedClass(const std::string& className, int iValue, const ColorType& classColor, \
      bool bDisplayed = true) \
   { \
      QString strClassName; \
      if (className.empty() == false) \
      { \
         strClassName = QString::fromStdString(className); \
      } \
      \
      QColor clrClass; \
      if (classColor.isValid() == true) \
      { \
         clrClass.setRgb(classColor.mRed, classColor.mGreen, classColor.mBlue); \
      } \
      \
      PseudocolorClass* pClass = impClass::addClass(strClassName, iValue, clrClass, bDisplayed); \
      if (pClass != NULL) \
      { \
         return impClass::getClassID(pClass); \
      } \
      \
      return -1; \
   } \
   void getClassIDs(std::vector<int>& classIds) const \
   { \
      classIds.clear(); \
      \
      std::vector<PseudocolorClass*> classList = impClass::getAllClasses(); \
      for (unsigned int i = 0; i < classList.size(); i++) \
      { \
         PseudocolorClass* pClass = classList[i]; \
         if (pClass != NULL) \
         { \
            int iID = impClass::getClassID(pClass); \
            if (iID != -1) \
            { \
               classIds.push_back(iID); \
            } \
         } \
      } \
   } \
   unsigned int getClassCount() const \
   { \
      return impClass::getClassCount(); \
   } \
   bool removeClass(int iID) \
   { \
      bool bSuccess = false; \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         bSuccess = impClass::removeClass(pClass); \
      } \
      \
      return bSuccess; \
   } \
   void clear() \
   { \
      return impClass::clear(); \
   } \
   bool setClassProperties(int iID, const std::string& className, int iValue, const ColorType& classColor, \
      bool bDisplayed) \
   { \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         QString strClassName; \
         if (className.empty() == false) \
         { \
            strClassName = QString::fromStdString(className); \
         } \
         \
         QColor clrClass; \
         if (classColor.isValid() == true) \
         { \
            clrClass.setRgb(classColor.mRed, classColor.mGreen, classColor.mBlue); \
         } \
         \
         pClass->setProperties(strClassName, iValue, clrClass, bDisplayed); \
         return true; \
      } \
      \
      return false; \
   } \
   bool setClassName(int iID, const std::string& className) \
   { \
      if (className.empty() == true) \
      { \
         return false; \
      } \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         QString strOldName = pClass->getName(); \
         QString strClassName = QString::fromStdString(className); \
         \
         if (strClassName != strOldName) \
         { \
            pClass->setClassName(strClassName); \
            return true; \
         } \
      } \
      \
      return false; \
   } \
   bool getClassName(int iID, std::string& className) const \
   { \
      className.erase(); \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         QString strClassName = pClass->getName(); \
         if (strClassName.isEmpty() == false) \
         { \
            className = strClassName.toStdString(); \
            return true; \
         } \
      } \
      \
      return false; \
   } \
   bool setClassValue(int iID, int iValue) \
   { \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         int iOldValue = pClass->getValue(); \
         if (iValue != iOldValue) \
         { \
            pClass->setValue(iValue); \
            return true; \
         } \
      } \
      \
      return false; \
   } \
   int getClassValue(int iID) const \
   { \
      int iValue = -1; \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         iValue = pClass->getValue(); \
      } \
      \
      return iValue; \
   } \
   bool setClassColor(int iID, const ColorType& classColor) \
   { \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         QColor clrClass; \
         if (classColor.isValid() == true) \
         { \
            clrClass.setRgb(classColor.mRed, classColor.mGreen, classColor.mBlue); \
         } \
         \
         QColor clrOldColor = pClass->getColor(); \
         if (clrClass != clrOldColor) \
         { \
            pClass->setColor(clrClass); \
            return true; \
         } \
      } \
      \
      return false; \
   } \
   ColorType getClassColor(int iID) const \
   { \
      ColorType classColor; \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         QColor clrClass = pClass->getColor(); \
         if (clrClass.isValid() == true) \
         { \
            classColor.mRed = clrClass.red(); \
            classColor.mGreen = clrClass.green(); \
            classColor.mBlue = clrClass.blue(); \
         } \
      } \
      \
      return classColor; \
   } \
   bool setClassDisplayed(int iID, bool bDisplayed) \
   { \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         bool bIsDisplayed = pClass->isDisplayed(); \
         if (bDisplayed != bIsDisplayed) \
         { \
            pClass->setDisplayed(bDisplayed); \
            return true; \
         } \
      } \
      \
      return false; \
   } \
   bool isClassDisplayed(int iID) const \
   { \
      bool bDisplayed = false; \
      \
      PseudocolorClass* pClass = impClass::getClassById(iID); \
      if (pClass != NULL) \
      { \
         bDisplayed = pClass->isDisplayed(); \
      } \
      \
      return bDisplayed; \
   } \
   SymbolType getSymbol() const \
   { \
      return impClass::getSymbol(); \
   } \
   void setSymbol(SymbolType symbol) \
   { \
      return impClass::setSymbol(symbol); \
   }

#endif
