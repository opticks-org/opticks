/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOUSEMODEIMP_H
#define MOUSEMODEIMP_H

#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtGui/QCursor>

#include "MouseMode.h"

#include <string>

class MouseModeImp : public MouseMode
{
public:
   MouseModeImp(const QString& strModeName, const QCursor& mouseCursor, QAction* pAction = NULL);
   ~MouseModeImp();

   void getName(std::string& modeName) const;
   QString getName() const;
   QCursor getCursor() const;
   QAction* getAction() const;

private:
   QString mName;
   QCursor mCursor;
   QAction* mpAction;
};

#endif
