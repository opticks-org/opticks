/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QTextCursor>

#include "ScriptingWidget.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "Interpreter.h"
#include "LayerList.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "RasterElement.h"
#include "SessionManager.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "xmlwriter.h"

#include <vector>

XERCES_CPP_NAMESPACE_USE
using namespace std;

ScriptingWidget::ScriptingWidget(const QString& strPlugInName, QWidget* parent) :
   QTextEdit(parent),
   mDisplayedOnce(false),
   mPrompt(strPlugInName + "> "),
   mCommandIndex(0),
   mMaxParagraphs(512),
   mInterpreterName(strPlugInName),
   mInterpreter(NULL, false),
   mpPims(Service<PlugInManagerServices>().get(), SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), 
      Slot(this, &ScriptingWidget::plugInDestroyed))
{
   // TODO: Add a syntax highlighter to set the command color

   // Initialization
   setFrameStyle(QFrame::NoFrame);
   setMinimumWidth(150);
   setUndoRedoEnabled(false);
   setFocus();
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

void ScriptingWidget::setCommandColor(const QString& command, const QColor& commandColor)
{
   if ((command.isEmpty() == true) || (commandColor.isValid() == false))
   {
      return;
   }

   QMap<QString, QColor>::iterator iter = mCommandColors.find(command);
   if (iter != mCommandColors.end())
   {
      mCommandColors[command] = commandColor;
   }
}

QColor ScriptingWidget::getCommandColor(const QString& command) const
{
   QColor commandColor;

   QMap<QString, QColor>::const_iterator iter = mCommandColors.find(command);
   if (iter != mCommandColors.end())
   {
      commandColor = iter.value();
   }

   return commandColor;
}

const QMap<QString, QColor>& ScriptingWidget::getCommandColors() const
{
   return mCommandColors;
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
   return mInterpreter->getPlugIn();
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
      const string& interpreterId = pInterpreter->getId();
      pXml->addAttr("interpreterId", interpreterId);
   }

   // Command colors
   for (QMap<QString, QColor>::const_iterator iter = mCommandColors.begin(); iter != mCommandColors.end(); ++iter)
   {
      pXml->pushAddPoint(pXml->addElement("CommandColor"));
      pXml->addAttr("name", iter.key().toStdString());
      pXml->addAttr("color", iter.value().name().toStdString());
      pXml->popAddPoint();
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
   string fontText = A(pElement->getAttribute(X("commandFont")));
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

   string colorText = A(pElement->getAttribute(X("outputColor")));
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
   string interpreterId = A(pElement->getAttribute(X("interpreterId")));
   if (interpreterId.empty() == false)
   {
      Service<SessionManager> pManager;

      PlugIn* pInterpreter = dynamic_cast<PlugIn*>(pManager->getSessionItem(interpreterId));
      if (pInterpreter == NULL)
      {
         return false;
      }

      mpPims.reset(NULL);
      mInterpreter->setPlugIn(pInterpreter);
      mpPims.reset(Service<PlugInManagerServices>().get());
   }

   // Command colors
   mCommandColors.clear();
   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("CommandColor")))
      {
         DOMElement* pChildElement = static_cast<DOMElement*>(pChild);

         QString commandName = QString::fromStdString(A(pChildElement->getAttribute(X("name"))));
         QColor commandColor;

         colorText = A(pChildElement->getAttribute(X("color")));
         if (colorText.empty() == false)
         {
            commandColor.setNamedColor(QString::fromStdString(colorText));
         }

         mCommandColors.insert(commandName, commandColor);
      }
   }

   return true;
}

