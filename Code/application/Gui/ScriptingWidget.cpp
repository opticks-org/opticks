/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ScriptingWidget.h"
#include "Interpreter.h"
#include "InterpreterManager.h"
#include "SessionManager.h"
#include "Slot.h"
#include "xmlwriter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QMenu>
#include <QtGui/QTextCursor>

#include <vector>

XERCES_CPP_NAMESPACE_USE

ScriptingWidget::ScriptingWidget(const QString& strPlugInName, QWidget* parent) :
   QTextEdit(parent),
   mDropOccurring(false),
   mDisplayedOnce(false),
   mPrompt(strPlugInName + "> "),
   mCommandIndex(0),
   mMaxParagraphs(512),
   mInterpreterName(strPlugInName),
   mInterpreter(),
   mCommandStartPos(0),
   mExecutingCommand(false),
   mInteractive(false),
   mpPims(Service<PlugInManagerServices>().get(), SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), 
      Slot(this, &ScriptingWidget::plugInDestroyed))
{
   // Initialization
   setFrameStyle(QFrame::NoFrame);
   setMinimumWidth(150);
   setUndoRedoEnabled(false);
   setFocus();

   mpClearAction = new QAction("Clear", this);
   VERIFYNR(connect(mpClearAction, SIGNAL(triggered(bool)), this, SLOT(clearOutput())));
   mpGlobalOutputAction = new QAction("Global Output Shown", this);
   mpGlobalOutputAction->setCheckable(true);
   VERIFYNR(connect(mpGlobalOutputAction, SIGNAL(triggered(bool)), this, SLOT(globalOutputChanged(bool))));
}

ScriptingWidget::~ScriptingWidget()
{
}

void ScriptingWidget::setCommandFont(const QFont& commandFont)
{
   mCommandFont = commandFont;
}

QFont ScriptingWidget::getCommandFont() const
{
   return mCommandFont;
}

void ScriptingWidget::setOutputFont(const QFont& outputFont)
{
   mOutputFont = outputFont;
}

QFont ScriptingWidget::getOutputFont() const
{
   return mOutputFont;
}

void ScriptingWidget::setErrorFont(const QFont& errorFont)
{
   mErrorFont = errorFont;
}

QFont ScriptingWidget::getErrorFont() const
{
   return mErrorFont;
}

void ScriptingWidget::setOutputColor(const QColor& outputColor)
{
   if ((outputColor != mOutputColor) && (outputColor.isValid() == true))
   {
      mOutputColor = outputColor;
   }
}

QColor ScriptingWidget::getOutputColor() const
{
   return mOutputColor;
}

void ScriptingWidget::setErrorColor(const QColor& errorColor)
{
   if ((errorColor != mErrorColor) && (errorColor.isValid() == true))
   {
      mErrorColor = errorColor;
   }
}

QColor ScriptingWidget::getErrorColor() const
{
   return mErrorColor;
}

void ScriptingWidget::setMaxNumParagraphs(unsigned int numParagraphs)
{
   if (numParagraphs < 1)
   {
      numParagraphs = 1;
   }

   mMaxParagraphs = numParagraphs;

   // Truncate the current text if necessary
   clipToMaxParagraphs();
}

unsigned int ScriptingWidget::getMaxNumParagraphs() const
{
   return mMaxParagraphs;
}

const PlugIn* ScriptingWidget::getInterpreter() const
{
   return mInterpreter.get();
}

bool ScriptingWidget::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   // Attributes
   pXml->addAttr("commandFont", mCommandFont.toString().toStdString());
   pXml->addAttr("outputFont", mOutputFont.toString().toStdString());
   pXml->addAttr("errorFont", mErrorFont.toString().toStdString());
   pXml->addAttr("outputColor", mOutputColor.name().toStdString());
   pXml->addAttr("errorColor", mErrorColor.name().toStdString());
   pXml->addAttr("maxParagraphs", mMaxParagraphs);

   // Interpreter
   const PlugIn* pInterpreter = getInterpreter();
   if (pInterpreter != NULL)
   {
      const std::string& interpreterId = pInterpreter->getId();
      pXml->addAttr("interpreterId", interpreterId);
   }

   return true;
}

