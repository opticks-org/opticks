/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "Filename.h"
#include "FileResource.h"
#include "FeatureClass.h"
#include "FeatureClassWidget.h"
#include "FeatureLayerImporter.h"
#include "FeatureManager.h"
#include "FeatureMenuEditorDlg.h"
#include "FeatureProxyConnector.h"
#include "IconImages.h"
#include "ImportDescriptor.h"
#include "MenuBar.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "ProgressResource.h"
#include "QueryOptions.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Slot.h"
#include "ToolBar.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/any.hpp>

#include <QtGui/QBitmap>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QMenu>
#include <QtGui/QVBoxLayout>

using namespace std;
XERCES_CPP_NAMESPACE_USE

#define APP_FEATUREMANAGER_REFRESH_ACTION "APP_FEATUREMANAGER_REFRESH_ACTION"
#define APP_FEATUREMANAGER_SEPARATOR_ACTION "APP_FEATUREMANAGER_SEPARATOR_ACTION"
#define APP_FEATUREMANAGER_EXPORT_ACTION "APP_FEATUREMANAGER_EXPORT_ACTION"
#define APP_FEATUREMANAGER_PROPERTIES_ACTION "APP_FEATUREMANAGER_PROPERTIES_ACTION"

const string FeatureManager::PLUGIN_NAME = "Feature Manager";
const string FeatureManager::MENU_NAME = "Geographic Features";
const string FeatureManager::EDIT = "Edit this menu";
const string FeatureManager::IMPORT = "Import geodatabase";

const unsigned int FeatureManager::mCurrentVersion = 1;

FeatureManager::FeatureManager() :
   mpExplorer(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &FeatureManager::updateContextMenu)),
   mpSeparator(NULL),
   mpProxy(NULL)
{
   AlgorithmShell::setName(PLUGIN_NAME);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{2AF907E2-DB70-4567-9623-CF0C0BBE06F5}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   destroyAfterExecute(false);
   executeOnStartup(true);
}

FeatureManager::~FeatureManager()
{
   if (mpProxy != NULL)
   {
      delete mpProxy;
   }
   vector<DataElement*> featureClasses = Service<ModelServices>()->getElements("FeatureClass");
   for (vector<DataElement*>::iterator element = featureClasses.begin(); element != featureClasses.end(); ++element)
   {
      Any* pAnyElement = dynamic_cast<Any*>(*element);
      if (model_cast<FeatureClass*>(*element) != NULL)
      {
         pAnyElement->setData(NULL);
      }
   }
   ToolBar* pToolBar = static_cast<ToolBar*>(Service<DesktopServices>()->getWindow(MENU_NAME, TOOLBAR));
   if (pToolBar != NULL)
   {
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      if (pMenuBar != NULL)
      {
         for_each(mImportActions.begin(), mImportActions.end(),
            boost::bind(&MenuBar::removeMenuItem, pMenuBar, _1));
      }
   }
   mImportActions.clear();
}

bool FeatureManager::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   return true;
}

bool FeatureManager::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   return true;
}

bool FeatureManager::execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList)
{
   StepResource pStep("Start feature manager", "app", "0FEB5273-6667-42D0-9C0A-FAB4DFAB5498",
      "Unable to start feature manager");

   if (isBatch() == false)
   {
      Service<DesktopServices> pDesktop;
      ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->createWindow(MENU_NAME, TOOLBAR));
      VERIFY(pToolBar != NULL);
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      VERIFY(pMenuBar != NULL);

      if (addMenuItems(*pMenuBar) == false)
      {
         return false;
      }

      if (refresh() == false)
      {
         return false;
      }
   }

   pStep->finalize(Message::Success);
   return true;
}

bool FeatureManager::addMenuItems(MenuBar &menuBar)
{
   QAction* pEditAction = menuBar.addCommand(MENU_NAME + "\\" + EDIT, PLUGIN_NAME + "/" + EDIT);
   QAction* pImportGeodatabaseAction = menuBar.addCommand(MENU_NAME + "\\" + IMPORT, PLUGIN_NAME + "/" + IMPORT);
   VERIFY(pEditAction != NULL && pImportGeodatabaseAction != NULL);
   pEditAction->setAutoRepeat(false);
   pImportGeodatabaseAction->setAutoRepeat(false);

   QMenu* pMenu = menuBar.getMenu(menuBar.getMenuItem(MENU_NAME));
   VERIFY(pMenu != NULL);

   mpSeparator = pMenu->insertSeparator(pEditAction);
   VERIFY(mpSeparator != NULL);

   VERIFY(connect(pEditAction, SIGNAL(triggered()), this, SLOT(openEditDlg())));
   VERIFY(connect(pImportGeodatabaseAction, SIGNAL(triggered()), this, SLOT(importGeodatabase())));
   VERIFY(connect(pMenu, SIGNAL(triggered(QAction*)), this, SLOT(menuItemTriggered(QAction*))));

   return true;
}

