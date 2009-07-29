/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLayout>

#include "SplashScreen.h"
#include "AppVersion.h"
#include "ConfigurationSettingsImp.h"
#include "DateTime.h"
#include "Progress.h"
#include "Service.h"
#include "Slot.h"
#include "StringUtilities.h"

#include <string>
using namespace std;

SplashScreen::SplashScreen(Progress* pProgress) :
   QWidget(NULL, Qt::SplashScreen),
   mpProgress(pProgress),
   mpProgressLabel(NULL),
   mpProgressBar(NULL)
{
   // Get the version and build date
   QString strVersion;
   bool bProductionRelease = false;
   QString strReleaseType;

   ConfigurationSettingsImp* pConfigSettings =
      dynamic_cast<ConfigurationSettingsImp*>(Service<ConfigurationSettings>().get());
   strVersion = QString::fromStdString(pConfigSettings->getVersion());
   strVersion += " Build " + QString::fromStdString(pConfigSettings->getBuildRevision());

   bProductionRelease = pConfigSettings->isProductionRelease();
   strReleaseType = QString::fromStdString(
      StringUtilities::toDisplayString(pConfigSettings->getReleaseType()));

   QString strReleaseDescription = QString::fromStdString(pConfigSettings->getReleaseDescription());

   QFont normalFont = QApplication::font();
   normalFont.setPointSize(10);

   QFont boldFont = QApplication::font();
   boldFont.setBold(true);
   boldFont.setPointSize(10);

   QPalette currentPalette = palette();
   currentPalette.setColor(QPalette::Window, Qt::white);
   setPalette(currentPalette);

   // Pixmap
   mpPixmapLabel = new QLabel(this);
   mpPixmapLabel->setMargin(0);
   mpPixmapLabel->setScaledContents(true);
   mpPixmapLabel->setPixmap(QPixmap(":/images/Splash"));

   // Information frame
   QFrame* pInfoFrame = new QFrame(this);
   pInfoFrame->setFrameStyle(QFrame::Box | QFrame::Plain);

   // Version
   QString strApplicationName = QString::fromStdString(APP_NAME);
   QLabel* pVersionLabel = new QLabel(strApplicationName + " Version:", pInfoFrame);
   pVersionLabel->setFont(boldFont);

   QLabel* pVersion = new QLabel(strVersion, pInfoFrame);
   pVersion->setFont(normalFont);

   // Progress
   mpProgressLabel = new QLabel(pInfoFrame);
   mpProgressLabel->setFont(normalFont);

   mpProgressBar = new QProgressBar(pInfoFrame);
   mpProgressBar->setTextVisible(false);
   mpProgressBar->setFixedSize(100, 16);
   mpProgressBar->hide();

   // Release information
   QWidget* pReleaseWidget = new QWidget(pInfoFrame);
   pReleaseWidget->setAutoFillBackground(true);

   QPalette releasePalette = pReleaseWidget->palette();
   releasePalette.setColor(QPalette::Window, Qt::red);
   releasePalette.setColor(QPalette::WindowText, Qt::white);
   pReleaseWidget->setPalette(releasePalette);

   QLabel* pDeveloperInfo = new QLabel(strReleaseType, pReleaseWidget);
   pDeveloperInfo->setFont(boldFont);

   QLabel* pReleaseInfo = new QLabel("Not for Production Use", pReleaseWidget);
   pReleaseInfo->setFont(boldFont);

   // Release description
   QWidget* pReleaseDescriptionWidget = new QWidget(pInfoFrame);
   pReleaseDescriptionWidget->setAutoFillBackground(true);

   QPalette releaseDescriptionPalette = pReleaseDescriptionWidget->palette();
   releaseDescriptionPalette.setColor(QPalette::Window, Qt::blue);
   releaseDescriptionPalette.setColor(QPalette::WindowText, Qt::white);
   pReleaseDescriptionWidget->setPalette(releaseDescriptionPalette);

   QLabel* pReleaseDescriptionInfo = new QLabel(strReleaseDescription, pReleaseDescriptionWidget);
   pReleaseDescriptionInfo->setFont(boldFont);
   pReleaseDescriptionInfo->setWordWrap(true);

   // Layout
   QHBoxLayout* pVersionLayout = new QHBoxLayout();
   pVersionLayout->setMargin(0);
   pVersionLayout->setSpacing(5);
   pVersionLayout->addSpacing(5);
   pVersionLayout->addWidget(pVersionLabel);
   pVersionLayout->addWidget(pVersion, 10);
   pVersionLayout->addSpacing(5);

   QHBoxLayout* pProgressLayout = new QHBoxLayout();
   pProgressLayout->setMargin(0);
   pProgressLayout->setSpacing(5);
   pProgressLayout->addSpacing(5);
   pProgressLayout->addWidget(mpProgressLabel, 10);
   pProgressLayout->addWidget(mpProgressBar);
   pProgressLayout->addSpacing(5);

   QHBoxLayout* pReleaseLayout = new QHBoxLayout(pReleaseWidget);
   pReleaseLayout->setMargin(0);
   pReleaseLayout->setSpacing(5);
   pReleaseLayout->addSpacing(5);
   pReleaseLayout->addWidget(pDeveloperInfo);
   pReleaseLayout->addStretch();
   pReleaseLayout->addWidget(pReleaseInfo);
   pReleaseLayout->addSpacing(5);

   QHBoxLayout* pReleaseDescriptionLayout = new QHBoxLayout(pReleaseDescriptionWidget);
   pReleaseDescriptionLayout->setMargin(0);
   pReleaseDescriptionLayout->setSpacing(5);
   pReleaseDescriptionLayout->addSpacing(5);
   pReleaseDescriptionLayout->addWidget(pReleaseDescriptionInfo);
   pReleaseDescriptionLayout->addSpacing(5);

   QVBoxLayout* pInfoLayout = new QVBoxLayout(pInfoFrame);
   pInfoLayout->setMargin(0);
   pInfoLayout->setSpacing(0);
   pInfoLayout->addWidget(pReleaseWidget);
   pInfoLayout->addWidget(pReleaseDescriptionWidget);
   pInfoLayout->addLayout(pVersionLayout);
   pInfoLayout->addLayout(pProgressLayout);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(0);
   pLayout->addWidget(mpPixmapLabel, 10);
   pLayout->addWidget(pInfoFrame);

   // Initialization
   pDeveloperInfo->setHidden(strReleaseType.isEmpty());
   pReleaseInfo->setHidden(bProductionRelease);
   pReleaseDescriptionInfo->setHidden(strReleaseDescription.isEmpty());

   mpRotateImageTimer = new QTimer(this);
   mpRotateImageTimer->setInterval(750);
   connect(mpRotateImageTimer, SIGNAL(timeout()), this, SLOT(rotateImage()));

   if (mpProgress != NULL)
   {
      mpProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &SplashScreen::update));
   }
}

