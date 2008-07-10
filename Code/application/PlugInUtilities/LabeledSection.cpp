/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include "AppVerify.h"
#include "LabeledSection.h"

LabeledSection::LabeledSection(QWidget* pParent) :
   QWidget(pParent),
   mpExpandLabel(NULL),
   mpTextLabel(NULL),
   mpSectionWidget(NULL)
{
   initialize();
}

LabeledSection::LabeledSection(const QString& text, QWidget* pParent) :
   QWidget(pParent),
   mpExpandLabel(NULL),
   mpTextLabel(NULL),
   mpSectionWidget(NULL)
{
   initialize(text);
}

LabeledSection::LabeledSection(QWidget* pSectionWidget, const QString& text, QWidget* pParent) :
   QWidget(pParent),
   mpExpandLabel(NULL),
   mpTextLabel(NULL),
   mpSectionWidget(NULL)
{
   initialize(text, pSectionWidget);
}

void LabeledSection::initialize(const QString& text, QWidget* pSectionWidget)
{
   // Expand label
   mpExpandLabel = new QLabel(this);

   // Text label
   mpTextLabel = new QLabel(this);
   QFont font = mpTextLabel->font();
   font.setBold(true);
   mpTextLabel->setFont(font);

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Layout
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpExpandLabel, 0, 0);
   pLayout->addWidget(mpTextLabel, 0, 1);
   pLayout->addWidget(pHLine, 0, 2);
   pLayout->setRowStretch(1, 10);
   pLayout->setColumnStretch(2, 10);

   // Initialization
   setText(text);
   setSectionWidget(pSectionWidget);
}

LabeledSection::~LabeledSection()
{
}

QString LabeledSection::getText() const
{
   VERIFYRV(mpTextLabel != NULL, QString());
   return mpTextLabel->text();
}

void LabeledSection::setText(const QString &newText)
{
   VERIFYNRV(mpTextLabel != NULL);
   mpTextLabel->setText(newText);
}

QWidget *LabeledSection::getSectionWidget() const
{
   return mpSectionWidget;
}

void LabeledSection::setSectionWidget(QWidget *pNewSectionWidget)
{
   QGridLayout *pLayout = dynamic_cast<QGridLayout*>(layout());
   VERIFYNRV(pLayout != NULL);

   if (mpSectionWidget != NULL)
   {
      mpSectionWidget->setParent(NULL);
      mpSectionWidget->removeEventFilter(this);
      pLayout->removeWidget(mpSectionWidget);
   }

   mpSectionWidget = pNewSectionWidget;
   if (mpSectionWidget != NULL)
   {
      mpSectionWidget->setParent(this);
      mpSectionWidget->installEventFilter(this);
      pLayout->addWidget(mpSectionWidget, 1, 1, 1, 2);
   }

   updateIndicator();
}

void LabeledSection::collapse()
{
   if (mpSectionWidget != NULL)
   {
      mpSectionWidget->hide();
   }
}

void LabeledSection::expand()
{
   if (mpSectionWidget != NULL)
   {
      mpSectionWidget->show();
   }
}

bool LabeledSection::eventFilter(QObject* pObject, QEvent* pEvent)
{
   bool bReturn = QWidget::eventFilter(pObject, pEvent);
   if ((pObject != NULL) && (pEvent != NULL))
   {
      if (pObject == mpSectionWidget)
      {
         QEvent::Type eventType = pEvent->type();
         if ((eventType == QEvent::Show) || (eventType == QEvent::ShowToParent) ||
            (eventType == QEvent::Hide) || (eventType == QEvent::HideToParent))
         {
            updateIndicator();
         }
      }
   }

   return bReturn;
}

void LabeledSection::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      const QPoint& pos = pEvent->pos();
      if (mpSectionWidget != NULL)
      {
         QPoint labelPos = mpExpandLabel->pos();
         int labelWidth = mpExpandLabel->width();
         int labelHeight = mpExpandLabel->height();

         QRect labelRect(labelPos.x(), labelPos.y(), labelWidth, labelHeight);
         if (labelRect.contains(pos) == true)
         {
            mpSectionWidget->setVisible(mpSectionWidget->isHidden());
         }
      }
   }

   QWidget::mousePressEvent(pEvent);
}

void LabeledSection::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      const QPoint& pos = pEvent->pos();
      if (mpSectionWidget != NULL)
      {
         QPoint headerPos = mpTextLabel->pos();
         int headerWidth = width() - headerPos.x();
         int headerHeight = mpTextLabel->height();

         QRect headerRect(headerPos.x(), headerPos.y(), headerWidth, headerHeight);
         if (headerRect.contains(pos) == true)
         {
            mpSectionWidget->setVisible(mpSectionWidget->isHidden());
         }
      }
   }

   QWidget::mouseDoubleClickEvent(pEvent);
}

void LabeledSection::updateIndicator()
{
   if (mpSectionWidget != NULL)
   {
      bool bVisible = mpSectionWidget->isVisible();

      // Show the indicator
      mpExpandLabel->show();

      // Update the indicator pixmap
      QStyle* pStyle = style();
      if (pStyle != NULL)
      {
         QPixmap labelPixmap(9, 9);
         labelPixmap.fill(Qt::white);

         QStyleOption option;
         option.rect = labelPixmap.rect();
         option.state = QStyle::State_Children;
         if (bVisible == true)
         {
            option.state |= QStyle::State_Open;
         }

         QPainter pixmapPainter(&labelPixmap);
         pStyle->drawPrimitive(QStyle::PE_IndicatorBranch, &option, &pixmapPainter);

         mpExpandLabel->setPixmap(labelPixmap);
      }

      // Emit the signal
      if (bVisible == true)
      {
         emit expanded();
      }
      else
      {
         emit collapsed();
      }
   }
   else
   {
      // Hide the indicator
      mpExpandLabel->hide();
   }
}
