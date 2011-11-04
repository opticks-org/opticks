/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QMovie>
#include <QtGui/QVBoxLayout>

#include "ProgressDlg.h"

#include "AppAssert.h"
#include "Executable.h"
#include "DataVariant.h"

#include <string>
using namespace std;

ProgressDlg::ProgressDlg(const QString& strCaption, QWidget* parent) :
   QDialog(parent),
   mpProgressLabel(NULL),
   mpProgressBar(NULL),
   mpCancel(NULL),
   mpWarningWidget(NULL),
   mpWarningEdit(NULL),
   mbComplete(false),
   mpProgressObject(NULL),
   mpPlugIn(NULL),
   mbHasAbort(false),
   mbAborted(false)
{
   setObjectName("Progress Dialog");
   mbAutoClose = Progress::getSettingAutoClose();

   // Bitmap
   QLabel* pBitmap = new QLabel(this);
   mpProgressMovie = new QMovie(this);
   mpProgressMovie->setFileName(":/images/application-progress");
   pBitmap->setMovie(mpProgressMovie);

   // Progress label
   mpProgressLabel = new QLabel(this);
   mpProgressLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   mpProgressLabel->setWordWrap(true);
   mpProgressLabel->setMinimumWidth(225);

   QHBoxLayout* pMessageLayout = new QHBoxLayout();
   pMessageLayout->setMargin(0);
   pMessageLayout->setSpacing(10);
   pMessageLayout->addWidget(pBitmap);
   pMessageLayout->addWidget(mpProgressLabel, 10);

   // Progress bar
   mpProgressBar = new QProgressBar(this);
   mpProgressBar->setRange(0, 100);

   // Cancel button
   mpCancel = new QPushButton("&Cancel", this);
   mpCancel->setDefault(true);
   mpCancel->setEnabled(false);

   // Warnings edit
   mpWarningWidget = new QWidget(this);
   mpWarningWidget->hide();

   QLabel* pWarningLabel = new QLabel("Warnings:", mpWarningWidget);
   mpWarningEdit = new QTextEdit(mpWarningWidget);
   mpWarningEdit->setLineWrapMode(QTextEdit::WidgetWidth);
   mpWarningEdit->setReadOnly(true);

   QPalette pltWarning = mpWarningEdit->palette();
   pltWarning.setColor(QPalette::Base, Qt::lightGray);
   mpWarningEdit->setPalette(pltWarning);

   QVBoxLayout* pWarningLayout = new QVBoxLayout(mpWarningWidget);
   pWarningLayout->setMargin(0);
   pWarningLayout->setSpacing(5);
   pWarningLayout->addWidget(pWarningLabel);
   pWarningLayout->addWidget(mpWarningEdit);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addLayout(pMessageLayout);
   pLayout->addWidget(mpProgressBar);
   pLayout->addWidget(mpCancel, 0, Qt::AlignRight);
   pLayout->addWidget(mpWarningWidget);

   // Initialization
   mpProgressMovie->start();
   setWindowTitle(strCaption);
   setModal(true);
   setMinimumSize(300, 125);
   setAttribute(Qt::WA_DeleteOnClose);

   // Connections
   connect(mpCancel, SIGNAL(clicked()), this, SLOT(close()));
}

ProgressDlg::~ProgressDlg()
{
}

