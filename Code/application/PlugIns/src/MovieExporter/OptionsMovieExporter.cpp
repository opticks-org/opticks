/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsMovieExporter.h"

#include "LabeledSection.h"
#include "SessionManager.h"

#include <avcodec.h>
#include <QtCore/QStringList>
#include <QtGui/QBitmap>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

namespace
{
   /* XPM */
   const char *const lock_xpm[] = {
      "32 22 62 1",
      " 	c None",
      ".	c #FFFFFF",
      "+	c #FBFEFF",
      "@	c #FAFAFD",
      "#	c #E8DB99",
      "$	c #E1E4EC",
      "%	c #E0ECF6",
      "&	c #D5F0F1",
      "*	c #D1C795",
      "=	c #D0D7E6",
      "-	c #CEC48B",
      ";	c #C9FFFF",
      ">	c #C7D0D9",
      ",	c #C6D2ED",
      "'	c #C5CFD8",
      ")	c #C3BB8C",
      "!	c #C1B784",
      "~	c #C0CBD4",
      "{	c #B6C2CC",
      "]	c #B3BFCA",
      "^	c #ACB9C4",
      "/	c #A9B7C2",
      "(	c #A2B0BC",
      "_	c #A1AFB9",
      ":	c #9FAEBA",
      "<	c #9CABB8",
      "[	c #97A7B4",
      "}	c #95A5B2",
      "|	c #92A2B0",
      "1	c #8D9EAC",
      "2	c #8B9CAA",
      "3	c #8A9A9F",
      "4	c #889AA8",
      "5	c #8897A2",
      "6	c #878C99",
      "7	c #8395A4",
      "8	c #8093A2",
      "9	c #7E91A0",
      "0	c #7D9A9F",
      "a	c #7C8396",
      "b	c #798D9C",
      "c	c #768A9A",
      "d	c #738898",
      "e	c #71838F",
      "f	c #6C8192",
      "g	c #697F90",
      "h	c #656351",
      "i	c #62798A",
      "j	c #607789",
      "k	c #5E7587",
      "l	c #5E727E",
      "m	c #4E6370",
      "n	c #4B4B45",
      "o	c #4B4B43",
      "p	c #4B4A42",
      "q	c #415864",
      "r	c #374F5C",
      "s	c #2F4755",
      "t	c #2F343A",
      "u	c #294250",
      "v	c #23262C",
      "w	c #161A25",
      "                                ",
      "                                ",
      "                                ",
      "            nwwwwhh             ",
      "          tt3...$66v            ",
      "          tt3...$66v            ",
      "         n00&wwww==ann          ",
      "         w;;w    ww,ww          ",
      "         w;;w    ww,ww          ",
      "         w;;w    ww,ww          ",
      "       wwwwwwwwwwwwwwww         ",
      "       wwwwwwwwwwwwwwww         ",
      "       ww'~~{^^([117bbw         ",
      "       wwsssrqqmlee5__w         ",
      "       wwsssrqqmlee5__w         ",
      "       ww]//:[[17ccfiiw         ",
      "       wwsssrqqmlee5__w         ",
      "       wwsssrqqmlee5__w         ",
      "       ww:111bbcgjjkkkw         ",
      "       wwwwwwwwwwwwwwww         ",
      "       wwwwwwwwwwwwwwww         ",
      "                                "};

