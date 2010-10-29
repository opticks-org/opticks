/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QHBoxLayout>

#include "ElidedButton.h"
#include "ElidedLabel.h"
#include "InfoBar.h"

InfoBar::InfoBar(QWidget* parent) :
   QWidget(parent),
   mpTitle(new ElidedLabel(this)),
   mpButton(new ElidedButton(this)),
   mpIconLabel(new QLabel(this))
{
   // Initialization
   setAutoFillBackground(true);

   mpTitle->setMinimumWidth(100);
   mpButton->setMinimumWidth(100);
   mpButton->setFlat(true);
   mpButton->hide();

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(2);
   pLayout->setSpacing(10);
   pLayout->addSpacing(5);
   pLayout->addWidget(mpTitle, 10, Qt::AlignLeft);
   pLayout->addWidget(mpButton, 10, Qt::AlignLeft);
   pLayout->addWidget(mpIconLabel);
   pLayout->addSpacing(5);
}

InfoBar::~InfoBar()
{
   setMenu(NULL);
}

void InfoBar::setBackgroundColor(const QColor& clrBackground)
{
   QPalette infoBarPalette = palette();
   infoBarPalette.setColor(QPalette::Window, clrBackground);
   setPalette(infoBarPalette);
}

QString InfoBar::getTitle() const
{
   QString strTitle = mpTitle->text();
   return strTitle;
}

QColor InfoBar::getTitleColor() const
{
   QPalette titlePalette = mpTitle->palette();
   QColor clrTitle = titlePalette.color(QPalette::WindowText);

   return clrTitle;
}

QFont InfoBar::getTitleFont() const
{
   QFont fntTitle = mpTitle->font();
   return fntTitle;
}

void InfoBar::setMenu(QMenu* pMenu)
{
   QMenu* pButtonMenu = mpButton->menu();
   if (pMenu == pButtonMenu)
   {
      return;
   }

   if (pButtonMenu != NULL)
   {
      disconnect(pButtonMenu, SIGNAL(triggered(QAction*)), this, SLOT(setTitle(QAction*)));
      delete pButtonMenu;
   }

   mpButton->setMenu(pMenu);

   if (pMenu == NULL)
   {
      setTitleButton(false);
   }
   else
   {
      QString strTitle = getTitle();
      if (strTitle.isEmpty() == false)
      {
         setTitleButton(true);
      }

      connect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(setTitle(QAction*)));
   }
}

QMenu* InfoBar::getMenu() const
{
   QMenu* pMenu = mpButton->menu();
   return pMenu;
}

void InfoBar::setInfoIcon(const QPixmap& pixInfo)
{
   mpIconLabel->setPixmap(pixInfo);
}

const QPixmap* InfoBar::getInfoIcon() const
{
   const QPixmap* pPixmap = mpIconLabel->pixmap();
   return pPixmap;
}

QSize InfoBar::sizeHint() const
{
   int iWidth = 0;
   int iHeight = 0;

   // Title
   QFont ftTitle;
   QString strText;

   if (mpTitle->isHidden() == false)
   {
      ftTitle = mpTitle->font();
      strText = mpTitle->text();
   }
   else
   {
      ftTitle = mpButton->font();
      strText = mpButton->text();
   }

   if (strText.isEmpty() == false)
   {
      QFontMetrics titleMetrics(ftTitle);
      iWidth = titleMetrics.width(strText);
      iHeight = titleMetrics.height();
   }

   // Icon
   const QPixmap* pPixmap = getInfoIcon();
   if (pPixmap != NULL)
   {
      iWidth += pPixmap->width();
      if (pPixmap->height() > iHeight)
      {
         iHeight = pPixmap->height();
      }
   }

   // Margins and spacing
   iWidth += 34;
   iHeight += 4;

   return QSize(iWidth, iHeight);
}

void InfoBar::setTitle(const QString& strTitle)
{
   QString strCurrentTitle = getTitle();
   if (strCurrentTitle == strTitle)
   {
      return;
   }

   mpTitle->setText(strTitle);
   mpButton->setText(strTitle);

   QMenu* pMenu = mpButton->menu();
   if (pMenu != NULL)
   {
      bool bNull = strTitle.isEmpty();
      setTitleButton(!bNull);
   }

   QFont fntTitle = mpButton->font();
   resizeTitleButton(fntTitle);

   emit titleChanged(strTitle);

   if (pMenu != NULL)
   {
      QList<QAction*> menuActions = pMenu->actions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         QAction* pAction = menuActions[i];
         if (pAction != NULL)
         {
            QString strAction = pAction->text();
            if (strAction == strTitle)
            {
               emit titleChanged(pAction);
            }
         }
      }
   }
}

void InfoBar::setTitleColor(QColor clrTitle)
{
   QPalette titlePalette = mpTitle->palette();
   titlePalette.setColor(QPalette::WindowText, clrTitle);
   mpTitle->setPalette(titlePalette);

   QPalette buttonPalette = mpButton->palette();
   buttonPalette.setColor(QPalette::ButtonText, clrTitle);
   mpButton->setPalette(buttonPalette);
}

void InfoBar::setTitleFont(QFont fntTitle)
{
   mpTitle->setFont(fntTitle);
   mpButton->setFont(fntTitle);

   resizeTitleButton(fntTitle);
}

void InfoBar::setElideMode(Qt::TextElideMode mode)
{
   mpTitle->setElideMode(mode);
   mpButton->setElideMode(mode);
}

void InfoBar::setTitleButton(bool bButton)
{
   mpTitle->setVisible(!bButton);
   mpButton->setVisible(bButton);
}

void InfoBar::resizeTitleButton(QFont fntTitle)
{
   QString strTitle = getTitle();

   QFontMetrics fntMetrics = QFontMetrics(fntTitle);
   int iWidth = fntMetrics.width(strTitle);
   int iHeight = fntMetrics.height();

   mpButton->setMaximumWidth(iWidth + 30);   // Account for width of popup arrow
   mpButton->setFixedHeight(iHeight + 2);
}

void InfoBar::setTitle(QAction* pAction)
{
   if (pAction != NULL)
   {
      QString strTitle = pAction->text();
      setTitle(strTitle);
   }
}
