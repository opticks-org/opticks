/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENGRDAWIDGET_H__
#define ENGRDAWIDGET_H__

#include "AttachmentPtr.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ExecutableShell.h"
#include "ObjectResource.h"
#include <QtGui/QWidget>
#include <string>

class QGroupBox;
class QLineEdit;
class QRadioButton;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;
class SessionItem;

class EngrdaWidget : public QWidget
{
   Q_OBJECT

public:
   EngrdaWidget(QWidget* pParent = NULL);
   virtual ~EngrdaWidget();

   bool initialize(SessionItem* pSessionItem);
   bool initialize(const DynamicObject* pMeta);
   bool applyChanges();
   bool canDisplayMetadata(const DynamicObject& treMetadata) const;

private slots:
   void updateData(QTreeWidgetItem* pItem);

private:
   QTreeWidget* mpTres;
   QLineEdit* mpValueType;
   QLineEdit* mpDataUnits;
   QTableWidget* mpData;
   QGroupBox* mpRadioGroup;
   QRadioButton* mpRowMajor;

   FactoryResource<DynamicObject> mpMeta;

public:
   static const std::string& getTypeName()
   {
      static std::string val = "ENGRDA";
      return val;
   }
   static const std::string& getDescription()
   {
      static std::string val = "Display information contained in NITF ENGRDA TREs.";
      return val;
   }
   static const std::string& getShortDescription()
   {
      static std::string val;
      return val;
   }
   static const std::string& getCreator()
   {
      static std::string val = "Ball Aerospace & Technologies Corp.";
      return val;
   }
   static const std::string& getCopyright()
   {
      static std::string val = APP_COPYRIGHT_MSG;
      return val;
   }
   static const std::string& getVersion()
   {
      static std::string val = APP_VERSION_NUMBER;
      return val;
   }
   static const std::string& getDescriptorId()
   {
      static std::string val = "{5bbfc188-93e6-4e83-819b-2aaa71ecba46}";
      return val;
   }
   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }
};

#endif