bool ScriptingWidget::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   if (pElement == NULL)
   {
      return false;
   }

   // Attributes
   std::string fontText = A(pElement->getAttribute(X("commandFont")));
   if (fontText.empty() == false)
   {
      if (mCommandFont.fromString(QString::fromStdString(fontText)) == false)
      {
         return false;
      }
   }

   fontText = A(pElement->getAttribute(X("outputFont")));
   if (fontText.empty() == false)
   {
      if (mOutputFont.fromString(QString::fromStdString(fontText)) == false)
      {
         return false;
      }
   }

   fontText = A(pElement->getAttribute(X("errorFont")));
   if (fontText.empty() == false)
   {
      if (mErrorFont.fromString(QString::fromStdString(fontText)) == false)
      {
         return false;
      }
   }

   std::string colorText = A(pElement->getAttribute(X("outputColor")));
   if (colorText.empty() == false)
   {
      mOutputColor.setNamedColor(QString::fromStdString(colorText));
   }

   colorText = A(pElement->getAttribute(X("errorColor")));
   if (colorText.empty() == false)
   {
      mErrorColor.setNamedColor(QString::fromStdString(colorText));
   }

   mMaxParagraphs = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("maxParagraphs"))));

   // Interpreter
   std::string interpreterId = A(pElement->getAttribute(X("interpreterId")));
   if (interpreterId.empty() == false)
   {
      Service<SessionManager> pManager;

      PlugIn* pInterpreter = dynamic_cast<PlugIn*>(pManager->getSessionItem(interpreterId));
      if (pInterpreter == NULL)
      {
         return false;
      }

      mpPims.reset(NULL);
      mInterpreter = PlugInResource(pInterpreter);
      mpPims.reset(Service<PlugInManagerServices>().get());
   }

   return true;
}

void ScriptingWidget::showEvent(QShowEvent* pEvent)
{
   if ((mInterpreter.get() == NULL) && (mInterpreterName.isEmpty() == false))
   {
      Interpreter* pInterpreter = NULL;
      setReadOnly(true);
      std::vector<PlugIn*> plugIns = mpPims->getPlugInInstances(mInterpreterName.toStdString());
      if (plugIns.empty() == false && dynamic_cast<InterpreterManager*>(plugIns.front()) != NULL)
      {
         mInterpreter = PlugInResource(plugIns.front());
      }
      else
      {
         mInterpreter = PlugInResource(mInterpreterName.toStdString());
      }

      InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
      if (pInterMgr != NULL)
      {
         mInteractive = pInterMgr->isInteractiveEnabled();
         pInterMgr->start();
         QString startupMsg = QString::fromStdString(pInterMgr->getStartupMessage());
         addOutputText(startupMsg, false);
         pInterMgr->attach(SIGNAL_NAME(InterpreterManager, InterpreterStarted), Slot(this, &ScriptingWidget::interpreterStarted));
         pInterpreter = pInterMgr->getInterpreter();
      }

      if (pInterpreter != NULL)
      {
         pInterpreter->attach(SIGNAL_NAME(Interpreter, OutputText), Slot(this, &ScriptingWidget::receiveOutput));
         pInterpreter->attach(SIGNAL_NAME(Interpreter, ErrorText), Slot(this, &ScriptingWidget::receiveOutput));
         setReadOnly(!mInteractive);
      }
   }

   // Append a prompt if the widget is shown for the first time
   if (mDisplayedOnce == false)
   {
      appendPrompt();
      mDisplayedOnce = true;
   }

   // Show the widget
   QTextEdit::showEvent(pEvent);
}

