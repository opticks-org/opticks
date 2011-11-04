/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QFrame>
#include <QtGui/QLayout>

#include "PlugIn.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PropertiesScriptingWindow.h"
#include "ScriptingWidget.h"
#include "ScriptingWindow.h"
#include "xmlwriter.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

ScriptingWindow::ScriptingWindow(const string& id, QWidget* pParent) :
   DockWindowAdapter(id, "Scripting Window", pParent),
   mpEmptyLabel(NULL),
   mpTabWidget(NULL),
   mpStack(NULL),
   mCommandFont(QApplication::font()),
   mOutputFont(QApplication::font()),
   mErrorFont(QApplication::font()),
   mOutputColor(Qt::darkGreen),
   mErrorColor(Qt::darkRed),
   mScrollBuffer(512),
   mpSessionManager(Service<SessionManager>().get(),
            SIGNAL_NAME(SessionManager, Closed), Slot(this, &ScriptingWindow::sessionClosed))
{
   // Widget stack
   mpStack = new QStackedWidget(this);

   // Empty label
   mpEmptyLabel = new QLabel("No interpreters are available", mpStack);
   mpEmptyLabel->setMinimumSize(250, 125);
   mpEmptyLabel->setAlignment(Qt::AlignCenter);
   mpStack->addWidget(mpEmptyLabel);

   // Tab widget
   mpTabWidget = new QTabWidget(mpStack);
   mpTabWidget->setTabPosition(QTabWidget::South);
   mpTabWidget->setTabShape(QTabWidget::Triangular);
   mpStack->addWidget(mpTabWidget);

   // Initialization
   setIcon(QIcon(":/icons/Script"));
   setWidget(mpStack);
   addPropertiesPage(PropertiesScriptingWindow::getName());
   updateInterpreters();
}

ScriptingWindow::~ScriptingWindow()
{
}

const string& ScriptingWindow::getObjectType() const
{
   static string type("ScriptingWindow");
   return type;
}

bool ScriptingWindow::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return DockWindowAdapter::isKindOf(className);
}

WindowType ScriptingWindow::getWindowType() const
{
   return DOCK_WINDOW;
}

void ScriptingWindow::setCommandFont(const QFont& commandFont)
{
   if (commandFont == mCommandFont)
   {
      return;
   }

   mCommandFont = commandFont;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*>(mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setCommandFont(mCommandFont);
      }
   }
}

QFont ScriptingWindow::getCommandFont() const
{
   return mCommandFont;
}

void ScriptingWindow::setOutputFont(const QFont& outputFont)
{
   if (outputFont == mOutputFont)
   {
      return;
   }

   mOutputFont = outputFont;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setOutputFont(mOutputFont);
      }
   }
}

QFont ScriptingWindow::getOutputFont() const
{
   return mOutputFont;
}

void ScriptingWindow::setErrorFont(const QFont& errorFont)
{
   if (errorFont == mErrorFont)
   {
      return;
   }

   mErrorFont = errorFont;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setErrorFont(mErrorFont);
      }
   }
}

QFont ScriptingWindow::getErrorFont() const
{
   return mErrorFont;
}

void ScriptingWindow::setOutputColor(const QColor& outputColor)
{
   if ((outputColor == mOutputColor) || (outputColor.isValid() == false))
   {
      return;
   }

   mOutputColor = outputColor;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setOutputColor(mOutputColor);
      }
   }
}

QColor ScriptingWindow::getOutputColor() const
{
   return mOutputColor;
}

void ScriptingWindow::setErrorColor(const QColor& errorColor)
{
   if ((errorColor == mErrorColor) || (errorColor.isValid() == false))
   {
      return;
   }

   mErrorColor = errorColor;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setErrorColor(mErrorColor);
      }
   }
}

QColor ScriptingWindow::getErrorColor() const
{
   return mErrorColor;
}

void ScriptingWindow::setScrollBuffer(int maxParagraphs)
{
   if (maxParagraphs == mScrollBuffer)
   {
      return;
   }

   mScrollBuffer = maxParagraphs;

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
      if (pWidget != NULL)
      {
         pWidget->setMaxNumParagraphs(mScrollBuffer);
      }
   }
}

int ScriptingWindow::getScrollBuffer() const
{
   return mScrollBuffer;
}

ScriptingWidget* ScriptingWindow::getInterpreter(const QString& strName) const
{
   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      QString strTabLabel = mpTabWidget->tabText(i);
      if (strTabLabel == strName)
      {
         ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
         return pWidget;
      }
   }

   return NULL;
}

ScriptingWidget* ScriptingWindow::getCurrentInterpreter() const
{
   ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->currentWidget());
   return pWidget;
}

