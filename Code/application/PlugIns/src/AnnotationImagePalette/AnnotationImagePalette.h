/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONIMAGEPALETTE_H
#define ANNOTATIONIMAGEPALETTE_H

#include "DockWindowShell.h"

class AnnotationImagePalette : public DockWindowShell
{
public:
   AnnotationImagePalette();
   virtual ~AnnotationImagePalette();
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual QAction* createAction();
   virtual QWidget* createWidget();

private:
   AnnotationImagePalette(const AnnotationImagePalette& rhs);
   AnnotationImagePalette& operator=(const AnnotationImagePalette& rhs);
};

#endif