void ScriptingWidget::keyPressEvent(QKeyEvent* e)
{
   if (e != NULL)
   {
      QTextCursor currentSelection = textCursor();
      int iStartSelection = currentSelection.selectionStart();
      int iEndSelection = currentSelection.selectionEnd();

      int iCommandStart = getCommandPosition();

      if ((e->key() == Qt::Key_Backspace) || (e->key() == Qt::Key_Left))
      {
         // Do not move the cursor past the current command
         if (iStartSelection <= iCommandStart)
         {
            return;
         }
      }
      else if ((e->key() == Qt::Key_Up) || (e->key() == Qt::Key_Down))
      {
         if (mCommandStack.empty() == false)
         {
            // Remove the current command text
            currentSelection.setPosition(iCommandStart);
            currentSelection.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, toPlainText().size() - iCommandStart);
            currentSelection.removeSelectedText();

            // Get the next command in the stack
            if (e->key() == Qt::Key_Up)
            {
               if (mCommandIndex > 0)
               {
                  mCommandIndex--;
               }
            }
            else if (e->key() == Qt::Key_Down)
            {
               // Let user go one past the end of the stack to clear command line.
               if (mCommandIndex < mCommandStack.count())
               {
                  mCommandIndex++;
               }
            }

            QString strCommand;
            if (mCommandIndex >= 0 && mCommandIndex < mCommandStack.count())
            {
               strCommand = mCommandStack[mCommandIndex];
            }
            else
            {
               // One past the end of the command stack, so clear the command text
               strCommand.clear();
            }

            // Add the new command text
            currentSelection.insertText(strCommand);
            currentSelection.setPosition(currentSelection.selectionEnd());
            setTextCursor(currentSelection);
         }

         return;
      }
      else if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter))
      {
         executeCommand();
         clipToMaxParagraphs();
         return;
      }
      else
      {
         // Do not overwrite selected text from previous entries
         if (iStartSelection < iCommandStart)
         {
            // Set the cursor to the start of the current command
            currentSelection.setPosition(iCommandStart);

            // Select the portion of the command that was previously selected
            if (iEndSelection >= iCommandStart)
            {
               currentSelection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                  iEndSelection - iCommandStart);
            }
         }

         setTextCursor(currentSelection);

         // Set the command font as the current font
         setCurrentFont(mCommandFont);
      }
   }

   QTextEdit::keyPressEvent(e);
}

void ScriptingWidget::contextMenuEvent(QContextMenuEvent* pEvent)
{
   Interpreter* pInterpreter = NULL;
   InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
   if (pInterMgr != NULL)
   {
      pInterpreter = pInterMgr->getInterpreter();
   }
   if (pInterpreter == NULL)
   {
      mpGlobalOutputAction->setDisabled(true);
      mpGlobalOutputAction->setChecked(false);
   }
   else
   {
      bool globalOutput = pInterpreter->isGlobalOutputShown();
      mpGlobalOutputAction->setEnabled(true);
      mpGlobalOutputAction->setChecked(globalOutput);
   }
   QMenu* pMenu = createStandardContextMenu(pEvent->globalPos());
   pMenu->addSeparator();
   pMenu->addAction(mpClearAction);
   pMenu->addAction(mpGlobalOutputAction);
   pMenu->exec(pEvent->globalPos());
   delete pMenu;
}

unsigned int ScriptingWidget::getNumParagraphs() const
{
   unsigned int numParagraphs = 0;

   QString strText = toPlainText();
   if (strText.isEmpty() == false)
   {
      numParagraphs = strText.count("\n") + 1;
   }

   return numParagraphs;
}

QString ScriptingWidget::getCommandText() const
{
   QString strCommand;

   QString strText = toPlainText();
   if (strText.isEmpty() == false)
   {
      strCommand = strText.mid(getCommandPosition());
   }

   return strCommand;
}

int ScriptingWidget::getCommandPosition() const
{
   return mCommandStartPos;
}

void ScriptingWidget::clipToMaxParagraphs()
{
   while (getNumParagraphs() > mMaxParagraphs)
   {
      QString strText = toPlainText();
      if (strText.isEmpty() == true)
      {
         break;
      }

      int iPos = strText.indexOf("\n");
      if (iPos == -1)
      {
         break;
      }

      QTextCursor paragraphSelection = textCursor();
      paragraphSelection.setPosition(0);
      paragraphSelection.setPosition(iPos + 1, QTextCursor::KeepAnchor);
      paragraphSelection.removeSelectedText();
      mCommandStartPos -= iPos + 1;
      paragraphSelection.movePosition(QTextCursor::End);

      setTextCursor(paragraphSelection);
   }
}

