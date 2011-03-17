/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTPROPERTIESDLG_H
#define PLOTPROPERTIESDLG_H

#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

class CustomColorButton;
class GraphicTextWidget;
class PlotWidget;

class PlotPropertiesDlg : public QDialog
{
   Q_OBJECT

public:
   PlotPropertiesDlg(PlotWidget* pPlot, QWidget* pParent = 0);
   ~PlotPropertiesDlg();

protected slots:
   void accept();
   void applyChanges();

private:
   PlotWidget* mpPlot;

   QComboBox* mpClassPositionCombo;

   QComboBox* mpOrgPositionCombo;
   GraphicTextWidget* mpOrgText;

   QComboBox* mpGridlineStyleCombo;
   QComboBox* mpGridlineWidthCombo;
   CustomColorButton* mpGridlineColorButton;

   QComboBox* mpLineStyleCombo;
   QComboBox* mpLineWidthCombo;
   CustomColorButton* mpObjectColorButton;

   QComboBox* mpXScaleCombo;
   QComboBox* mpYScaleCombo;

   QComboBox* mpSymbolSizeCombo;
   QComboBox* mpDisplayMethodCombo;
   QComboBox* mpSelectionModeCombo;
   QComboBox* mpSelectionDisplayModeCombo;
   QComboBox* mpSymbolChooserCombo;
   QCheckBox* mpEnableShading;
};

#endif
