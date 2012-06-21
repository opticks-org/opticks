/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVOLUTIONMATRIXEDITOR_H
#define CONVOLUTIONMATRIXEDITOR_H

#include "DockWindowShell.h"

class ConvolutionMatrixEditor : public DockWindowShell
{
public:
   ConvolutionMatrixEditor();
   virtual ~ConvolutionMatrixEditor();
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual QAction* createAction();
   virtual QWidget* createWidget();

private:
   ConvolutionMatrixEditor(const ConvolutionMatrixEditor& rhs);
   ConvolutionMatrixEditor& operator=(const ConvolutionMatrixEditor& rhs);
};

#endif