void ScriptingWidget::appendPrompt()
{
   QString prompt;
   InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
   if (pInterMgr != NULL)
   {
      Interpreter* pInterpreter = pInterMgr->getInterpreter();
      if (pInterMgr->isStarted() && pInterpreter != NULL && mInteractive)
      {
         prompt = QString::fromStdString(pInterpreter->getPrompt());
      }
   }
   mPrompt = prompt;

   setCurrentFont(font());
   setTextColor(Qt::black);
   if (!prompt.isEmpty())
   {
      QTextCursor cursorPosition = textCursor();
      cursorPosition.movePosition(QTextCursor::End);
      int endPos = cursorPosition.position();
      cursorPosition.movePosition(QTextCursor::StartOfLine);
      int startLinePos = cursorPosition.position();
      cursorPosition.movePosition(QTextCursor::End);
      if (startLinePos != endPos)
      {
         cursorPosition.insertText("\n", currentCharFormat());
      }
      cursorPosition.insertText(mPrompt, currentCharFormat());
      setTextCursor(cursorPosition);
   }
   mCommandStartPos = toPlainText().size();
   setCurrentFont(mCommandFont);
}

void ScriptingWidget::executeCommand()
{
   QString strCommand = getCommandText();
   if (strCommand.isEmpty() == false)
   {
      // Add the command to the stack and reset the current stack command
      // Current stack command should be one past end of stack.
      mCommandStack.append(strCommand);
      mCommandIndex = mCommandStack.count();
   }

   QTextCursor cursor = textCursor();
   cursor.movePosition(QTextCursor::End);
   cursor.insertText("\n", currentCharFormat());
   executeCommand(strCommand);
}

void ScriptingWidget::executeCommand(const QString& strCommand)
{
   Interpreter* pInterpreter = NULL;
   InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
   if (pInterMgr != NULL)
   {
      pInterpreter = pInterMgr->getInterpreter();
   }
   if (pInterpreter == NULL)
   {
      appendPrompt();
      QMessageBox::critical(this, "Scripting Window", "Could not get the interpreter plug-in!");
      return;
   }

   // Set the input values for the plug-in
   mExecutingCommand = true;
   pInterpreter->executeCommand(strCommand.toStdString());
   mExecutingCommand = false;

   appendPrompt();
}

void ScriptingWidget::addOutputText(const QString& strMessage, bool error)
{
   if (strMessage.isEmpty() == false)
   {
      QTextCharFormat newTextFormat = currentCharFormat();
      if (error)
      {
         newTextFormat.setFont(mErrorFont);
         newTextFormat.setForeground(QBrush(mErrorColor));
      }
      else
      {
         newTextFormat.setFont(mOutputFont);
         newTextFormat.setForeground(QBrush(mOutputColor));
      }
      QTextCursor cursor = textCursor();
      cursor.clearSelection();
      if (mExecutingCommand)
      {
         cursor.movePosition(QTextCursor::End);
      }
      else
      {
         int pos = std::max(mCommandStartPos - mPrompt.size(), 0);
         cursor.setPosition(pos);
         mCommandStartPos += strMessage.size();
      }
      cursor.insertText(strMessage, newTextFormat);
      if (mExecutingCommand)
      {
         setTextCursor(cursor);
      }
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   }
}

void ScriptingWidget::clearOutput()
{
   clear();
   appendPrompt();
}

void ScriptingWidget::globalOutputChanged(bool newValue)
{
   InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
   if (pInterMgr == NULL)
   {
      return;
   }
   Interpreter* pInterpreter = pInterMgr->getInterpreter();
   if (pInterpreter == NULL)
   {
      return;
   }
   pInterpreter->showGlobalOutput(newValue);
}

bool ScriptingWidget::canInsertFromMimeData(const QMimeData* pSource) const
{
   if (pSource->hasUrls() || pSource->hasText())
   {
      return true;
   }
   return false;
}

void ScriptingWidget::dropEvent(QDropEvent* pEvent)
{
   mDropOccurring = true;
   QTextEdit::dropEvent(pEvent);
   mDropOccurring = false;
}