      /* XPM */
      const char *const unlock_xpm[] = {
         "32 22 59 1",
         " 	c None",
         ".	c #FFFFFF",
         "+	c #FBFEFF",
         "@	c #F9FAFD",
         "#	c #E1E4EC",
         "$	c #E0ECF6",
         "%	c #D5F0F1",
         "&	c #D0D7E6",
         "*	c #C9FFFF",
         "=	c #C7D0D9",
         "-	c #C6D2ED",
         ";	c #C6D0D9",
         ">	c #C5CFD8",
         ",	c #C0CBD4",
         "'	c #B6C2CC",
         ")	c #B3BFCA",
         "!	c #B1BAC3",
         "~	c #ACB9C4",
         "{	c #A9B7C2",
         "]	c #A6AEB8",
         "^	c #A2B0BC",
         "/	c #A1AFB9",
         "(	c #9FAEBA",
         "_	c #9CABB8",
         ":	c #97A7B4",
         "<	c #95A5B2",
         "[	c #92A2B0",
         "}	c #8D9EAC",
         "|	c #8B9CAA",
         "1	c #8A9A9F",
         "2	c #889AA8",
         "3	c #8898A2",
         "4	c #8395A4",
         "5	c #8093A2",
         "6	c #7E91A0",
         "7	c #7D9A9F",
         "8	c #7C8396",
         "9	c #7A7F8C",
         "0	c #798D9C",
         "a	c #768A9A",
         "b	c #738898",
         "c	c #71838F",
         "d	c #6C8192",
         "e	c #697F90",
         "f	c #62798A",
         "g	c #607789",
         "h	c #5E7587",
         "i	c #5E727E",
         "j	c #585F69",
         "k	c #4E6370",
         "l	c #4D535D",
         "m	c #424852",
         "n	c #415864",
         "o	c #374F5C",
         "p	c #2F4755",
         "q	c #2C333D",
         "r	c #294250",
         "s	c #212530",
         "t	c #161A25",
         "                                ",
         "                                ",
         "                                ",
         "                 mmttttjj       ",
         "                q11...#99s      ",
         "                q11...#99s      ",
         "               m7%%tttt&&8ll    ",
         "               t*tt    tt-tt    ",
         "               t*tt    tt-tt    ",
         "               t*tt    tt-tt    ",
         "    tttttttttttttttt   ttttt    ",
         "    tttttttttttttttt   ttttt    ",
         "    tt>,,'~~^::}400t            ",
         "    ttppponnkiic3]]t            ",
         "    ttppponnkiic3]]t            ",
         "    tt){{(::}44adfft            ",
         "    ttppponnkiic3((t            ",
         "    ttppponnkiic3((t            ",
         "    tt(}}}00aeeghhht            ",
         "    tttttttttttttttt            ",
         "    tttttttttttttttt            ",
         "                                "};
         
   QStringList defaultFramerateItems;
};

