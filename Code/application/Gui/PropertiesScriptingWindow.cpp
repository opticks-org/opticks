/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFontDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "AppVersion.h"
#include "CustomColorButton.h"
#include "CustomTreeWidget.h"
#include "LabeledSection.h"
#include "PropertiesScriptingWindow.h"
#include "ScriptingWidget.h"
#include "ScriptingWindow.h"

using namespace std;

PropertiesScriptingWindow::PropertiesScriptingWindow() :
   LabeledSectionGroup(NULL),
   mpScriptingWindow(NULL)
{
   // General
   QWidget* pGeneralWidget = new QWidget(this);

   QLabel* pScrollLabel = new QLabel("Scroll Buffer:", pGeneralWidget);
   mpScrollSpin = new QSpinBox(this);
   mpScrollSpin->setMinimum(1);
   mpScrollSpin->setMaximum(1024);
   mpScrollSpin->setSingleStep(1);

   LabeledSection* pGeneralSection = new LabeledSection(pGeneralWidget, "General", this);

   QGridLayout* pGeneralGrid = new QGridLayout(pGeneralWidget);
   pGeneralGrid->setMargin(0);
   pGeneralGrid->setSpacing(5);
   pGeneralGrid->addWidget(pScrollLabel, 0, 0);
   pGeneralGrid->addWidget(mpScrollSpin, 0, 1, Qt::AlignLeft);
   pGeneralGrid->setRowStretch(1, 10);
   pGeneralGrid->setColumnStretch(1, 10);

   // Input text
   QWidget* pInputWidget = new QWidget(this);

   QStringList columnNames;
   columnNames.append("Command");
   columnNames.append("Color");

   mpCommandColorTree = new CustomTreeWidget(pInputWidget);
   mpCommandColorTree->setHeaderLabels(columnNames);
   mpCommandColorTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpCommandColorTree->setRootIsDecorated(false);
   mpCommandColorTree->setSortingEnabled(true);
   mpCommandColorTree->setFullCellColor(false);
   mpCommandColorTree->setMinimumHeight(100);

   QHeaderView* pHeader = mpCommandColorTree->header();
   if (pHeader != NULL)
   {
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(true);
   }

   QPushButton* pCommandFontButton = new QPushButton("Font...", pInputWidget);

   LabeledSection* pInputSection = new LabeledSection(pInputWidget, "Input Text", this);

   QVBoxLayout* pInputLayout = new QVBoxLayout(pInputWidget);
   pInputLayout->setMargin(0);
   pInputLayout->setSpacing(5);
   pInputLayout->addWidget(mpCommandColorTree, 10);
   pInputLayout->addWidget(pCommandFontButton, 0, Qt::AlignLeft);

   // Output text
   QWidget* pOutputWidget = new QWidget(this);

   QLabel* pOutputColorLabel = new QLabel("Output Color:", pOutputWidget);
   mpOutputColorButton = new CustomColorButton(pOutputWidget);
   mpOutputColorButton->usePopupGrid(true);
   QLabel* pOutputFontLabel = new QLabel("Output Font:", pOutputWidget);
   QPushButton* pOutputFontButton = new QPushButton("Edit...", pOutputWidget);

   QLabel* pErrorColorLabel = new QLabel("Error Color:", pOutputWidget);
   mpErrorColorButton = new CustomColorButton(pOutputWidget);
   mpErrorColorButton->usePopupGrid(true);
   QLabel* pErrorFontLabel = new QLabel("Error Font:", pOutputWidget);
   QPushButton* pErrorFontButton = new QPushButton("Edit...", pOutputWidget);

   LabeledSection* pOutputSection = new LabeledSection(pOutputWidget, "Output Text", this);

   QGridLayout* pOutputGrid = new QGridLayout(pOutputWidget);
   pOutputGrid->setMargin(0);
   pOutputGrid->setSpacing(5);
   pOutputGrid->addWidget(pOutputColorLabel, 0, 0);
   pOutputGrid->addWidget(mpOutputColorButton, 0, 1, Qt::AlignLeft);
   pOutputGrid->addWidget(pOutputFontLabel, 1, 0);
   pOutputGrid->addWidget(pOutputFontButton, 1, 1, Qt::AlignLeft);
   pOutputGrid->setRowMinimumHeight(2, 15);
   pOutputGrid->addWidget(pErrorColorLabel, 3, 0);
   pOutputGrid->addWidget(mpErrorColorButton, 3, 1, Qt::AlignLeft);
   pOutputGrid->addWidget(pErrorFontLabel, 4, 0);
   pOutputGrid->addWidget(pErrorFontButton, 4, 1, Qt::AlignLeft);
   pOutputGrid->setRowStretch(5, 10);
   pOutputGrid->setColumnStretch(1, 10);

   // Initialization
   addSection(pGeneralSection);
   addSection(pInputSection, 10);
   addSection(pOutputSection);
   setSizeHint(450, 400);

   // Connections
   connect(pCommandFontButton, SIGNAL(clicked()), this, SLOT(setCommandFont()));
   connect(pOutputFontButton, SIGNAL(clicked()), this, SLOT(setOutputFont()));
   connect(pErrorFontButton, SIGNAL(clicked()), this, SLOT(setErrorFont()));
}

