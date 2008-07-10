/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PSEUDOCOLORCLASS_H
#define PSEUDOCOLORCLASS_H

#include <QtCore/QObject>
#include <QtGui/QColor>

class PseudocolorLayer;

class PseudocolorClass : public QObject
{
   Q_OBJECT

public:
   PseudocolorClass(PseudocolorLayer* pLayer);
   PseudocolorClass(PseudocolorClass& rhs);
   ~PseudocolorClass();

   void setLayer(PseudocolorLayer* pLayer);
   void setProperties(const QString& strName, int iValue, const QColor& clrClass, bool bDisplayed);

   int getValue() const;
   QString getName() const;
   QColor getColor() const;
   bool isDisplayed() const;

public slots:
   void setValue(int iValue);
   void setClassName(const QString& strName);
   void setColor(const QColor& clrClass);
   void setDisplayed(bool bDisplay);

signals:
   void valueChanged(int iValue);
   void nameChanged(const QString& strName);
   void colorChanged(const QColor& clrClass);
   void displayStateChanged(bool bDisplay);

private:
   PseudocolorLayer* mpLayer;
   int miValue;
   QString mstrName;
   QColor mColor;
   bool mbDisplayed;
};

#endif
