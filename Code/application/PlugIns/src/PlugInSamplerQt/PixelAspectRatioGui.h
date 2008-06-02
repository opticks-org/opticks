/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXELASPECTRATIOGUI_H__
#define PIXELASPECTRATIOGUI_H__

#include <QtGui/QDialog>
#include "PlotWidget.h"
#include "DockWindow.h"
#include "Layer.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;

class PixelAspectRatioGui : public QDialog
{
    Q_OBJECT

public:
    PixelAspectRatioGui( QWidget* pParent = 0, const char* pName = 0, bool modal = FALSE );
    ~PixelAspectRatioGui();

public slots:
    void applyScale();
    void generateNewView();

private:
    QPushButton* mpCancelButton;
    QPushButton* mpApplyButton;
    QPushButton* mpGenerateViewButton;
    QComboBox* mpCubeListCombo;
    QDoubleSpinBox* mpXScaleFactor;
    QDoubleSpinBox* mpYScaleFactor;
    QLabel* mpCubeListComboLabel;
    QLabel* mpXScaleFactorLabel;
    QLabel* mpYScaleFactorLabel;

    void init();
    std::vector<std::string> mCubeNames;
    Layer *mpScaledLayer;
    bool mbScalingApplied;

};

#endif