/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPushButton>
#include <QtGui/QSlider>

#include "DemoGuiImp.h"
#include "AlgorithmPattern.h"

/**
 *  Constructor for the DemoGuiImp. Connects the buttons to the proper slots
 *  and initializes the sliders.
 *
 *  @param pParent
 *       The parent widget of the GUI.
 *  @param pRunner
 *       A pointer to the algorithm to run on OK or Apply
 */
DemoGuiImp::DemoGuiImp( QWidget* pParent, AlgorithmRunner *pRunner) :
   QDialog(pParent),
   mpRunner(pRunner),
   mColor1(0,0,0), mColor2(0xff,0,0), mColor3(0xff,0xff,0), mColor4(0xff,0xff,0xff)
{
   setupUi(this);
   setModal(true);
   setButtonColors();

   slider1->setValue(0);
   slider1_2->setValue(33);
   slider1_2_2->setValue(67);
   slider1_2_2_2->setValue(100);

   connect(pushButton3, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pushButton7, SIGNAL(clicked()), this, SLOT(apply()));
   connect(pushButton4, SIGNAL(clicked()), this, SLOT(reject()));
}

/**
 *  Computes the nodes based on the position of the sliders.
 *
 *  @return  A vector of nodes based on the position of the sliders
 */
std::vector<PlugInSamplerQt::Node> &DemoGuiImp::getNodes()
{
   static std::vector<PlugInSamplerQt::Node> nodes;

   nodes.clear();
   nodes.push_back(PlugInSamplerQt::Node(slider1->value() / 100.0, QCOLOR_TO_COLORTYPE(mColor1)));
   nodes.push_back(PlugInSamplerQt::Node(slider1_2->value() / 100.0, QCOLOR_TO_COLORTYPE(mColor2)));
   nodes.push_back(PlugInSamplerQt::Node(slider1_2_2->value() / 100.0, QCOLOR_TO_COLORTYPE(mColor3)));
   nodes.push_back(PlugInSamplerQt::Node(slider1_2_2_2->value() / 100.0, QCOLOR_TO_COLORTYPE(mColor4)));

   return nodes;
}

/**
 *  An obligation of the base class.
 *
 *  @return  true
 */
bool DemoGuiImp::getModified()
{
   return true;
}

/**
 *  The callback for the OK button. Applies the current settings and then
 *  dismisses the dialog.
 */
void DemoGuiImp::accept()
{
   apply();
   QDialog::accept();
}

/**
 *  The callback for the Apply button. Applies the current settings by
 *  calling the AlgorithmRunner that was provided to the constructor.
 */
void DemoGuiImp::apply()
{
   if (mpRunner)
   {
      mpRunner->runAlgorithmFromGuiInputs();
   }
}

void DemoGuiImp::setButtonColors()
{
// NOTE::pixmap was used to compensate for a WindowsXP theme issue
   QSize pixSize(colorButton1->width() / 2, colorButton1->height() / 2);

   QPixmap lTmpPixmap(pixSize);

   lTmpPixmap.fill(mColor1);
   colorButton1->setIcon(QIcon(lTmpPixmap));
   colorButton1->setIconSize(pixSize);

   QPalette buttonPalette = colorButton1->palette();
   buttonPalette.setColor(QPalette::Button, mColor1);
   buttonPalette.setColor(QPalette::ButtonText, mColor1);
   colorButton1->setPalette(buttonPalette);

   lTmpPixmap.fill(mColor2);
   colorButton2->setIcon(QIcon(lTmpPixmap));
   colorButton2->setIconSize(pixSize);

   buttonPalette = colorButton2->palette();
   buttonPalette.setColor(QPalette::Button, mColor2);
   buttonPalette.setColor(QPalette::ButtonText, mColor2);
   colorButton2->setPalette(buttonPalette);

   lTmpPixmap.fill(mColor3);
   colorButton3->setIcon(QIcon(lTmpPixmap));
   colorButton3->setIconSize(pixSize);

   buttonPalette = colorButton3->palette();
   buttonPalette.setColor(QPalette::Button, mColor3);
   buttonPalette.setColor(QPalette::ButtonText, mColor3);
   colorButton3->setPalette(buttonPalette);

   lTmpPixmap.fill(mColor4);
   colorButton4->setIcon(QIcon(lTmpPixmap));
   colorButton4->setIconSize(pixSize);

   buttonPalette = colorButton4->palette();
   buttonPalette.setColor(QPalette::Button, mColor4);
   buttonPalette.setColor(QPalette::ButtonText, mColor4);
   colorButton4->setPalette(buttonPalette);
}