bool FeatureManager::refresh()
{
   Service<DesktopServices> pDesktop;

   ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow(MENU_NAME, TOOLBAR));
   VERIFY(pToolBar != NULL);
   MenuBar* pMenuBar = pToolBar->getMenuBar();
   VERIFY(pMenuBar != NULL);

   return refresh(*pMenuBar);
}

bool FeatureManager::refresh(MenuBar &menuBar)
{
   for_each(mImportActions.begin(), mImportActions.end(), boost::bind(&MenuBar::removeMenuItem, &menuBar, _1));
   mImportActions.clear();

   // generate new actions

   const DynamicObject* pOptionsSet = FeatureManager::getSettingOptions();
   if (pOptionsSet == NULL)
   {
      return true; // No options is a valid state
   }

   Service<DesktopServices> pDesktop;
   vector<string> names;
   pOptionsSet->getAttributeNames(names);
   for (vector<string>::const_iterator iter = names.begin();
      iter != names.end(); ++iter)
   {
      string menuLocation = MENU_NAME + "\\" + *iter;
      QAction* pAction = menuBar.addCommand(menuLocation, mpSeparator);
      VERIFY(pAction != NULL);
      pAction->setAutoRepeat(false);
      pDesktop->initializeAction(pAction, PLUGIN_NAME + "/" + *iter);
      mImportActions.push_back(pAction);
   }

   return true;
}

void FeatureManager::menuItemTriggered(QAction *pAction)
{
   vector<QAction*>::size_type i;
   for (i = 0; i < mImportActions.size(); ++i)
   {
      if (mImportActions[i] == pAction)
      {
         break;
      }
   }
   if (i >= mImportActions.size())
   {
      return; // not found
   }

   const DynamicObject* pOptionsSet = FeatureManager::getSettingOptions();

   if (pOptionsSet == NULL)
   {
      return;
   }

   VERIFYNRV(pOptionsSet->getNumAttributes() == mImportActions.size());

   QString name = pAction->text();
   DataVariant var = pOptionsSet->getAttribute(name.toStdString());
   DynamicObject* pOptions = var.getPointerToValue<DynamicObject>();
   if (pOptions == NULL)
   {
      return;
   }

   ProgressResource pProgress("Importing " + name.toStdString());

   ImporterResource featureLayerImporter(FeatureLayerImporter::PLUGIN_NAME, 
      FeatureLayerImporter::FILE_PLACEHOLDER, pProgress.get(), false);

   featureLayerImporter->getInArgList().setPlugInArgValue(FeatureLayerImporter::FEATURE_DYNAMIC_OBJECT_ARG, pOptions);
   featureLayerImporter->execute();
}

void FeatureManager::openEditDlg()
{
   FeatureMenuEditorDlg dlg(Service<DesktopServices>()->getMainWidget());
   if (dlg.exec() == QDialog::Accepted)
   {
      refresh();
   }
}

