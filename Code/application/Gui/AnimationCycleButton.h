/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONCYCLEBUTTON_H
#define ANIMATIONCYCLEBUTTON_H

#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "TypesFile.h"

class AnimationCycleGrid : public PixmapGrid
{
   Q_OBJECT

public:
   AnimationCycleGrid(QWidget* pParent);
   void setCurrentValue(AnimationCycle value);
   AnimationCycle getCurrentValue() const;

signals: 
   void valueChanged(AnimationCycle value);

private slots:
   void translateChange(const QString&);

private:
   AnimationCycleGrid(const AnimationCycleGrid& rhs);
   AnimationCycleGrid& operator=(const AnimationCycleGrid& rhs);
};

class AnimationCycleButton : public PixmapGridButton
{
   Q_OBJECT

public:
   AnimationCycleButton(QWidget* pParent);

   void setCurrentValue(AnimationCycle value);
   AnimationCycle getCurrentValue() const;

signals:
   void valueChanged(AnimationCycle value);

private:
   AnimationCycleButton(const AnimationCycleButton& rhs);
   AnimationCycleButton& operator=(const AnimationCycleButton& rhs);
};

#endif
