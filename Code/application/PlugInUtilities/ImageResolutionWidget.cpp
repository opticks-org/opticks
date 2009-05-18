/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include "AppVerify.h"
#include "ImageResolutionWidget.h"

#include <string>
#include <vector>
using namespace std;

namespace
{
   const char* const lock_xpm[] =
   {
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
      "                                "
   };

   const char* const unlock_xpm[] =
   {
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
      "                                "
   };
};

ImageResolutionWidget::ImageResolutionWidget(QWidget* pParent) : QWidget(pParent)
{
   // Resolution section
   QLabel* pWidthLabel = new QLabel("Width", this);
   mpResolutionX = new QLineEdit(this);
   mpResolutionX->setStatusTip("The number of pixels in the image width");
   mpResolutionX->setToolTip("Image width");
   QLabel* pHeightLabel = new QLabel("Height", this);
   mpResolutionY = new QLineEdit(this);
   mpResolutionY->setStatusTip("The number of pixels in the image height");
   mpResolutionY->setToolTip("Image height");
   QIntValidator* pValidator = new QIntValidator(this);
   pValidator->setBottom(2);
   mpResolutionX->setValidator(pValidator);
   mpResolutionY->setValidator(pValidator);

   QPixmap lockPixmap(lock_xpm);
   lockPixmap.setMask(lockPixmap.createHeuristicMask());
   mpLockIcon = new QIcon(lockPixmap);
   QPixmap unlockPixmap(unlock_xpm);
   unlockPixmap.setMask(unlockPixmap.createHeuristicMask());
   mpUnlockIcon = new QIcon(unlockPixmap);
   mpResolutionAspectLock = new QPushButton(*mpLockIcon, QString(), this);
   mpResolutionAspectLock->setCheckable(true);
   mpResolutionAspectLock->setChecked(true);
   mpResolutionAspectLock->setToolTip("Lock the resolution aspect ratio.");

   VERIFYNR(connect(mpResolutionX, SIGNAL(editingFinished()), this, SLOT(checkResolutionX())));
   VERIFYNR(connect(mpResolutionY, SIGNAL(editingFinished()), this, SLOT(checkResolutionY())));

   QGridLayout* pResolutionLayout = new QGridLayout(this);
   pResolutionLayout->addWidget(pWidthLabel, 1, 0);
   pResolutionLayout->addWidget(mpResolutionX, 1, 1);
   pResolutionLayout->addWidget(pHeightLabel, 2, 0);
   pResolutionLayout->addWidget(mpResolutionY, 2, 1);
   pResolutionLayout->addWidget(mpResolutionAspectLock, 3, 0, 4, 2, Qt::AlignLeft);
   pResolutionLayout->setColumnStretch(5, 10);
}

ImageResolutionWidget::~ImageResolutionWidget()
{
   delete mpLockIcon;
   delete mpUnlockIcon;
}

void ImageResolutionWidget::getResolution(unsigned int &width, unsigned int &height) const
{
   width = mpResolutionX->text().toUInt();
   height = mpResolutionY->text().toUInt();
}

void ImageResolutionWidget::setResolution(unsigned int width, unsigned int height)
{
   const QValidator* pValidX = mpResolutionX->validator();
   const QValidator* pValidY = mpResolutionY->validator();
   int pos1 = 0;
   int pos2 = 0;
   QString widthStr = QString::number(width);
   QString heightStr = QString::number(height);
   if ((pValidX == NULL || pValidX->validate(widthStr, pos1) == QValidator::Acceptable) &&
      (pValidY == NULL || pValidY->validate(heightStr, pos2) == QValidator::Acceptable))
   {
      mpResolutionX->setText(widthStr);
      mpResolutionY->setText(heightStr);
      checkResolutionX(true);
      checkResolutionY(true);
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

bool ImageResolutionWidget::getAspectRatioLock()
{
   return mpResolutionAspectLock->isChecked();
}

void ImageResolutionWidget::checkResolutionX(bool ignoreAspectRatio)
{
   if (!mpResolutionAspectLock->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionX->text().toUInt();
   if ((val % 2) != 0)
   {
      mpResolutionX->setText(QString::number(val + 1));
   }
   if (!ignoreAspectRatio && mpResolutionAspectLock->isChecked())
   {
      unsigned int newY = val;
      if (mCurrentResolutionY > 0 && mCurrentResolutionX > 0)
      {
         newY = (val * mCurrentResolutionY) / static_cast<double>(mCurrentResolutionX);
      }
      if ((newY % 2) != 0)
      {
         newY++;
      }
      mpResolutionY->setText(QString::number(newY));
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void ImageResolutionWidget::checkResolutionY(bool ignoreAspectRatio)
{
   if (!mpResolutionAspectLock->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionY->text().toUInt();
   if ((val % 2) != 0)
   {
      mpResolutionY->setText(QString::number(val + 1));
   }
   if (!ignoreAspectRatio && mpResolutionAspectLock->isChecked())
   {
      unsigned int newX = val;
      if (mCurrentResolutionX > 0 && mCurrentResolutionY > 0)
      {
         newX = (val * mCurrentResolutionX) / static_cast<double>(mCurrentResolutionY);
      }
      if ((newX % 2) != 0)
      {
         newX++;
      }
      mpResolutionX->setText(QString::number(newX));
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void ImageResolutionWidget::setAspectRatioLock(bool state)
{
   mpResolutionAspectLock->setChecked(state);
}