void ProgressDlg::progressUpdated(Subject& subject, const string& signal, const boost::any& value)
{
   mpProgressObject = NULL;
   mpPlugIn = NULL;

   // Get the current Progress values
   ProgressAdapter* pProgressObject = dynamic_cast<ProgressAdapter*> (&subject);
   VERIFYNRV(pProgressObject != NULL);

   string label = "";
   int iPercent = 0;
   ReportingLevel eLevel = NORMAL;
   pProgressObject->getProgress(label, iPercent, eLevel);

   // Update the dialog
   mpProgressObject = pProgressObject;
   mpPlugIn = dynamic_cast<Executable*>(pProgressObject->getPlugIn());

   // Set the label text if there is new text
   QString strLabel;
   if (label.empty() == false)
   {
      strLabel = QString::fromStdString(label);
   }

   if (strLabel.isEmpty() == false)
   {
      if (eLevel == WARNING || eLevel == ERRORS)
      {
         bool bVisible = mpWarningWidget->isVisible();
         if (bVisible == false)
         {
            mpWarningWidget->show();
            setMinimumSize(300, 250);
         }

         mpWarningEdit->append(strLabel);
      }
      if (eLevel != WARNING)
      {
         QString strOldLabel = mpProgressLabel->text();
         if (strLabel != strOldLabel)
         {
            mpProgressLabel->setText(strLabel);
         }
      }
   }

   // Set the progress percentage
   bool bComplete = true;

   if (eLevel == ABORT)
   {
      if (mbAutoClose == false)
      {
         mpProgressBar->reset();
         mpProgressBar->setValue(0);
      }
   }
   else if (eLevel == ERRORS)
   {
      if (iPercent < 0)
      {
         iPercent = 0;
      }

      int iOldPercent = mpProgressBar->value();
      if (iPercent < iOldPercent)
      {
         mpProgressBar->reset();
      }

      mpProgressBar->setValue(iPercent);
   }
   else
   {
      // Reset the internal abort flag
      mbAborted = false;

      if (eLevel == WARNING)
      {
         // Ignore the input percentage for warnings
         iPercent = mpProgressBar->value();
      }
      else
      {
         // Set the progress only if it is a new value
         int iOldPercent = mpProgressBar->value();
         if (iPercent < iOldPercent)
         {
            mpProgressBar->reset();

            if (iPercent >= 0)
            {
               mpProgressBar->setValue(iPercent);
            }
         }
         else if (iPercent > iOldPercent)
         {
            mpProgressBar->setValue(iPercent);
         }
      }

      if (iPercent != mpProgressBar->maximum())
      {
         bComplete = false;
      }
   }

   // Update the process flag and Cancel button
   setProcessComplete(bComplete);

   // Have the application process events to check if the user clicked the Cancel button
   qApp->processEvents();

   // Ensure the entire progress message is displayed and the warning widget is correctly displayed
   adjustSize();

   // Display the dialog as necessary
   if ((bComplete == true) && (mbAutoClose == true) && (eLevel != ERRORS) &&
      (mpWarningWidget->isVisible() == false))
   {
      if (isVisible() == true)
      {
         hide();
      }
   }
   else if (isVisible() == false)
   {
      show();
   }
}

void ProgressDlg::progressDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   mpProgressObject = NULL;
   mpPlugIn = NULL;

   // Get the current Progress values
   ProgressAdapter* pProgressObject = dynamic_cast<ProgressAdapter*> (&subject);
   VERIFYNRV(pProgressObject != NULL);

   string label = "";
   int iPercent = 0;
   ReportingLevel eLevel = NORMAL;
   pProgressObject->getProgress(label, iPercent, eLevel);

   if (((mbAutoClose == true) && (eLevel != ERRORS) && (mpWarningWidget->isVisible() == false)) ||
      (isVisible() == false))
   {
      delete this;
   }
}

void ProgressDlg::enableAbort(bool bEnable)
{
   mbHasAbort = bEnable;

   if (mbHasAbort == false)
   {
      mbAborted = false;
   }
}

bool ProgressDlg::hasAbort()
{
   return mbHasAbort;
}

bool ProgressDlg::isAborted()
{
   return mbAborted;
}

void ProgressDlg::reject()
{
   close();
}

void ProgressDlg::closeEvent(QCloseEvent* e)
{
   if (mbComplete == false)
   {
      if (mpPlugIn != NULL)
      {
         mpPlugIn->abort();
      }
      else
      {
         mbAborted = true;
      }

      emit aborted();
   }

   mpWarningEdit->clear();
   mpWarningWidget->hide();
   setMinimumSize(300, 125);

   if (mpProgressObject == NULL)
   {
      if (e != NULL)
      {
         e->accept();
      }
   }
   else
   {
      if (e != NULL)
      {
         e->ignore();
      }

      hide();
   }
}

void ProgressDlg::setProcessComplete(bool bComplete)
{
   // Set the complete flag to prevent aborting
   mbComplete = bComplete;

   if (bComplete)
   {
      mpProgressMovie->stop();
   }
   else
   {
      mpProgressMovie->start();
   }

   // Update the cancel/close button text
   QString strButtonText = "&Close";
   if (bComplete == false)
   {
      strButtonText = "&Cancel";
   }

   mpCancel->setText(strButtonText);

   // Enable the button
   bool bEnable = false;
   if (bComplete == true)
   {
      bEnable = true;
   }
   else if (mpPlugIn != NULL)
   {
      bEnable = mpPlugIn->hasAbort();
   }
   else
   {
      bEnable = mbHasAbort;
   }

   mpCancel->setEnabled(bEnable);
}