void ScriptingWidget::insertFromMimeData(const QMimeData* pSource)
{
   QString filePath;
   QString text;
   if (mDropOccurring && pSource->hasUrls())
   {
      QList<QUrl> urls = pSource->urls();
      if (!urls.empty())
      {
         filePath = urls.front().toLocalFile();
      }
   }
   if (pSource->hasText())
   {
      text = pSource->text();
      int numNewlines = text.count("\n");
      QString trimmedText = text.trimmed();
      bool haveFile = false;
      if (mDropOccurring && numNewlines <= 1 && QFile::exists(trimmedText))
      {
         filePath = trimmedText;
      }
   }
   if (!filePath.isEmpty())
   {
      //don't get here if mDropOccurring == false
      InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
      bool bMatch = false;
      if (pInterMgr != NULL)
      {
         QString strInterpreterExtensions = QString::fromStdString(pInterMgr->getFileExtensions());
         if (!strInterpreterExtensions.isEmpty())
         {
            QStringList filterListCandidates = strInterpreterExtensions.split(";;", QString::SkipEmptyParts);
            QStringList filterList;
            for (int i = 0; i < filterListCandidates.count(); i++)
            {
               QString strExtensions = filterListCandidates[i];
               if (strExtensions.isEmpty() == false)
               {
                  int iOpenPos = strExtensions.indexOf("(");
                  int iClosePos = strExtensions.lastIndexOf(")");
                  strExtensions = strExtensions.mid(iOpenPos + 1, iClosePos - iOpenPos - 1);

                  QStringList globPatterns = strExtensions.split(QString(" "), QString::SkipEmptyParts);
                  QString catchAll = QString::fromStdString("*");
                  QString catchAll2 = QString::fromStdString("*.*");
                  for (int globCount = 0; globCount < globPatterns.count(); ++globCount)
                  {
                     QString pattern = globPatterns[globCount];
                     if ((pattern != catchAll) && (pattern != catchAll2))
                     {
                        filterList << pattern;
                     }
                  }
               }
            }

            bMatch = QDir::match(filterList, filePath);
         }
      }

      QFile file(filePath);
      if (bMatch && file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
         QTextStream reader(&file);
         QString fileContent = reader.readAll();
         QTextCursor cursor = textCursor();
         cursor.movePosition(QTextCursor::End);
         cursor.insertText("\n");
         executeCommand(fileContent);
         return;
      }
   }
   if (!text.isEmpty())
   {
      bool insertText = true;
      if (mDropOccurring)
      {
         if (getCommandText().isEmpty())
         {
            insertText = false;
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText("\n");
            executeCommand(text);
         }
      }
      if (insertText)
      {
         QTextCursor cursor = textCursor();
         cursor.insertText(text, currentCharFormat());
      }
   }
}

void ScriptingWidget::interpreterStarted(Subject& subject, const std::string& signal, const boost::any& data)
{
   Interpreter* pInterpreter = NULL;
   InterpreterManager* pInterMgr = dynamic_cast<InterpreterManager*>(mInterpreter.get());
   if (pInterMgr != NULL)
   {
      pInterpreter = pInterMgr->getInterpreter();
      QString startupMsg = QString::fromStdString(pInterMgr->getStartupMessage());
      addOutputText(startupMsg, false);
   }
   if (pInterpreter != NULL)
   {
      pInterpreter->attach(SIGNAL_NAME(Interpreter, OutputText), Slot(this, &ScriptingWidget::receiveOutput));
      pInterpreter->attach(SIGNAL_NAME(Interpreter, ErrorText), Slot(this, &ScriptingWidget::receiveOutput));
      setReadOnly(!mInteractive);
      appendPrompt();
   }
}

void ScriptingWidget::receiveOutput(Subject& subject, const std::string& signal, const boost::any& data)
{
   std::string text = boost::any_cast<std::string>(data);
   if (signal == SIGNAL_NAME(Interpreter, OutputText))
   {
      addOutputText(QString::fromStdString(text), false);
   }
   else if (signal == SIGNAL_NAME(Interpreter, ErrorText))
   {
      addOutputText(QString::fromStdString(text), true);
   }
}

void ScriptingWidget::plugInDestroyed(Subject &subject, const std::string &signal, const boost::any &data)
{
   if (boost::any_cast<PlugIn*>(data) == mInterpreter.get())
   {
      mInterpreter = PlugInResource();
   }
}