void ScriptingWidget::showEvent(QShowEvent* pEvent)
{
   // Load the interpreter plug-in
   if ((mInterpreter->getPlugIn() == NULL) && (mInterpreterName.isEmpty() == false))
   {
      vector<PlugIn*> plugIns = mpPims->getPlugInInstances(mInterpreterName.toStdString());
      if (plugIns.empty() == false && dynamic_cast<Interpreter*>(plugIns.front()) != NULL)
      {
         mInterpreter->setPlugIn(plugIns.front());
      }
      else
      {
         mInterpreter->setPlugIn(mInterpreterName.toStdString());
      }

      Interpreter* pInterpreter = dynamic_cast<Interpreter*>(mInterpreter->getPlugIn());
      if (pInterpreter != NULL)
      {
         vector<string> keywords;
         pInterpreter->getKeywordList(keywords);
         for (unsigned int i = 0; i < keywords.size(); i++)
         {
            string keyword = keywords.at(i);
            if (keyword.empty() == false)
            {
               mCommandColors.insert(QString::fromStdString(keyword), Qt::blue);
            }
         }
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

      int iCommandStart = -1;
      int iCommandEnd = -1;
      getCommandPosition(iCommandStart, iCommandEnd);

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
            currentSelection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
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
      strCommand = strText.section("\n", -1, -1);
      if (strCommand.startsWith(mPrompt) == true)
      {
         strCommand = strCommand.mid(mPrompt.length(), strCommand.length() - mPrompt.length());
      }
   }

   return strCommand;
}

void ScriptingWidget::getCommandPosition(int& iCommandStart, int& iCommandEnd) const
{
   iCommandStart = 0;
   iCommandEnd = 0;

   QString strText = toPlainText();
   if (strText.isEmpty() == false)
   {
      iCommandStart = strText.length();
      iCommandEnd = strText.length();

      QString strCommand = getCommandText();
      if (strCommand.isEmpty() == false)
      {
         iCommandStart = strText.lastIndexOf(strCommand);
      }
   }
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
      paragraphSelection.movePosition(QTextCursor::End);

      setTextCursor(paragraphSelection);
   }
}

void ScriptingWidget::appendPrompt()
{
   InterpreterExt1* pInterpreter = dynamic_cast<InterpreterExt1*>(mInterpreter->getPlugIn());
   if (pInterpreter != NULL)
   {
      mPrompt = QString::fromStdString(pInterpreter->getPrompt());
   }

   setCurrentFont(font());
   setTextColor(Qt::black);
   append(mPrompt);

   QTextCursor cursorPosition = textCursor();
   cursorPosition.movePosition(QTextCursor::End);
   setTextCursor(cursorPosition);

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

   executeCommand(strCommand);
}

void ScriptingWidget::executeCommand(const QString& strCommand)
{
   if (mInterpreter->getPlugIn() == NULL)
   {
      appendPrompt();
      QMessageBox::critical(this, "Scripting Window", "Could not get the interpreter plug-in!");
      return;
   }

   // Process the command
   if (strCommand == "clear")
   {
      // Clear the text
      clear();

      // Clear the command stack
      mCommandStack.clear();
      mCommandIndex = 0;
   }
   else if (strCommand == "commands")
   {
      vector<string> keywords;

      Interpreter* pInterpreter = dynamic_cast<Interpreter*>(mInterpreter->getPlugIn());
      if (pInterpreter != NULL)
      {
         pInterpreter->getKeywordList(keywords);
      }

      for (unsigned int i = 0; i < keywords.size(); i++)
      {
         string keyword = keywords.at(i);
         if (keyword.empty() == false)
         {
            addOutputText(QString::fromStdString(keyword));
         }
      }
   }
   else
   {
      // Set the input values for the plug-in
      string command = strCommand.toStdString();
      mInterpreter->getInArgList().setPlugInArgValue(Interpreter::CommandArg(), &command);

      Service<DesktopServices> pDesktop;

      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pDesktop->getCurrentWorkspaceWindowView());
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
            if (pRaster != NULL)
            {
               mInterpreter->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pRaster);
            }
         }
      }

      string outputText = "";
      string returnType = "";

      // Run the command in the plug-in
      bool bSuccess = mInterpreter->execute();
      if (bSuccess == true)
      {
         mInterpreter->getOutArgList().getPlugInArgValue(Interpreter::OutputTextArg(), outputText);
         mInterpreter->getOutArgList().getPlugInArgValue(Interpreter::ReturnTypeArg(), returnType);
      }

      // Add the output text to the window
      if (outputText.empty() == false)
      {
         // Set the font and color based on the return type
         if (returnType == "Output")
         {
            setCurrentFont(mOutputFont);
            setTextColor(mOutputColor);
         }
         else if (returnType == "Error")
         {
            setCurrentFont(mErrorFont);
            setTextColor(mErrorColor);
         }

         addOutputText(QString::fromStdString(outputText));
      }
   }

   appendPrompt();
}

void ScriptingWidget::addOutputText(const QString& strMessage)
{
   if (strMessage.isEmpty() == false)
   {
      append(strMessage);
   }
}

void ScriptingWidget::plugInDestroyed(Subject &subject, const std::string &signal, const boost::any &data)
{
   if (boost::any_cast<PlugIn*>(data) == mInterpreter->getPlugIn())
   {
      mInterpreter->setPlugIn(NULL);
      mCommandColors.clear();
   }
}