void FeatureManager::updateContextMenu(Subject &subject, const string &signal, const boost::any &data)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(data);
   if (pMenu == NULL)
   {
      return;
   }

   vector<FeatureClass*> featureClasses = getFeatureClasses(pMenu->getSessionItems());
   if (featureClasses.empty() || featureClasses.size() != pMenu->getSessionItems().size())
   {
      return;
   }

   QAction* pRefreshAction = new QAction("Refresh", pMenu->getActionParent());
   pRefreshAction->setAutoRepeat(false);
   VERIFYNR(connect(pRefreshAction, SIGNAL(triggered()), this, SLOT(refreshFeatureClass())));
   pMenu->addActionBefore(pRefreshAction, APP_FEATUREMANAGER_REFRESH_ACTION, APP_APPLICATIONWINDOW_EXPORT_ACTION);

   if (featureClasses.size() == 1)
   {
      // Separator
      QAction* pSeparatorAction = new QAction(pMenu->getActionParent());
      pSeparatorAction->setSeparator(true);
      pMenu->addActionAfter(pSeparatorAction, APP_FEATUREMANAGER_SEPARATOR_ACTION, APP_FEATUREMANAGER_REFRESH_ACTION);

      // Replace the default Export action
      QPixmap exportPix(IconImages::SaveIcon);
      exportPix.setMask(exportPix.createHeuristicMask());
      QIcon exportIcon(exportPix);

      QAction* pExportAction = new QAction(exportIcon, "Export", pMenu->getActionParent());
      pExportAction->setAutoRepeat(false);
      VERIFYNR(connect(pExportAction, SIGNAL(triggered()), this, SLOT(exportFeatureClass())));
      pMenu->addActionAfter(pExportAction, APP_FEATUREMANAGER_EXPORT_ACTION, APP_FEATUREMANAGER_SEPARATOR_ACTION);

      pMenu->removeAction(APP_APPLICATIONWINDOW_EXPORT_ACTION);

      // Replace the default Properties action
      QPixmap propertiesPix(IconImages::PropertiesIcon);
      propertiesPix.setMask(propertiesPix.createHeuristicMask());
      QIcon propertiesIcon(propertiesPix);

      QAction* pPropertiesAction = new QAction(propertiesIcon, "&Properties...", pMenu->getActionParent());
      pPropertiesAction->setAutoRepeat(false);
      VERIFYNR(connect(pPropertiesAction, SIGNAL(triggered()), this, SLOT(displayFeatureClassProperties())));
      pMenu->addActionAfter(pPropertiesAction, APP_FEATUREMANAGER_PROPERTIES_ACTION, APP_FEATUREMANAGER_EXPORT_ACTION);

      pMenu->removeAction(APP_APPLICATIONWINDOW_PROPERTIES_ACTION);
   }
}

void FeatureManager::refreshFeatureClass()
{
   Service<SessionExplorer> pExplorer;
   vector<SessionItem*> filteredSessionItems;
   vector<FeatureClass*> featureClasses = getFeatureClasses(
      pExplorer->getSelectedSessionItems(), &filteredSessionItems);

   VERIFYNRV(filteredSessionItems.size() == featureClasses.size());

   ProgressResource pProgress("Geographic feature");
   string errorMessage;

   bool success = true;
   for (size_t i = 0; i < featureClasses.size() && success; ++i)
   {
      // collapse and re-expand session items to prevent N^2 type behavior in the SessionExplorer
      bool expanded = pExplorer->isSessionItemExpanded(filteredSessionItems[i]);
      if (expanded)
      {
         pExplorer->collapseSessionItem(filteredSessionItems[i]);
      }
      success = featureClasses[i]->update(pProgress.get(), errorMessage);
      if (expanded)
      {
         pExplorer->expandSessionItem(filteredSessionItems[i]);
      }
   }

   if (success)
   {
      pProgress->updateProgress("Complete", 100, NORMAL);
   }
   else
   {
      pProgress->updateProgress("Error: " + errorMessage, 0, ERRORS);
   }
}

void FeatureManager::exportFeatureClass()
{
   Service<SessionExplorer> pExplorer;
   vector<FeatureClass*> featureClasses = getFeatureClasses(pExplorer->getSelectedSessionItems());
   if (featureClasses.size() != 1)
   {
      return;
   }

   FeatureClass* pFeatureClass = featureClasses.front();

   QString filename = QFileDialog::getSaveFileName(Service<DesktopServices>()->getMainWidget(), 
      "Export to", QString(), "Geographic feature layer (*.geolayer)");
   if (filename.isEmpty())
   {
      return;
   }

   FileResource pFile(filename.toStdString().c_str(), "wt");
   VERIFYNRV(pFile != NULL);

   FactoryResource<DynamicObject> pDynObj = pFeatureClass->toDynamicObject();

   XMLWriter writer("GeoFeatureLayer");
   pDynObj->toXml(&writer);
   writer.writeToFile(pFile);
}

void FeatureManager::displayFeatureClassProperties()
{
   vector<SessionItem*> sessionItems;

   Service<SessionExplorer> pExplorer;
   vector<FeatureClass*> featureClasses = getFeatureClasses(pExplorer->getSelectedSessionItems(), &sessionItems);
   if (featureClasses.size() != 1)
   {
      return;
   }

   VERIFYNRV(sessionItems.size() == featureClasses.size());

   SessionItem* pItem = sessionItems.front();
   if (pItem != NULL)
   {
      vector<string> propertiesPages;
      propertiesPages.push_back("Feature Class Properties");

      Service<DesktopServices> pDesktop;
      pDesktop->displayProperties(pItem, propertiesPages, false);
   }
}

