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

#include "ScriptingWindow.h"
#include "DesktopServicesImp.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "PropertiesScriptingWindow.h"
#include "ScriptingWidget.h"

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
   vector<PlugInDescriptor*> plugIns = pManager->getPlugInDescriptors(PlugInManagerServices::InterpreterType());

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

void ScriptingWindow::sessionClosed(Subject &subject, const std::string &signal, const boost::any &data)
{
   while (mpTabWidget->count() > 0)
   {
      ScriptingWidget* pWidget = static_cast<ScriptingWidget*>(mpTabWidget->widget(0));
      mpTabWidget->removeTab(0);
      delete pWidget;
   }
}
