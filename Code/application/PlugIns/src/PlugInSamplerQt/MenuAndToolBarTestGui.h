/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MENUANDTOOLBARTESTGUI_H
#define MENUANDTOOLBARTESTGUI_H

#include <QtCore/QVariant>
#include <QtCore/QObject>
#include <QtWidgets/QDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QWidget>
#include "PlotWidget.h"
#include "DockWindow.h"
#include "HistogramWindow.h"
#include "HistogramPlot.h"
#include "Layer.h"
#include "Service.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QLabel;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QComboBox;
class QDoubleSpinBox;
class QFrame;

class MenuAndToolBarTestGui : public QDialog
{
    Q_OBJECT

public:
    MenuAndToolBarTestGui( QWidget* pParent = 0, const char* pName = 0, bool modal = false );
    ~MenuAndToolBarTestGui();

public slots:
    void disableFileOpenButton();
    void enableFileOpenButton();
    void addMenuItems();
    void removeMenuItems();
    void disableGeneralAlgorithmsMenuItems();
    void enableGeneralAlgorithmsMenuItems();
    void reorderToolboxToolbarButtons();
    void resetToolboxToolbarButtons();

private:
    MenuAndToolBarTestGui(const MenuAndToolBarTestGui& rhs);
    MenuAndToolBarTestGui& operator=(const MenuAndToolBarTestGui& rhs);
    QPushButton* mpCancelButton;
    QPushButton* mpDisableOpenButton;
    QPushButton* mpEnableOpenButton;
    QPushButton* mpAddTestItemsButton;
    QPushButton* mpRemoveTestItemsButton;
    QPushButton* mpDisableSpectralItemsButton;
    QPushButton* mpEnableSpectralItemsButton;
    QPushButton* mpReorderToolboxButton;
    QPushButton* mpResetToolboxButton;

    void init();
    std::vector<QAction*> mToolbarItems;
};

#endif
