/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCRIPTINGWINDOW_H
#define SCRIPTINGWINDOW_H

#include "AttachmentPtr.h"
#include "DockWindowAdapter.h"
#include "SessionManager.h"

#include <QtGui/QLabel>
#include <QtGui/QTabWidget>
#include <QtGui/QStackedWidget>
#include <string>

class ScriptingWidget;

class ScriptingWindow : public DockWindowAdapter
{
   Q_OBJECT

public:
   ScriptingWindow(const std::string& id, QWidget* pParent = 0);
   ~ScriptingWindow();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WindowType getWindowType() const;

   void setCommandFont(const QFont& commandFont);
   QFont getCommandFont() const;
   void setOutputFont(const QFont& outputFont);
   QFont getOutputFont() const;
   void setErrorFont(const QFont& errorFont);
   QFont getErrorFont() const;

   void setOutputColor(const QColor& outputColor);
   QColor getOutputColor() const;
   void setErrorColor(const QColor& errorColor);
   QColor getErrorColor() const;

   void setScrollBuffer(int maxParagraphs);
   int getScrollBuffer() const;

   ScriptingWidget* getInterpreter(const QString& strName) const;
   ScriptingWidget* getCurrentInterpreter() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void updateInterpreters();

protected slots:
   void clear();

private:
   void sessionClosed(Subject &subject, const std::string &signal, const boost::any &data);

   QLabel* mpEmptyLabel;
   QTabWidget* mpTabWidget;
   QStackedWidget* mpStack;

   QFont mCommandFont;
   QFont mOutputFont;
   QFont mErrorFont;

   QColor mOutputColor;
   QColor mErrorColor;

   int mScrollBuffer;
   AttachmentPtr<SessionManager> mpSessionManager;
};

#endif