OptionsMovieExporter::OptionsMovieExporter() :
   LabeledSectionGroup(NULL), mCurrentResolutionX(0), mCurrentResolutionY(0)
{
   if(defaultFramerateItems.isEmpty())
   {
      defaultFramerateItems << "Custom..."
                            << " 1.00fps - 1/1"
                            << "15.00fps - 15/1"
                            << "24.00fps - 24/1"
                            << "29.97fps - 30000/1001"
                            << "30.00fps - 30/1";
   }
   
   // Resolution section
   QWidget *pResolutionLayoutWidget = new QWidget(this);
   mpUseViewResolution = new QCheckBox("Use View Resolution", pResolutionLayoutWidget);
   mpResolutionX = new QLineEdit(pResolutionLayoutWidget);
   mpResolutionY = new QLineEdit(pResolutionLayoutWidget);
   QIntValidator *pValidator = new QIntValidator(pResolutionLayoutWidget);
   pValidator->setBottom(2);
   mpResolutionX->setValidator(pValidator);
   mpResolutionY->setValidator(pValidator);

   QPixmap lockPixmap(lock_xpm);
   lockPixmap.setMask(lockPixmap.createHeuristicMask());
   mpLockIcon = new QIcon(lockPixmap);
   QPixmap unlockPixmap(unlock_xpm);
   unlockPixmap.setMask(unlockPixmap.createHeuristicMask());
   mpUnlockIcon = new QIcon(unlockPixmap);
   mpResolutionAspectLock = new QPushButton(*mpLockIcon, QString(), pResolutionLayoutWidget);
   mpResolutionAspectLock->setCheckable(true);
   mpResolutionAspectLock->setChecked(true);
   mpResolutionAspectLock->setToolTip("Lock the resolution aspect ratio.");

   connect(mpUseViewResolution, SIGNAL(toggled(bool)), this, SLOT(setUseViewResolution(bool)));
   connect(mpResolutionX, SIGNAL(editingFinished()), this, SLOT(checkResolutionX()));
   connect(mpResolutionY, SIGNAL(editingFinished()), this, SLOT(checkResolutionY()));
   connect(mpResolutionAspectLock, SIGNAL(toggled(bool)), this, SLOT(aspectLockToggled(bool)));

   QGridLayout *pResolutionLayout = new QGridLayout(pResolutionLayoutWidget);
   pResolutionLayout->addWidget(mpUseViewResolution, 0, 0, 1, 2, Qt::AlignLeft);
   pResolutionLayout->addWidget(mpResolutionX, 1, 0);
   pResolutionLayout->addWidget(mpResolutionY, 2, 0);
   pResolutionLayout->addWidget(mpResolutionAspectLock, 1, 1, 2, 1, Qt::AlignCenter);
   pResolutionLayout->setColumnStretch(4, 10);

   LabeledSection *pResolutionSection = new LabeledSection(pResolutionLayoutWidget, "Output Resolution Options", this);

   setResolution(OptionsMovieExporter::getSettingWidth(), OptionsMovieExporter::getSettingHeight());

   // Bitrate/Framerate section
   QWidget *pRateLayoutWidget = new QWidget(this);
   QLabel *pBitrateLabel = new QLabel("Bitrate", pRateLayoutWidget);
   mpBitrate = new QSlider(Qt::Horizontal, pRateLayoutWidget);
   mpBitrate->setMinimum(1);
   mpBitrate->setMaximum(9800);
   mpBitrate->setPageStep(100);
   mpBitrate->setSingleStep(1);
   mpBitrate->setTracking(true);
   mpBitrate->setTickInterval(1000);
   mpBitrate->setTickPosition(QSlider::TicksBelow);
   mpBitrateValue = new QLabel(pRateLayoutWidget);
   QLabel *pFramerateLabel = new QLabel("Frame Rate", pRateLayoutWidget);
   mpFramerateNum = new QSpinBox(pRateLayoutWidget);
   mpFramerateNum->setRange(1, 100000);
   mpFramerateNum->setSingleStep(10);
   QLabel *pFramerateSlash = new QLabel("frames /", pRateLayoutWidget);
   mpFramerateDen = new QSpinBox(pRateLayoutWidget);
   mpFramerateDen->setRange(1, 100000);
   mpFramerateDen->setSingleStep(10);
   QLabel *pFramerateUnits = new QLabel("s", pRateLayoutWidget);
   mpFramerateList = new QComboBox(pRateLayoutWidget);

   QGridLayout *pRateLayout = new QGridLayout(pRateLayoutWidget);
   pRateLayout->setMargin(0);
   pRateLayout->setSpacing(5);
   pRateLayout->addWidget(pBitrateLabel, 0, 0);
   pRateLayout->addWidget(mpBitrate, 0, 1, 1, 2);
   pRateLayout->addWidget(mpBitrateValue, 0, 3);
   pRateLayout->addWidget(pFramerateLabel, 1, 0);
   pRateLayout->addWidget(mpFramerateNum, 1, 1);
   pRateLayout->addWidget(pFramerateSlash, 1, 2);
   pRateLayout->addWidget(mpFramerateDen, 1, 3);
   pRateLayout->addWidget(pFramerateUnits, 1, 4);
   pRateLayout->addWidget(mpFramerateList, 2, 1, 1, 3);
   pRateLayout->setColumnStretch(5, 10);

   mpBitrateValue->setMinimumWidth(
      QFontMetrics(mpBitrateValue->font()).width("10000 kbit/s"));
   connect(mpBitrate, SIGNAL(valueChanged(int)), this, SLOT(updateBitrate(int)));
   connect(mpFramerateList, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(frameRateListChanged(const QString&)));
   setFramerates(std::vector<boost::rational<int> >());

   LabeledSection *pRateSection = new LabeledSection(pRateLayoutWidget, "Bitrate and Framerate Options", this);

   setBitrate(OptionsMovieExporter::getSettingBitrate());
   setFramerate(boost::rational<int>(OptionsMovieExporter::getSettingFramerateNum(),
                                     OptionsMovieExporter::getSettingFramerateDen()));

   // Advanced Options
   QWidget *pAdvancedLayoutWidget = new QWidget(this);
   QLabel *pMeMethodLabel = new QLabel("Motion Estimation Method", pAdvancedLayoutWidget);
   mpMeMethod = new QComboBox(pAdvancedLayoutWidget);
   mpMeMethod->addItems(QStringList() << "Zero" << "PHODS" << "Log" << "X1" << "EPZS" << "Full");
   mpMeMethod->setEditable(false);
   QLabel *pGopSizeLabel = new QLabel("GOP Size", pAdvancedLayoutWidget);
   mpGopSize = new QSpinBox(pAdvancedLayoutWidget);
   mpGopSize->setMinimum(0);
   mpGopSize->setMaximum(30);
   QLabel *pQCompressLabel = new QLabel("Q-scale Compression", pAdvancedLayoutWidget);
   mpQCompress = new QDoubleSpinBox(pAdvancedLayoutWidget);
   mpQCompress->setToolTip("Amount of q-scale change between easy & hard scenes");
   mpQCompress->setMinimum(0.0);
   mpQCompress->setMaximum(1.0);
   mpQCompress->setSingleStep(0.1);
   QLabel *pQBlurLabel = new QLabel("Q-scale Smoothing", pAdvancedLayoutWidget);
   mpQBlur = new QDoubleSpinBox(pAdvancedLayoutWidget);
   mpQBlur->setToolTip("Amount of q-scale smoothing over time");
   mpQBlur->setMinimum(0.0);
   mpQBlur->setMaximum(1.0);
   mpQBlur->setSingleStep(0.1);
   QLabel *pQuantizerLabel = new QLabel("Quantizer Range", pAdvancedLayoutWidget);
   mpQMinimum = new QSpinBox(pAdvancedLayoutWidget);
   mpQMinimum->setMinimum(2);
   mpQMinimum->setMaximum(31);
   mpQMaximum = new QSpinBox(pAdvancedLayoutWidget);
   mpQMaximum->setMinimum(2);
   mpQMaximum->setMaximum(31);
   QLabel *pQDiffMaximumLabel = new QLabel("Maximum Quantizer Difference", pAdvancedLayoutWidget);
   mpQDiffMaximum = new QSpinBox(pAdvancedLayoutWidget);
   mpQDiffMaximum->setMinimum(2);
   mpQDiffMaximum->setMaximum(31);
   QLabel *pMaxBFramesLabel = new QLabel("Maximum B-Frames", pAdvancedLayoutWidget);
   mpMaxBFrames = new QSpinBox(pAdvancedLayoutWidget);
   mpMaxBFrames->setMinimum(0);
   mpMaxBFrames->setMaximum(30);
   QLabel *pBQuantLabel = new QLabel("B-Frame Quantizer Factor/Offset", pAdvancedLayoutWidget);
   mpBQuantFactor = new QDoubleSpinBox(pAdvancedLayoutWidget);
   mpBQuantFactor->setToolTip("Q-scale factor between IP and B frames");
   mpBQuantFactor->setMinimum(0.0);
   mpBQuantFactor->setMaximum(31.0);
   mpBQuantFactor->setSingleStep(0.1);
   mpBQuantOffset = new QDoubleSpinBox(pAdvancedLayoutWidget);
   mpBQuantOffset->setToolTip("Q-scale offset between IP and B frames");
   mpBQuantOffset->setMinimum(-1.0);
   mpBQuantOffset->setMaximum(31.0);
   mpBQuantOffset->setSingleStep(0.1);
   QLabel *pDiaSizeLabel = new QLabel("Motion Search Range", pAdvancedLayoutWidget);
   mpDiaSize = new QSpinBox(pAdvancedLayoutWidget);
   mpDiaSize->setToolTip("-1 for fast encode, 2-4 for better quality and compression");
   mpDiaSize->setMinimum(-10);
   mpDiaSize->setMaximum(10);
   QGroupBox *pFlagsGroup = new QGroupBox("Flags", pAdvancedLayoutWidget);
   mpQScale = new QCheckBox("Fixed Quantizer Scale", pFlagsGroup);
   mpQPel = new QCheckBox("Quarter Pixel Motion Estimation", pFlagsGroup);
   mpGmc = new QCheckBox("Global Motion Compensation", pFlagsGroup);
   mpNormalizeAqp = new QCheckBox("Normalize Adaptive Quantization", pFlagsGroup);
   mpTrellis = new QCheckBox("Trellis Quantization", pFlagsGroup);
   mpAcPred = new QCheckBox("MPEG-4 AC Prediction", pFlagsGroup);
   mpCbpRd = new QCheckBox("CBP Rate Distortion", pFlagsGroup);
   mpQpRd = new QCheckBox("QP Rate Distortion", pFlagsGroup);
   mpObmc = new QCheckBox("Overlapped Block Motion Compensation", pFlagsGroup);
   mpClosedGop = new QCheckBox("Close GOP", pFlagsGroup);
   QGridLayout *pFlagsLayout = new QGridLayout(pFlagsGroup);
   pFlagsLayout->addWidget(mpQScale, 0, 0);
   pFlagsLayout->addWidget(mpQPel, 1, 0);
   pFlagsLayout->addWidget(mpGmc, 2, 0);
   pFlagsLayout->addWidget(mpNormalizeAqp, 3, 0);
   pFlagsLayout->addWidget(mpTrellis, 4, 0);
   pFlagsLayout->addWidget(mpAcPred, 0, 1);
   pFlagsLayout->addWidget(mpCbpRd, 1, 1);
   pFlagsLayout->addWidget(mpQpRd, 2, 1);
   pFlagsLayout->addWidget(mpObmc, 3, 1);
   pFlagsLayout->addWidget(mpClosedGop, 4, 1);
   pFlagsGroup->setLayout(pFlagsLayout);

   QGridLayout *pAdvancedLayout = new QGridLayout(pAdvancedLayoutWidget);
   pAdvancedLayout->setMargin(0);
   pAdvancedLayout->setSpacing(5);
   pAdvancedLayout->addWidget(pMeMethodLabel, 0, 0);
   pAdvancedLayout->addWidget(mpMeMethod, 0, 1);
   pAdvancedLayout->addWidget(pGopSizeLabel, 1, 0);
   pAdvancedLayout->addWidget(mpGopSize, 1, 1);
   pAdvancedLayout->addWidget(pQCompressLabel, 2, 0);
   pAdvancedLayout->addWidget(mpQCompress, 2, 1);
   pAdvancedLayout->addWidget(pQBlurLabel, 3, 0);
   pAdvancedLayout->addWidget(mpQBlur, 3, 1);
   pAdvancedLayout->addWidget(pQuantizerLabel, 4, 0);
   pAdvancedLayout->addWidget(mpQMinimum, 4, 1);
   pAdvancedLayout->addWidget(mpQMaximum, 4, 2);
   pAdvancedLayout->addWidget(pQDiffMaximumLabel, 5, 0);
   pAdvancedLayout->addWidget(mpQDiffMaximum, 5, 1);
   pAdvancedLayout->addWidget(pMaxBFramesLabel, 6, 0);
   pAdvancedLayout->addWidget(mpMaxBFrames, 6, 1);
   pAdvancedLayout->addWidget(pBQuantLabel, 7, 0);
   pAdvancedLayout->addWidget(mpBQuantFactor, 7, 1);
   pAdvancedLayout->addWidget(mpBQuantOffset, 7, 2);
   pAdvancedLayout->addWidget(pDiaSizeLabel, 8, 0);
   pAdvancedLayout->addWidget(mpDiaSize, 8, 1);
   pAdvancedLayout->addWidget(pFlagsGroup, 9, 0, 1, 3);
   pAdvancedLayout->setColumnStretch(3, 10);

   LabeledSection *pAdvancedSection = new LabeledSection(pAdvancedLayoutWidget, "Advanced Options", this);
   pAdvancedSection->collapse();

   setMeMethod(OptionsMovieExporter::getSettingMeMethod());
   setGopSize(OptionsMovieExporter::getSettingGopSize());
   setQCompress(OptionsMovieExporter::getSettingQCompress());
   setQBlur(OptionsMovieExporter::getSettingQBlur());
   setQMinimum(OptionsMovieExporter::getSettingQMinimum());
   setQMaximum(OptionsMovieExporter::getSettingQMaximum());
   setQDiffMaximum(OptionsMovieExporter::getSettingQDiffMaximum());
   setMaxBFrames(OptionsMovieExporter::getSettingMaxBFrames());
   setBQuantFactor(OptionsMovieExporter::getSettingBQuantFactor());
   setBQuantOffset(OptionsMovieExporter::getSettingBQuantOffset());
   setDiaSize(OptionsMovieExporter::getSettingDiaSize());
   setFlags(OptionsMovieExporter::getSettingFlags());

   // Save Settings
   mpSaveSettings = new QCheckBox("Save Settings");
   mpSettingsSection = new LabeledSection(mpSaveSettings, "Settings", this);
   mpSettingsSection->hide();

   // Initialization
   addSection(pResolutionSection);
   addSection(pRateSection);
   addSection(pAdvancedSection);
   addSection(mpSettingsSection);
   addStretch(10);
   setSizeHint(500, 300);
}

