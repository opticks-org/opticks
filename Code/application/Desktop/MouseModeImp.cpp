/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>

#include "MouseModeImp.h"

using namespace std;

MouseModeImp::MouseModeImp(const QString& strModeName, const QCursor& mouseCursor, QAction* pAction) :
   mName(strModeName),
   mCursor(mouseCursor),
   mpAction(pAction)
{
}

MouseModeImp::MouseModeImp(const QString& strModeName, const QBitmap& mouseCursor, const QBitmap& cursorMask,
   int iHotX, int iHotY, QAction* pAction) :
   mName(strModeName),
   mCursor(Qt::ArrowCursor),
   mpAction(pAction)
{
   if (mouseCursor.isNull() == false)
   {
      if (cursorMask.isNull() == false)
      {
         mCursor = QCursor(mouseCursor, cursorMask, iHotX, iHotY);
      }
      else
      {
         mCursor = QCursor(mouseCursor, iHotX, iHotY);
      }
   }
}

MouseModeImp::~MouseModeImp()
{
}

void MouseModeImp::getName(string& modeName) const
{
   modeName.erase();
   if (mName.isEmpty() == false)
   {
      modeName = mName.toStdString();
   }
}

QString MouseModeImp::getName() const
{
   return mName;
}

QCursor MouseModeImp::getCursor() const
{
   return mCursor;
}

QAction* MouseModeImp::getAction() const
{
   return mpAction;
}