void FeatureManager::importGeodatabase()
{
   ProgressResource pProgressRes("Geodatabase import");
   Progress* pProgress = pProgressRes.get();
   StepResource pStep("Run Algorithm", "app", "F218A414-6C65-457a-B012-AF676FC44C26");

   ImporterResource pImporter(FeatureLayerImporter::PLUGIN_NAME, 
      FeatureLayerImporter::FILE_PLACEHOLDER, pProgress);

   vector<ImportDescriptor*> descriptors = pImporter->getImportDescriptors();
   FAIL_IF(descriptors.size() != 1, "Could not get the import descriptor", return);

   DataDescriptor* pDescriptor = descriptors.front()->getDataDescriptor();
   FAIL_IF(pDescriptor == NULL, "Could not get the data descriptor", return);

   Importer* pImporterPlugIn = dynamic_cast<Importer*>(pImporter->getPlugIn());
   FAIL_IF(pImporterPlugIn == NULL, "Could not find the importer", return);

   FeatureClassWidget* pProperties = dynamic_cast<FeatureClassWidget*>(
      pImporterPlugIn->getImportOptionsWidget(pDescriptor));

   FAIL_IF(pProperties == NULL, "Could not get properties widget", return);

   Service<DesktopServices> pDesktop;
   auto_ptr<QDialog> pDialog(new QDialog(pDesktop->getMainWidget()));
  
   QDialogButtonBox* pButtons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, pDialog.get());

   QFrame* pHLine = new QFrame(pDialog.get());
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   QVBoxLayout* pLayout = new QVBoxLayout(pDialog.get());
   pProperties->layout()->setMargin(0);
   pLayout->addWidget(pProperties);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pButtons);

   FAIL_IF(!QObject::connect(pButtons, SIGNAL(accepted()), pDialog.get(),
      SLOT(accept())), "Could not connect buttons", return);
   FAIL_IF(!QObject::connect(pButtons, SIGNAL(rejected()), pDialog.get(),
      SLOT(reject())), "Could not connect buttons", return);

   int result = pDialog->exec();

   pProperties->setParent(NULL);

   if (result == QDialog::Accepted)
   {
      if (pImporter->execute())
      {
         pStep->finalize();
      }
      else
      {
         pStep->finalize(Message::Failure, "Geodatabase import failed");
      }
   }
   return;
}

void FeatureManager::proxyInitialized()
{
   MessageResource msg("ArcGIS proxy has been successfully initialized.",
      "app", "BDEC90B2-60EE-493A-A912-761531B17005");
}

FeatureProxyConnector *FeatureManager::getProxy()
{
   if (mpProxy == NULL)
   {
      // Start up ArcProxy
      Service<ConfigurationSettings> pSettings;
      string plugInPath = pSettings->getPlugInPath();
      string arcProxyExec = FeatureManager::getSettingArcProxyExecutable();
#if defined(WIN_API)
      QString proxyPath = QString::fromStdString(
         plugInPath + SLASH + "ArcProxy" + SLASH + arcProxyExec + EXE_EXTENSION);
#elif defined(UNIX_API)
      QString proxyPath = QString::fromStdString(
         plugInPath + SLASH + "ArcProxy" + SLASH + "sol" + arcProxyExec + EXE_EXTENSION);
#else
#error "Don't know how to start ArcProxy"
#endif

      mpProxy = new FeatureProxyConnector(proxyPath);
      VERIFYRV(mpProxy != NULL, NULL);
      connect(mpProxy, SIGNAL(initialized()), this, SLOT(proxyInitialized()));

      mpProxy->initialize();

      Service<SessionExplorer> pExplorer;
      mpExplorer.reset(pExplorer.get());

      Service<ModelServices> pModel;
      pModel->addElementType("FeatureClass");
   }

   return mpProxy;
}