PropertiesScriptingWindow::~PropertiesScriptingWindow()
{
}

bool PropertiesScriptingWindow::initialize(SessionItem* pSessionItem)
{
   mpScriptingWindow = dynamic_cast<ScriptingWindow*>(pSessionItem);
   if (mpScriptingWindow == NULL)
   {
      return false;
   }

   // General
   mpScrollSpin->setValue(mpScriptingWindow->getScrollBuffer());

   // Input text
   mpCommandColorTree->clear();

   ScriptingWidget* pWidget = mpScriptingWindow->getCurrentInterpreter();
   if (pWidget != NULL)
   {
      const QMap<QString, QColor>& commandColors = pWidget->getCommandColors();

      QMap<QString, QColor>::const_iterator iter;
      for (iter = commandColors.begin(); iter != commandColors.end(); ++iter)
      {
         QString strCommand = iter.key();
         QColor commandColor = iter.value();

         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpCommandColorTree);
         if (pItem != NULL)
         {
            pItem->setText(0, strCommand);
            mpCommandColorTree->setCellColor(pItem, 1, commandColor);
         }
      }
   }

   mCommandFont = mpScriptingWindow->getCommandFont();

   // Output text
   mpOutputColorButton->setColor(mpScriptingWindow->getOutputColor());
   mOutputFont = mpScriptingWindow->getOutputFont();
   mpErrorColorButton->setColor(mpScriptingWindow->getErrorColor());
   mErrorFont = mpScriptingWindow->getErrorFont();

   return true;
}

bool PropertiesScriptingWindow::applyChanges()
{
   if (mpScriptingWindow == NULL)
   {
      return false;
   }

   // General
   mpScriptingWindow->setScrollBuffer(mpScrollSpin->value());

   // Input text
   ScriptingWidget* pWidget = mpScriptingWindow->getCurrentInterpreter();
   if (pWidget != NULL)
   {
      QTreeWidgetItemIterator iter(mpCommandColorTree);
      while (*iter != NULL)
      {
         QTreeWidgetItem* pItem = *iter;
         if (pItem != NULL)
         {
            QString strCommand = pItem->text(0);
            QColor commandColor = mpCommandColorTree->getCellColor(pItem, 1);
            pWidget->setCommandColor(strCommand, commandColor);
         }

         ++iter;
      }
   }

   mpScriptingWindow->setCommandFont(mCommandFont);

   // Output text
   mpScriptingWindow->setOutputColor(mpOutputColorButton->getColor());
   mpScriptingWindow->setOutputFont(mOutputFont);
   mpScriptingWindow->setErrorColor(mpErrorColorButton->getColor());
   mpScriptingWindow->setErrorFont(mErrorFont);

   return true;
}

const string& PropertiesScriptingWindow::getName()
{
   static string name = "Scripting Window Properties";
   return name;
}

const string& PropertiesScriptingWindow::getPropertiesName()
{
   static string propertiesName = "Scripting Window";
   return propertiesName;
}

const string& PropertiesScriptingWindow::getDescription()
{
   static string description = "General setting properties of the scripting window";
   return description;
}

const string& PropertiesScriptingWindow::getShortDescription()
{
   static string description;
   return description;
}

const string& PropertiesScriptingWindow::getCreator()
{
   static string creator = "Ball Aerospace & Technologies Corp.";
   return creator;
}

const string& PropertiesScriptingWindow::getCopyright()
{
   static string copyright = APP_COPYRIGHT_MSG;
   return copyright;
}

const string& PropertiesScriptingWindow::getVersion()
{
   static string version = APP_VERSION_NUMBER;
   return version;
}

const string& PropertiesScriptingWindow::getDescriptorId()
{
   static string id = "{56BAF1B6-3CA0-4E45-8BE1-903DE297C327}";
   return id;
}

bool PropertiesScriptingWindow::isProduction()
{
   return APP_IS_PRODUCTION_RELEASE;
}

void PropertiesScriptingWindow::setCommandFont()
{
   bool bSuccess = false;

   QFont commandFont = QFontDialog::getFont(&bSuccess, mCommandFont, this);
   if (bSuccess == true)
   {
      mCommandFont = commandFont;
   }
}

void PropertiesScriptingWindow::setOutputFont()
{
   bool bSuccess = false;

   QFont outputFont = QFontDialog::getFont(&bSuccess, mOutputFont, this);
   if (bSuccess == true)
   {
      mOutputFont = outputFont;
   }
}

void PropertiesScriptingWindow::setErrorFont()
{
   bool bSuccess = false;

   QFont errorFont = QFontDialog::getFont(&bSuccess, mErrorFont, this);
   if (bSuccess == true)
   {
      mErrorFont = errorFont;
   }
}
