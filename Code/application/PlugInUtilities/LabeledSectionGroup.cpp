/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"

using namespace std;

LabeledSectionGroup::LabeledSectionGroup(QWidget* pParent) :
   QScrollArea(pParent),
   mpLayout(NULL)
{
   // Scroll widget
   QWidget* pScrollWidget = new QWidget(this);

   // Layout
   mpLayout = new QVBoxLayout(pScrollWidget);
   mpLayout->setMargin(0);
   mpLayout->setSpacing(15);

   // Initialization
   setFrameStyle(QFrame::NoFrame);
   setWidget(pScrollWidget);
   setWidgetResizable(true);
   viewport()->setAutoFillBackground(false);
   pScrollWidget->setAutoFillBackground(false);
}

LabeledSectionGroup::~LabeledSectionGroup()
{
}

void LabeledSectionGroup::addSection(LabeledSection* pSection, int stretch, Qt::Alignment alignment)
{
   if ((pSection != NULL) && (hasSection(pSection) == false))
   {
      pSection->setParent(widget());
      mpLayout->addWidget(pSection, stretch, alignment);
      mSections.insert(pSection, stretch);
      VERIFYNR(connect(pSection, SIGNAL(collapsed()), this, SLOT(disableSectionStretch())));
      VERIFYNR(connect(pSection, SIGNAL(expanded()), this, SLOT(enableSectionStretch())));
   }
}

void LabeledSectionGroup::addStretch(int stretch)
{
   mpLayout->addStretch(stretch);
}

void LabeledSectionGroup::removeSection(LabeledSection* pSection)
{
   if ((pSection != NULL) && (hasSection(pSection) == true))
   {
      pSection->setParent(NULL);
      mpLayout->removeWidget(pSection);
      mSections.remove(pSection);
      VERIFYNR(disconnect(pSection, SIGNAL(collapsed()), this, SLOT(disableSectionStretch())));
      VERIFYNR(disconnect(pSection, SIGNAL(expanded()), this, SLOT(enableSectionStretch())));
   }
}

bool LabeledSectionGroup::hasSection(LabeledSection* pSection) const
{
   if (pSection != NULL)
   {
      return mSections.contains(pSection);
   }

   return false;
}

vector<LabeledSection*> LabeledSectionGroup::getSections() const
{
   vector<LabeledSection*> sections;

   QMap<LabeledSection*, int>::const_iterator iter;
   for (iter = mSections.begin(); iter != mSections.end(); ++iter)
   {
      LabeledSection* pSection = iter.key();
      if (pSection != NULL)
      {
         sections.push_back(pSection);
      }
   }

   return sections;
}

void LabeledSectionGroup::expandSection(LabeledSection* pSection)
{
   if ((pSection != NULL) && (hasSection(pSection) == true))
   {
      pSection->expand();
   }
}

void LabeledSectionGroup::collapseSection(LabeledSection* pSection)
{
   if ((pSection != NULL) && (hasSection(pSection) == true))
   {
      pSection->collapse();
   }
}

void LabeledSectionGroup::clear()
{
   QList<LabeledSection*> sections = mSections.uniqueKeys();
   for (int i = 0; i < sections.count(); ++i)
   {
      LabeledSection* pSection = sections[i];
      if (pSection != NULL)
      {
         removeSection(pSection);
      }
   }

   for (int i = 0; i < mpLayout->count(); ++i)
   {
      QLayoutItem* pItem = mpLayout->takeAt(i);
      if (pItem != NULL)
      {
         delete pItem;
      }
   }
}

QSize LabeledSectionGroup::sizeHint() const
{
   QSize groupSize = QScrollArea::sizeHint();
   if (mPreferredSize.isValid())
   {
      groupSize = mPreferredSize;
   }

   return groupSize;
}

void LabeledSectionGroup::setSizeHint(int width, int height)
{
   setSizeHint(QSize(width, height));
}

void LabeledSectionGroup::setSizeHint(const QSize& size)
{
   mPreferredSize = size;
}

void LabeledSectionGroup::enableSectionStretch()
{
   LabeledSection* pSection = dynamic_cast<LabeledSection*>(sender());
   if (pSection != NULL)
   {
      QMap<LabeledSection*, int>::iterator iter = mSections.find(pSection);
      if (iter != mSections.end())
      {
         int stretch = iter.value();
         mpLayout->setStretchFactor(pSection, stretch);

         emit sectionExpanded(pSection);
      }
   }
}

void LabeledSectionGroup::disableSectionStretch()
{
   LabeledSection* pSection = dynamic_cast<LabeledSection*>(sender());
   if (pSection != NULL)
   {
      mpLayout->setStretchFactor(pSection, 0);
      emit sectionCollapsed(pSection);
   }
}
