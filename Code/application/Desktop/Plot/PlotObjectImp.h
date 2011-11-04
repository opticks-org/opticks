/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTOBJECTIMP_H
#define PLOTOBJECTIMP_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include "LocationType.h"
#include "SerializableImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "xmlwriter.h"

#include "XercesIncludes.h"

class PlotViewImp;

class PlotObjectImp : public QObject, public SubjectImp, public Serializable
{
   Q_OBJECT

public:
   PlotObjectImp(PlotViewImp* pPlot, bool bPrimary);
   ~PlotObjectImp();

   PlotObjectImp& operator= (const PlotObjectImp& object);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   PlotViewImp* getPlot() const;
   virtual PlotObjectType getType() const = 0;
   virtual void draw() = 0;

   QString getObjectName() const;
   bool isVisible() const;
   bool isPrimary() const;
   bool isSelected() const;

   virtual bool hit(LocationType point) const;
   virtual bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY);
   virtual const QPixmap& getLegendPixmap(bool bSelected) const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void setObjectName(const QString& strObjectName);
   virtual void setVisible(bool bVisible);
   virtual void setSelected(bool bSelect);

signals:
   void legendPixmapChanged();
   void visibilityChanged(bool bDisplayed);
   void selected(bool bSelected);
   void renamed(const QString& strNewName);
   void extentsChanged();

private:
   PlotObjectImp(const PlotObjectImp& rhs);

   PlotViewImp* mpPlot;
   QString mName;
   bool mbVisible;
   bool mbPrimary;
   bool mbSelected;
};

#define PLOTOBJECTADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define PLOTOBJECTADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
   void setObjectName(const std::string& objectName) \
   { \
      return impClass::setObjectName(QString::fromStdString(objectName)); \
   } \
   void getObjectName(std::string& objectName) const \
   { \
      objectName = impClass::getObjectName().toStdString(); \
   } \
   PlotObjectType getType() const \
   { \
      return impClass::getType(); \
   } \
   void setVisible(bool bVisible) \
   { \
      return impClass::setVisible(bVisible); \
   } \
   bool isVisible() const \
   { \
      return impClass::isVisible(); \
   } \
   bool isPrimary() const \
   { \
      return impClass::isPrimary(); \
   } \
   void setSelected(bool bSelect) \
   { \
      return impClass::setSelected(bSelect); \
   } \
   bool isSelected() const \
   { \
      return impClass::isSelected(); \
   } \
   bool getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY) \
   { \
      return impClass::getExtents(dMinX, dMinY, dMaxX, dMaxY); \
   }

#endif