OptionsMovieExporter::~OptionsMovieExporter()
{
   delete mpLockIcon;
   delete mpUnlockIcon;
}

void OptionsMovieExporter::applyChanges()
{
   if (mpSettingsSection->isHidden() || mpSaveSettings->isChecked())
   {
      unsigned int width=0, height=0;
      getResolution(width, height);
      OptionsMovieExporter::setSettingWidth(width);
      OptionsMovieExporter::setSettingHeight(height);
      OptionsMovieExporter::setSettingBitrate(mpBitrate->value());
      OptionsMovieExporter::setSettingFramerateNum(mpFramerateNum->value());
      OptionsMovieExporter::setSettingFramerateDen(mpFramerateDen->value());
      OptionsMovieExporter::setSettingMeMethod(getMeMethod());
      OptionsMovieExporter::setSettingGopSize(getGopSize());
      OptionsMovieExporter::setSettingQCompress(getQCompress());
      OptionsMovieExporter::setSettingQBlur(getQBlur());
      OptionsMovieExporter::setSettingQMinimum(getQMinimum());
      OptionsMovieExporter::setSettingQMaximum(getQMaximum());
      OptionsMovieExporter::setSettingQDiffMaximum(getQDiffMaximum());
      OptionsMovieExporter::setSettingMaxBFrames(getMaxBFrames());
      OptionsMovieExporter::setSettingBQuantFactor(getBQuantFactor());
      OptionsMovieExporter::setSettingBQuantOffset(getBQuantOffset());
      OptionsMovieExporter::setSettingDiaSize(getDiaSize());
      OptionsMovieExporter::setSettingFlags(getFlags());
   }
}