void ScriptingWindow::updateInterpreters()
{
   // Get the list of interpreter plug-ins
   Service<PlugInManagerServices> pManager;
   vector<PlugInDescriptor*> plugIns = pManager->getPlugInDescriptors(PlugInManagerServices::InterpreterManagerType());

   // Remove existing tabs for removed plug-ins
   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      QString strTabLabel = mpTabWidget->tabText(i);
      if (strTabLabel.isEmpty() == false)
      {
         string tabLabel = strTabLabel.toStdString();

         vector<PlugInDescriptor*>::iterator plugInIter;
         for (plugInIter = plugIns.begin(); plugInIter != plugIns.end(); ++plugInIter)
         {
            PlugInDescriptor* pDescriptor = *plugInIter;
            if (pDescriptor == NULL)
            {
               continue;
            }
            string plugIn = pDescriptor->getName();
            if (plugIn == tabLabel)
            {
               break;
            }
         }

         if (plugInIter == plugIns.end())
         {
            ScriptingWidget* pWidget = static_cast<ScriptingWidget*> (mpTabWidget->widget(i));
            mpTabWidget->removeTab(i--);
            delete pWidget;
         }
      }
   }

   // Add a tab for each new plug-in
   for (vector<PlugInDescriptor*>::iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor == NULL)
      {
         continue;
      }
      string currentPlugIn = pDescriptor->getName();
      if (currentPlugIn.empty())
      {
         continue;
      }
      QString strName = QString::fromStdString(currentPlugIn);
      if (getInterpreter(strName) == NULL)
      {
         ScriptingWidget* pWidget = new ScriptingWidget(strName);
         pWidget->setCommandFont(mCommandFont);
         pWidget->setOutputFont(mOutputFont);
         pWidget->setErrorFont(mErrorFont);
         pWidget->setOutputColor(mOutputColor);
         pWidget->setErrorColor(mErrorColor);

         mpTabWidget->addTab(pWidget, strName);
      }
   }

   // Set the window widget
   if (mpTabWidget->count() > 0)
   {
      mpStack->setCurrentWidget(mpTabWidget);
   }
   else
   {
      mpStack->setCurrentWidget(mpEmptyLabel);
   }
}

void ScriptingWindow::clear()
{
   while (mpTabWidget->count() > 0)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*>(mpTabWidget->widget(0));
      mpTabWidget->removeTab(0);
      delete pWidget;
   }
}

void ScriptingWindow::sessionClosed(Subject& subject, const std::string& signal, const boost::any& value)
{
   clear();
}

bool ScriptingWindow::toXml(XMLWriter* pXml) const
{
   if ((pXml == NULL) || (DockWindowImp::toXml(pXml) == false))
   {
      return false;
   }

   pXml->addAttr("commandFont", mCommandFont.toString().toStdString());
   pXml->addAttr("outputFont", mOutputFont.toString().toStdString());
   pXml->addAttr("errorFont", mErrorFont.toString().toStdString());
   pXml->addAttr("outputColor", mOutputColor.name().toStdString());
   pXml->addAttr("errorColor", mErrorColor.name().toStdString());
   pXml->addAttr("scrollBuffer", mScrollBuffer);

   ScriptingWidget* pCurrentInterpreter = getCurrentInterpreter();
   if (pCurrentInterpreter != NULL)
   {
      const PlugIn* pPlugIn = pCurrentInterpreter->getInterpreter();
      if (pPlugIn != NULL)
      {
         pXml->addAttr("currentInterpreterId", pPlugIn->getId());
      }
   }

   for (int i = 0; i < mpTabWidget->count(); ++i)
   {
      ScriptingWidget* pInterpreter = static_cast<ScriptingWidget*>(mpTabWidget->widget(i));
      if (pInterpreter != NULL)
      {
         pXml->pushAddPoint(pXml->addElement("Interpreter"));
         pXml->addAttr("name", mpTabWidget->tabText(i).toStdString());

         if (pInterpreter->toXml(pXml) == false)
         {
            return false;
         }

         pXml->popAddPoint();
      }
   }

   return true;
}

bool ScriptingWindow::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   // Clear any existing tabs
   clear();

   if (DockWindowImp::fromXml(pDocument, version) == false)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);
   if (pElement == NULL)
   {
      return false;
   }

   // Restore the attributes
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

   mScrollBuffer = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("scrollBuffer"))));

   // Restore the interpreters
   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("Interpreter")))
      {
         DOMElement* pChildElement = static_cast<DOMElement*>(pChild);

         QString nodeName = QString::fromStdString(A(pChildElement->getAttribute(X("name"))));
         if (nodeName.isEmpty() == false)
         {
            ScriptingWidget* pWidget = new ScriptingWidget(nodeName);
            if (pWidget->fromXml(pChild, version) == false)
            {
               delete pWidget;
               return false;
            }

            mpTabWidget->addTab(pWidget, nodeName);
         }
      }
   }

   // Set the current tab
   string currentInterpreterId = A(pElement->getAttribute(X("currentInterpreterId")));
   if (currentInterpreterId.empty() == false)
   {
      for (int i = 0; i < mpTabWidget->count(); ++i)
      {
         ScriptingWidget* pWidget = static_cast<ScriptingWidget*>(mpTabWidget->widget(i));
         if (pWidget != NULL)
         {
            const SessionItem* pSessionItem = dynamic_cast<const SessionItem*>(pWidget->getInterpreter());
            if ((pSessionItem != NULL) && (pSessionItem->getId() == currentInterpreterId))
            {
               mpTabWidget->setCurrentIndex(i);
               break;
            }
         }
      }
   }

   return true;
}