vector<FeatureClass*> FeatureManager::getFeatureClasses(const vector<SessionItem*> sessionItems,
                                                              vector<SessionItem*> *pFilteredSessionItems)
{
   if (pFilteredSessionItems != NULL)
   {
      pFilteredSessionItems->clear();
   }
   vector<FeatureClass*> featureClasses;
   for (vector<SessionItem*>::const_iterator iter = sessionItems.begin();
      iter != sessionItems.end(); ++iter)
   {
      SessionItem* pItem = *iter;
      if (pItem == NULL)
      {
         continue;
      }

      FeatureClass* pFeatureClass = model_cast<FeatureClass*>(pItem);
      if (pFeatureClass != NULL)
      {
         featureClasses.push_back(pFeatureClass);
         if (pFilteredSessionItems != NULL)
         {
            pFilteredSessionItems->push_back(pItem);
         }
         continue;
      }

      AnnotationElement* pElement = NULL;
      
      AnnotationLayer* pLayer = dynamic_cast<AnnotationLayer*>(pItem);
      if (pLayer != NULL)
      {
         pElement = dynamic_cast<AnnotationElement*>(pLayer->getDataElement());
      }
      else
      {
         pElement = dynamic_cast<AnnotationElement*>(pItem);
      }

      if (pElement != NULL)
      {
         Service<ModelServices> pModel;
         vector<DataElement*> elements = pModel->getElements(pElement, "FeatureClass");
         for (vector<DataElement*>::const_iterator elementIter = elements.begin(); 
            elementIter != elements.end(); ++elementIter)
         {
            pFeatureClass = model_cast<FeatureClass*>(*elementIter);
            if (pFeatureClass != NULL)
            {
               featureClasses.push_back(pFeatureClass);
               if (pFilteredSessionItems != NULL)
               {
                  if (pLayer != NULL)
                  {
                     pFilteredSessionItems->push_back(pLayer);
                  }
                  else
                  {
                     pFilteredSessionItems->push_back(pElement);
                  }
               }
            }
         }
      }
   }

   return featureClasses;
}

bool FeatureManager::serialize(SessionItemSerializer &serializer) const
{
   bool success = false;
   XMLWriter writer("FeatureManager");
   ToolBar* pToolBar = static_cast<ToolBar*>(Service<DesktopServices>()->getWindow(MENU_NAME, TOOLBAR));
   if (pToolBar != NULL)
   {
      success = writer.addAttr("toolbarId", pToolBar->getId());
   }
   vector<DataElement*> elements = Service<ModelServices>()->getElements("FeatureClass");
   for (vector<DataElement*>::iterator element = elements.begin(); element != elements.end(); ++element)
   {
      FeatureClass* pFeatureClass = model_cast<FeatureClass*>(*element);
      if (pFeatureClass != NULL)
      {
         writer.pushAddPoint(writer.addElement("FeatureClass"));
         writer.addAttr("dataElementId", (*element)->getId());
         writer.addAttr("parentElementId", (*element)->getParent()->getId());
         FactoryResource<DynamicObject> dynobj(pFeatureClass->toDynamicObject());
         if (dynobj.get() == NULL)
         {
            return false;
         }
         dynobj->toXml(&writer);
         writer.popAddPoint();
      }
   }
   success = success && serializer.serialize(writer);
   return success;
}

bool FeatureManager::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "FeatureManager");
   if (pRootElement == NULL)
   {
      return false;
   }
   string id = A(pRootElement->getAttribute(X("toolbarId")));
   ToolBar* pToolBar = dynamic_cast<ToolBar*>(Service<SessionManager>()->getSessionItem(id));
   if (pToolBar != NULL)
   {
      MenuBar* pMenuBar = pToolBar->getMenuBar();
      if (pMenuBar != NULL)
      {
         addMenuItems(*pMenuBar);
         if (!refresh(*pMenuBar))
         {
            return false;
         }
      }
   }
   for (DOMNode *pChld = pRootElement->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("FeatureClass")))
      {
         DOMElement* pElmnt = static_cast<DOMElement*>(pChld);
         string elementId = A(pElmnt->getAttribute(X("dataElementId")));
         Any* pElement = dynamic_cast<Any*>(Service<SessionManager>()->getSessionItem(elementId));
         if (pElement == NULL)
         {
            return false;
         }
         FeatureClass* pFeatureClass = new FeatureClass();
         FactoryResource<DynamicObject> dynobj;
         VERIFY(pFeatureClass != NULL && dynobj.get() != NULL);
         if (!dynobj->fromXml(pChld, XmlBase::VERSION) || !pFeatureClass->fromDynamicObject(dynobj.get()))
         {
            return false;
         }
         // set parent
         string parentId = A(pElmnt->getAttribute(X("parentElementId")));
         GraphicElement* pParent = dynamic_cast<GraphicElement*>(Service<SessionManager>()->getSessionItem(parentId));
         if (pParent != NULL)
         {
            pFeatureClass->setParentElement(pParent);
         }
         pElement->setData(pFeatureClass);
      }
   }

   // force attachment to session explorer
   getProxy();

   return true;
}