void OptionsMovieExporter::setPromptUserToSaveSettings(bool prompt)
{
   mpSettingsSection->setVisible(prompt);
}

void OptionsMovieExporter::getResolution(unsigned int &width, unsigned int &height) const
{
   if(mpUseViewResolution->isChecked())
   {
      width = height = 0;
   }
   else
   {
      width = mpResolutionX->text().toUInt();
      height = mpResolutionY->text().toUInt();
   }
}

void OptionsMovieExporter::setResolution(unsigned int width, unsigned int height)
{
   if(width == 0 && height == 0)
   {
      mpUseViewResolution->setChecked(true);
      return;
   }
   mpUseViewResolution->setChecked(false);
   const QValidator *pValidX = mpResolutionX->validator();
   const QValidator *pValidY = mpResolutionY->validator();
   int pos1 = 0;
   int pos2 = 0;
   if((pValidX == NULL || pValidX->validate(QString::number(width), pos1) == QValidator::Acceptable) &&
      (pValidY == NULL || pValidY->validate(QString::number(height), pos2) == QValidator::Acceptable))
   {
      mpResolutionX->setText(QString::number(width));
      mpResolutionY->setText(QString::number(height));
      checkResolutionX(true);
      checkResolutionY(true);
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

unsigned int OptionsMovieExporter::getBitrate() const
{
   return mpBitrate->value();
}

void OptionsMovieExporter::setBitrate(unsigned int bitrate)
{
   if((bitrate >= static_cast<unsigned int>(mpBitrate->minimum())) &&
      (bitrate <= static_cast<unsigned int>(mpBitrate->maximum())))
   {
      mpBitrate->setValue(bitrate);
   }
}

boost::rational<int> OptionsMovieExporter::getFramerate() const
{
   return boost::rational<int>(mpFramerateNum->value(), mpFramerateDen->value());
}

void OptionsMovieExporter::setFramerate(boost::rational<int> frameRate)
{
   int num = frameRate.numerator();
   int den = frameRate.denominator();
   if((num >= static_cast<int>(mpFramerateNum->minimum())) &&
      (den >= static_cast<int>(mpFramerateDen->minimum())) &&
      (frameRate <= boost::rational<int>(60, 1)))
   {
      int idx = mpFramerateList->findText(QString("%1/%2").arg(num).arg(den), Qt::MatchEndsWith);
      if(idx != -1)
      {
         mpFramerateList->setCurrentIndex(idx);
      }
      else
      {
         idx = mpFramerateList->findText("Custom...");
         if(idx != -1)
         {
            mpFramerateList->setCurrentIndex(idx);
            mpFramerateNum->setValue(num);
            mpFramerateDen->setValue(den);
         }
      }
   }
}

void OptionsMovieExporter::setFramerates(std::vector<boost::rational<int> > frameRates)
{
   mpFramerateList->clear();
   for(std::vector<boost::rational<int> >::iterator frit = frameRates.begin();
       frit != frameRates.end();
       ++frit)
   {
      mpFramerateList->addItem(QString("%1fps - %2/%3")
         .arg(boost::rational_cast<double>(*frit), 0, 'f', 2)
         .arg(frit->numerator())
         .arg(frit->denominator()));
   }
   if(mpFramerateList->count() == 0)
   {
      mpFramerateList->addItems(defaultFramerateItems);
   }
   mpFramerateList->setCurrentIndex(0);
}

string OptionsMovieExporter::getMeMethod() const
{
   return mpMeMethod->currentText().toStdString();
}

void OptionsMovieExporter::setMeMethod(const string &method)
{
   int idx = mpMeMethod->findText(QString::fromStdString(method));
   if(idx > -1)
   {
      mpMeMethod->setCurrentIndex(idx);
   }
}
int OptionsMovieExporter::getGopSize() const
{
   return mpGopSize->value();
}

void OptionsMovieExporter::setGopSize(int size)
{
   if(size >= mpGopSize->minimum() && size <= mpGopSize->maximum())
   {
      mpGopSize->setValue(size);
   }
}

float OptionsMovieExporter::getQCompress() const
{
   return static_cast<float>(mpQCompress->value());
}

void OptionsMovieExporter::setQCompress(float val)
{
   if(val >= mpQCompress->minimum() && val <= mpQCompress->maximum())
   {
      mpQCompress->setValue(val);
   }
}

float OptionsMovieExporter::getQBlur() const
{
   return static_cast<float>(mpQBlur->value());
}

void OptionsMovieExporter::setQBlur(float val)
{
   if(val >= mpQBlur->minimum() && val <= mpQBlur->maximum())
   {
      mpQBlur->setValue(val);
   }
}

int OptionsMovieExporter::getQMinimum() const
{
   return mpQMinimum->value();
}

void OptionsMovieExporter::setQMinimum(int val)
{
   if(val >= mpQMinimum->minimum() && val <= mpQMinimum->maximum())
   {
      mpQMinimum->setValue(val);
   }
}

int OptionsMovieExporter::getQMaximum() const
{
   return mpQMaximum->value();
}

void OptionsMovieExporter::setQMaximum(int val)
{
   if(val >= mpQMaximum->minimum() && val <= mpQMaximum->maximum())
   {
      mpQMaximum->setValue(val);
   }
}

int OptionsMovieExporter::getQDiffMaximum() const
{
   return mpQDiffMaximum->value();
}

void OptionsMovieExporter::setQDiffMaximum(int val)
{
   if(val >= mpQDiffMaximum->minimum() && val <= mpQDiffMaximum->maximum())
   {
      mpQDiffMaximum->setValue(val);
   }
}

int OptionsMovieExporter::getMaxBFrames() const
{
   return mpMaxBFrames->value();
}

void OptionsMovieExporter::setMaxBFrames(int val)
{
   if(val >= mpMaxBFrames->minimum() && val <= mpMaxBFrames->maximum())
   {
      mpMaxBFrames->setValue(val);
   }
}

float OptionsMovieExporter::getBQuantFactor() const
{
   return static_cast<float>(mpBQuantFactor->value());
}

void OptionsMovieExporter::setBQuantFactor(float val)
{
   if(val >= mpBQuantFactor->minimum() && val <= mpBQuantFactor->maximum())
   {
      mpBQuantFactor->setValue(val);
   }
}

float OptionsMovieExporter::getBQuantOffset() const
{
   return static_cast<float>(mpBQuantOffset->value());
}

void OptionsMovieExporter::setBQuantOffset(float val)
{
   if(val >= mpBQuantOffset->minimum() && val <= mpBQuantOffset->maximum())
   {
      mpBQuantOffset->setValue(val);
   }
}

int OptionsMovieExporter::getDiaSize() const
{
   return mpDiaSize->value();
}

void OptionsMovieExporter::setDiaSize(int val)
{
   if(val >= mpDiaSize->minimum() && val <= mpDiaSize->maximum())
   {
      mpDiaSize->setValue(val);
   }
}

int OptionsMovieExporter::getFlags() const
{
   int flags=0;
   if(mpQScale->isChecked()) flags |= CODEC_FLAG_QSCALE;
   if(mpQPel->isChecked()) flags |= CODEC_FLAG_QPEL;
   if(mpGmc->isChecked()) flags |= CODEC_FLAG_GMC;
   if(mpNormalizeAqp->isChecked()) flags |= CODEC_FLAG_NORMALIZE_AQP;
   if(mpTrellis->isChecked()) flags |= CODEC_FLAG_TRELLIS_QUANT;
   if(mpAcPred->isChecked()) flags |= CODEC_FLAG_AC_PRED;
   if(mpCbpRd->isChecked()) flags |= CODEC_FLAG_CBP_RD;
   if(mpQpRd->isChecked()) flags |= CODEC_FLAG_QP_RD;
   if(mpObmc->isChecked()) flags |= CODEC_FLAG_OBMC;
   if(mpClosedGop->isChecked()) flags |= CODEC_FLAG_CLOSED_GOP;
   return flags;
}

void OptionsMovieExporter::setFlags(int val)
{
   mpQScale->setChecked(val & CODEC_FLAG_QSCALE);
   mpQPel->setChecked(val & CODEC_FLAG_QPEL);
   mpGmc->setChecked(val & CODEC_FLAG_GMC);
   mpNormalizeAqp->setChecked(val & CODEC_FLAG_NORMALIZE_AQP);
   mpTrellis->setChecked(val & CODEC_FLAG_TRELLIS_QUANT);
   mpAcPred->setChecked(val & CODEC_FLAG_AC_PRED);
   mpCbpRd->setChecked(val & CODEC_FLAG_CBP_RD);
   mpQpRd->setChecked(val & CODEC_FLAG_QP_RD);
   mpObmc->setChecked(val & CODEC_FLAG_OBMC);
   mpClosedGop->setChecked(val & CODEC_FLAG_CLOSED_GOP);
}

void OptionsMovieExporter::setUseViewResolution(bool v)
{
   mpUseViewResolution->setChecked(v);
   mpResolutionX->setEnabled(!v);
   mpResolutionY->setEnabled(!v);
   mpResolutionAspectLock->setEnabled(!v);
}

void OptionsMovieExporter::checkResolutionX(bool ignoreAspectRatio)
{
   if(mpUseViewResolution->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionX->text().toUInt();
   if((val % 2) != 0)
   {
      mpResolutionX->setText(QString::number(val + 1));
   }
   if(!ignoreAspectRatio && mpResolutionAspectLock->isChecked())
   {
      unsigned int newY = val;
      if(mCurrentResolutionY > 0 && mCurrentResolutionX > 0)
      {
         newY = (val * mCurrentResolutionY) / static_cast<double>(mCurrentResolutionX);
      }
      if((newY % 2) != 0)
      {
         newY++;
      }
      mpResolutionY->setText(QString::number(newY));
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void OptionsMovieExporter::checkResolutionY(bool ignoreAspectRatio)
{
   if(mpUseViewResolution->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionY->text().toUInt();
   if((val % 2) != 0)
   {
      mpResolutionY->setText(QString::number(val + 1));
   }
   if(!ignoreAspectRatio && mpResolutionAspectLock->isChecked())
   {
      unsigned int newX = val;
      if(mCurrentResolutionX > 0 && mCurrentResolutionY > 0)
      {
         newX = (val * mCurrentResolutionX) / static_cast<double>(mCurrentResolutionY);
      }
      if((newX % 2) != 0)
      {
         newX++;
      }
      mpResolutionX->setText(QString::number(newX));
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void OptionsMovieExporter::aspectLockToggled(bool state)
{
   mpResolutionAspectLock->setIcon(state ? *mpLockIcon : *mpUnlockIcon);
   if(state)
   {
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void OptionsMovieExporter::updateBitrate(int value)
{
   mpBitrateValue->setText(QString::number(value) + " kbit/s");
}

void OptionsMovieExporter::frameRateListChanged(const QString &value)
{
   QStringList rationalAndDecimal = value.split(" - ");
   if(value == "Custom...")
   {
      mpFramerateNum->setEnabled(true);
      mpFramerateDen->setEnabled(true);
   }
   else if(rationalAndDecimal.size() == 2)
   {
      QStringList split = rationalAndDecimal[1].split("/");
      if(split.size() == 2)
      {
         mpFramerateNum->setValue(split[0].toInt());
         mpFramerateDen->setValue(split[1].toInt());
         mpFramerateNum->setEnabled(false);
         mpFramerateDen->setEnabled(false);
      }
   }
}