/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREPLOTIMP_H
#define SIGNATUREPLOTIMP_H

#include <QtGui/QAction>

#include "CartesianPlotImp.h"
#include "TypesFile.h"

class SignaturePlotImp : public CartesianPlotImp
{
   Q_OBJECT

public:
   SignaturePlotImp(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~SignaturePlotImp();

   using SessionItemImp::setIcon;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfView(const std::string& className);
   static void getViewTypes(std::vector<std::string>& classList);

   SignaturePlotImp& operator= (const SignaturePlotImp& cartesianPlot);
   View* copy(QGLContext* pDrawContext = 0, QWidget* pParent = 0) const;
   bool copy(View *pView) const;
   using ViewImp::setName;

   PlotType getPlotType() const;
};

#define SIGNATUREPLOTADAPTEREXTENSION_CLASSES \
   CARTESIANPLOTADAPTEREXTENSION_CLASSES

#define SIGNATUREPLOTADAPTER_METHODS(impClass) \
   CARTESIANPLOTADAPTER_METHODS(impClass)

#endif
