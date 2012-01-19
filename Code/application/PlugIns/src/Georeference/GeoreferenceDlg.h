/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GEOREFERENCEDLG_H
#define GEOREFERENCEDLG_H

#include <QtGui/QDialog>

#include "TypesFile.h"

#include <string>
#include <vector>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QStackedWidget;

class GeoreferenceDlg : public QDialog
{
   Q_OBJECT

public:
   GeoreferenceDlg(const QString& title, const std::vector<std::string>& geoPluginNameList,
      const std::vector<QWidget*>& geoPluginWidgetList, QWidget* parent = NULL);
   ~GeoreferenceDlg();

   void setResultsName(const std::string& name);
   std::string getResultsName() const;
   GeocoordType getGeocoordType() const;

   int getGeorefAlgorithmIndex() const;
   bool getCreateLayer() const;
   bool getDisplayLayer() const;

protected slots:
   void setPlugin(int iPluginIndex);
   void createLayerChanged(bool create);
   void accept();

private:
   GeoreferenceDlg(const GeoreferenceDlg& rhs);
   GeoreferenceDlg& operator=(const GeoreferenceDlg& rhs);

   QListWidget* mpGeoList;
   QStackedWidget* mpStack;

   QLineEdit* mpResultsEdit;
   QComboBox* mpCoordCombo;
   QCheckBox* mpCreateLayer;
   QCheckBox* mpDisplayLayer;

   std::vector<QWidget*> mWidgets;
};

#endif // GEOREFERENCEDLG_H
