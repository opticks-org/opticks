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

#include "AlgorithmShell.h"
#include <boost/any.hpp>
#include <QtCore/QObject>

class DockWindow;

class ConvolutionMatrixEditor : public QObject, public AlgorithmShell
{
   Q_OBJECT

public:
   static std::string getWindowName() { return "Convolution Matrix Editor"; }
   ConvolutionMatrixEditor();
   virtual ~ConvolutionMatrixEditor();

   void windowHidden(Subject& subject, const std::string& signal, const boost::any& v);
   void windowShown(Subject& subject, const std::string& signal, const boost::any& v);

   bool execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList);
   bool getInputSpecification(PlugInArgList*& pArgList)
   {
      pArgList = NULL;
      return true;
   }
   bool getOutputSpecification(PlugInArgList*& pArgList)
   {
      pArgList = NULL;
      return true;
   }

   bool setBatch()
   {
      AlgorithmShell::setBatch();
      return false;
   }

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

protected slots:
   void displayEditorWindow(bool bDisplay);
   bool createEditorWindow();
   void createMenuItem();
   void attachToEditorWindow(DockWindow* pDockWindow);

private:
   QAction* mpWindowAction;
};

#endif
