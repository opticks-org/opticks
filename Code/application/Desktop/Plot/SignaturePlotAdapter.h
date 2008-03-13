/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREPLOTADAPTER_H
#define SIGNATUREPLOTADAPTER_H

#include "SignaturePlot.h"
#include "SignaturePlotImp.h"

class SignaturePlotAdapter : public SignaturePlot, public SignaturePlotImp SIGNATUREPLOTEXTENSION_CLASSES
{
public:
   SignaturePlotAdapter(const std::string& id, const std::string& viewName, QGLContext* pDrawContext = 0,
      QWidget* pParent = 0);
   ~SignaturePlotAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SIGNATUREPLOTADAPTER_METHODS(SignaturePlotImp)
};

#endif
