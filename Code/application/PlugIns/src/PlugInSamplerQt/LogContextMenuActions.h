/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOGCONTEXTMENUACTIONS_H
#define LOGCONTEXTMENUACTIONS_H

#include <QtCore/QObject>
#include <QtGui/QAction>

#include "AlgorithmShell.h"
#include "DesktopServices.h"

class LogContextMenuActions : public QObject, public AlgorithmShell
{
   Q_OBJECT

public:
   LogContextMenuActions();
   ~LogContextMenuActions();

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected slots:
   void logContextMenuActions(bool bLog);

private:
   LogContextMenuActions(const LogContextMenuActions& rhs);
   LogContextMenuActions& operator=(const LogContextMenuActions& rhs);
   bool createAction();
   Service<DesktopServices> mpDesktop;
   QAction* mpLogAction;
};

#endif