SplashScreen::~SplashScreen()
{
   if (mpProgress != NULL)
   {
      mpProgress->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &SplashScreen::update));
   }
}

void SplashScreen::setSplashImages(const list<string>& imagePaths)
{
   mImagePaths = imagePaths;
   mImagePaths.push_back("END_IMAGE_ROTATION");
}

void SplashScreen::rotateImage()
{
   bool lookingForGoodImage = true;
   while (lookingForGoodImage)
   {
      if (mImagePaths.empty())
      {
         return;
      }
      string imagePath = mImagePaths.front();
      mImagePaths.pop_front();
      if (imagePath != "END_IMAGE_ROTATION")
      {
         QPixmap splashImage(QString::fromStdString(imagePath));
         if (!splashImage.isNull())
         {
            lookingForGoodImage = false;
            mpPixmapLabel->setPixmap(splashImage);
            repaint();
            QApplication::flush();
         }
      }
   }
}

bool SplashScreen::canClose()
{
   if (isVisible())
   {
      QApplication::processEvents();
      return mImagePaths.empty();
   }
   else
   {
      return true;
   }
}

void SplashScreen::update(Subject &subject, const string &signal, const boost::any &v)
{
   if (dynamic_cast<Progress*>(&subject) == mpProgress)
   {
      // Get the current progress
      string progressText;
      int iPercent = 0;
      ReportingLevel level = NORMAL;

      mpProgress->getProgress(progressText, iPercent, level);

      // Update the text
      if (progressText.empty() == false)
      {
         mpProgressLabel->setText(QString::fromStdString(progressText));
      }

      // Update the progress
      if (iPercent == 0)
      {
         mpProgressBar->hide();
      }
      else
      {
         mpProgressBar->setValue(iPercent);
         mpProgressBar->show();
      }

      // Force a repaint since there is no event loop yet to process events
      repaint();
   }
}

bool SplashScreen::event(QEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->type() == QEvent::Polish)
      {
         adjustSize();

         QRect rcDesktop = QApplication::desktop()->screenGeometry();
         move(rcDesktop.center() - rect().center());
         mpRotateImageTimer->start();
      }
   }

   return QWidget::event(pEvent);
}

void SplashScreen::mousePressEvent(QMouseEvent* pEvent)
{
   hide();
   QWidget::mousePressEvent(pEvent);
}
