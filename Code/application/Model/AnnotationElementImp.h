/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONELEMENTIMP_H
#define ANNOTATIONELEMENTIMP_H

#include "GraphicElementImp.h"

class AnnotationElementImp : public GraphicElementImp
{
public:
   AnnotationElementImp(const DataDescriptorImp& descriptor, const std::string& id);
   ~AnnotationElementImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static void getElementTypes(std::vector<std::string>& classList);
   static bool isKindOfElement(const std::string& className);

private:
   AnnotationElementImp(const AnnotationElementImp& rhs);
   AnnotationElementImp& operator=(const AnnotationElementImp& rhs);
};

#define ANNOTATIONELEMENTADAPTEREXTENSION_CLASSES \
   GRAPHICELEMENTADAPTEREXTENSION_CLASSES

#define ANNOTATIONELEMENTADAPTER_METHODS(impClass) \
   GRAPHICELEMENTADAPTER_METHODS(impClass)

#endif
