/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOUSEMODEPLUGIN_H
#define MOUSEMODEPLUGIN_H

#include <QtCore/QObject>
#include <QtGui/QAction>

#include "AlgorithmShell.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"

#include <boost/any.hpp>
#include <string>

class MouseMode;
class SpatialDataView;

class MouseModePlugIn : public QObject, public AlgorithmShell
{
public:
   MouseModePlugIn();
   ~MouseModePlugIn();

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void windowAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void windowRemoved(Subject& subject, const std::string& signal, const boost::any& value);

   void addMouseMode(SpatialDataView* pView);
   void removeMouseMode(SpatialDataView* pView);
   void enableAction();

private:
   MouseMode* mpMouseMode;
   QAction* mpMouseModeAction;
};

#endif
