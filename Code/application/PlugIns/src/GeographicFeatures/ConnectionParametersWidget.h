/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONNECTIONPARAMETERSWIDGET_H
#define CONNECTIONPARAMETERSWIDGET_H

#include "ConnectionParameters.h"
#include "ModifierWidget.h"

#include <vector>

class FileBrowser;
class QCheckBox;
class QLineEdit;
class QRadioButton;

class ConnectionParametersWidget : public ModifierWidget
{
   Q_OBJECT

public:
   ConnectionParametersWidget(QWidget* pParent = NULL);
   virtual ~ConnectionParametersWidget();

   ArcProxyLib::ConnectionParameters getConnectionParameters() const;
   void setConnectionParameters(const ArcProxyLib::ConnectionParameters& connection);

   void setAvailableConnectionTypes(const std::vector<ArcProxyLib::ConnectionType>& types);

protected slots:
   void updateWorkingDirectory(const QString& filename);
   void updateWidgets();

private:
   ConnectionParametersWidget(const ConnectionParametersWidget& rhs);
   ConnectionParametersWidget& operator=(const ConnectionParametersWidget& rhs);

   QRadioButton* mpShpButton;
   QWidget* mpShpWidget;
   FileBrowser* mpFilenameEdit;
   QCheckBox* mpUseArcCheck;

   QRadioButton* mpSdeButton;
   QWidget* mpSdeWidget;
   QLineEdit* mpUserEdit;
   QLineEdit* mpPasswordEdit;
   QLineEdit* mpDatabaseEdit;
   QLineEdit* mpServerEdit;
   QLineEdit* mpInstanceEdit;
   QLineEdit* mpVersionEdit;
   QLineEdit* mpFeatureClassEdit;

   bool mShapelibConnection;
   bool mShpConnection;
   bool mSdeConnection;
};

#endif
