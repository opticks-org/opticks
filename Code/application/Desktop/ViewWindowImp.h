/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWWINDOWIMP_H
#define VIEWWINDOWIMP_H

#include <QtGui/QWidget>

#include "WindowImp.h"
#include "TypesFile.h"

#include <string>

class View;

class ViewWindowImp : public WindowImp
{
public:
   ViewWindowImp(const std::string& id, const std::string& windowName);
   ~ViewWindowImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   using WindowImp::setName;

   virtual View* createView(const QString& strViewName, const ViewType& viewType);
   virtual View* getView() const;
   virtual void setWidget(QWidget* pWidget) = 0;
   virtual QWidget* getWidget() const = 0;

   virtual void print(bool bSetupDialog = false);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected:
   void setView(View *pView);

private:
   View* mpView;
};

#define VIEWWINDOWADAPTEREXTENSION_CLASSES \
   WINDOWADAPTEREXTENSION_CLASSES

#define VIEWWINDOWADAPTER_METHODS(impClass) \
   WINDOWADAPTER_METHODS(impClass) \
   View* createView(const std::string& viewName, const ViewType& viewType) \
   { \
      return impClass::createView(QString::fromStdString(viewName), viewType); \
   } \
   View* getView() const \
   { \
      return impClass::getView(); \
   } \
   void setWidget(QWidget* pWidget) \
   { \
      impClass::setWidget(pWidget); \
   } \
   QWidget* getWidget() const \
   { \
      return impClass::getWidget(); \
   } \
   void print(bool bSetupDialog = false) \
   { \
      impClass::print(bSetupDialog); \
   }

#endif
