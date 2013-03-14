/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCRIPTINGWIDGET_H
#define SCRIPTINGWIDGET_H

#include "AttachmentPtr.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"

#include <QtCore/QStringList>
#include <QtGui/QTextEdit>

#include <string>
#include <boost/any.hpp>

class PlugIn;
class QAction;
class Subject;

class ScriptingWidget : public QTextEdit
{
   Q_OBJECT

public:
   ScriptingWidget(const QString& strPlugInName, QWidget* parent = 0);
   ~ScriptingWidget();

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

   void setMaxNumParagraphs(unsigned int numParagraphs);
   unsigned int getMaxNumParagraphs() const;

   const PlugIn* getInterpreter() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   void executeCommand(const QString& strCommand);

protected:
   void showEvent(QShowEvent* pEvent);
   void keyPressEvent(QKeyEvent* e);
   void contextMenuEvent(QContextMenuEvent* pEvent);

   unsigned int getNumParagraphs() const;
   QString getCommandText() const;
   int getCommandPosition() const;

protected slots:
   void clipToMaxParagraphs();
   void appendPrompt();
   void executeCommand();
   void addOutputText(const QString& strMessage, bool error, bool processEvents);
   void clearOutput();
   void globalOutputChanged(bool newValue);

private:
   ScriptingWidget(const ScriptingWidget& rhs);
   ScriptingWidget& operator=(const ScriptingWidget& rhs);
   bool mDropOccurring;
   bool mDisplayedOnce;
   QString mPrompt;
   QStringList mCommandStack;
   int mCommandIndex;

   QFont mCommandFont;
   QFont mOutputFont;
   QFont mErrorFont;

   QColor mOutputColor;
   QColor mErrorColor;
   QAction* mpClearAction;
   QAction* mpGlobalOutputAction;

   unsigned int mMaxParagraphs;

   QString mInterpreterName;
   PlugInResource mInterpreter;
   int mCommandStartPos;
   bool mExecutingCommand;
   bool mInteractive;
   AttachmentPtr<PlugInManagerServices> mpPims;

   virtual bool canInsertFromMimeData(const QMimeData* pSource) const;
   virtual void dropEvent(QDropEvent* pEvent);
   virtual void insertFromMimeData(const QMimeData* pSource);
   void receiveOutput(Subject& subject, const std::string& signal, const boost::any& value);
   void interpreterStarted(Subject& subject, const std::string& signal, const boost::any& value);
   void plugInDestroyed(Subject& subject, const std::string& signal, const boost::any& value);
};

#endif
