/*
 * The information in this file is
 * Copyright(c) 2014 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSLASIMPORTER_H__
#define OPTIONSLASIMPORTER_H__

#include <QtGui/QComboBox>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>

class OptionLasImporter : public QObject
{
    Q_OBJECT

public:
    OptionLasImporter();
    ~OptionLasImporter();
    QWidget* getWidget();
    QComboBox* getDropDown();
    QLineEdit* getInputMaxPoints();
    QLineEdit* getInputGridSize();
    
public slots:
    void indexChangedEvent(int newIndex);

private:
    QWidget* mpWidget;
    QComboBox* mpDropDown;
    QLineEdit* mpInputMaxPoints;
    QLineEdit* mpInputGridSize;
    QLabel* mpDropDownLabel;
    QLabel* mpMaxPointsLabel;
    QLabel* mpGridSizeLabel;

};

#endif