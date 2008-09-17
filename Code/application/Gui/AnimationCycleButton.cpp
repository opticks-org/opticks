/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationCycleButton.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "Icons.h"
#include "StringUtilities.h"

AnimationCycleGrid::AnimationCycleGrid(QWidget* pParent): PixmapGrid(pParent)
{
   setNumRows(1);
   setNumColumns(3);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setPixmap(0, 0, pIcons->mPlayOnce,
      QString::fromStdString(StringUtilities::toXmlString(PLAY_ONCE)),
      QString::fromStdString(StringUtilities::toDisplayString(PLAY_ONCE)));
   setPixmap(0, 1, pIcons->mBouncePlay,
      QString::fromStdString(StringUtilities::toXmlString(BOUNCE)),
      QString::fromStdString(StringUtilities::toDisplayString(BOUNCE)));
   setPixmap(0, 2, pIcons->mRepeatPlay,
      QString::fromStdString(StringUtilities::toXmlString(REPEAT)),
      QString::fromStdString(StringUtilities::toDisplayString(REPEAT)));

   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(PLAY_ONCE)));

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void AnimationCycleGrid::setCurrentValue(AnimationCycle value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

AnimationCycle AnimationCycleGrid::getCurrentValue() const
{
   AnimationCycle retValue;
   std::string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<AnimationCycle>(curText);
   }
   return retValue;
}

void AnimationCycleGrid::translateChange(const QString& strText)
{
   AnimationCycle curType = StringUtilities::fromXmlString<AnimationCycle>(strText.toStdString());
   emit valueChanged(curType);
}

AnimationCycleButton::AnimationCycleButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   AnimationCycleGrid* pGrid = new AnimationCycleGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(AnimationCycle)), this, SIGNAL(valueChanged(AnimationCycle))));
}

void AnimationCycleButton::setCurrentValue(AnimationCycle value)
{
   AnimationCycleGrid* pGrid = dynamic_cast<AnimationCycleGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

AnimationCycle AnimationCycleButton::getCurrentValue() const
{
   AnimationCycle retValue;
   AnimationCycleGrid* pGrid = dynamic_cast<AnimationCycleGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}