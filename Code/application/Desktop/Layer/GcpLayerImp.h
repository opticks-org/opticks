/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPLAYERIMP_H
#define GCPLAYERIMP_H

#include <QtGui/QColor>

#include "LayerImp.h"
#include "TypesFile.h"

class GcpList;

class GcpLayerImp : public LayerImp
{
   Q_OBJECT

public:
   GcpLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~GcpLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   GcpLayerImp& operator= (const GcpLayerImp& gcpLayer);

   LayerType getLayerType() const;
   using LayerImp::setName;

   std::vector<ColorType> getColors() const;
   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);

   QColor getColor() const;
   GcpSymbol getSymbol() const;
   int getSymbolSize() const;

   bool acceptsMouseEvents() const;
   QCursor getMouseCursor() const;

   bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setColor(const QColor& clrGcp);
   void setSymbol(const GcpSymbol& gcpSymbol);
   void setSymbolSize(int iSize);
   void reset();

signals:
   void colorChanged(const QColor& clrGcp);
   void symbolChanged(const GcpSymbol& gcpSymbol);
   void sizeChanged(int iSize);

private:
   QColor mColor;
   GcpSymbol mSymbol;
   int mSymbolSize;   // Symbol radius in screen pixels
};

#define GCPLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define GCPLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   void setColor(const ColorType& gcpColor) \
   { \
      QColor clrGcp; \
      if (gcpColor.isValid() == true) \
      { \
         clrGcp.setRgb(gcpColor.mRed, gcpColor.mGreen, gcpColor.mBlue); \
      } \
      impClass::setColor(clrGcp);\
   }\
   ColorType getColor() const \
   { \
      ColorType gcpColor; \
      QColor clrGcp = impClass::getColor(); \
      if (clrGcp.isValid() == true) \
      { \
         gcpColor.mRed = clrGcp.red(); \
         gcpColor.mGreen = clrGcp.green(); \
         gcpColor.mBlue = clrGcp.blue(); \
      } \
      return gcpColor; \
   } \
   void setSymbol(const GcpSymbol& gcpSymbol) \
   { \
      impClass::setSymbol(gcpSymbol); \
   } \
   GcpSymbol getSymbol() const \
   { \
      return impClass::getSymbol(); \
   } \
   void setSymbolSize(int iSize) \
   { \
      impClass::setSymbolSize(iSize); \
   } \
   int getSymbolSize() const \
   { \
      return impClass::getSymbolSize(); \
   }

#endif
