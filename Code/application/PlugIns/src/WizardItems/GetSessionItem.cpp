/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "CartesianPlot.h"
#include "ClassificationLayer.h"
#include "DataElement.h"
#include "DataElementGroup.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "GetSessionItem.h"
#include "GraphicElement.h"
#include "GraphicLayer.h"
#include "HistogramPlot.h"
#include "LatLonLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementLayer.h"
#include "MessageLogResource.h"
#include "OrthographicView.h"
#include "PerspectiveView.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PlotWindow.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PolarPlot.h"
#include "ProductView.h"
#include "PseudocolorLayer.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "Signature.h"
#include "SignaturePlot.h"
#include "SignatureSet.h"
#include "SignatureLibrary.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"
#include "TiePointList.h"
#include "TiePointLayer.h"
#include "TypeConverter.h"
#include "View.h"
#include "ViewWindow.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>

#include <memory>
#include <string>
#include <vector>

namespace
{
   std::vector<View*> getViews()
   {
      std::vector<View*> views;
      std::vector<Window*> windows;
      Service<DesktopServices>()->getWindows(windows);
      for (std::vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
      {
         ViewWindow* pViewWindow = dynamic_cast<ViewWindow*>(*iter);
         if (pViewWindow != NULL)
         {
            View* pView = pViewWindow->getView();
            if (pView != NULL)
            {
               views.push_back(pView);
            }
         }

         PlotWindow* pPlotWindow = dynamic_cast<PlotWindow*>(*iter);
         if (pPlotWindow != NULL)
         {
            std::vector<PlotWidget*> plots;
            pPlotWindow->getPlots(plots);
            for (std::vector<PlotWidget*>::const_iterator plotIter = plots.begin();
               plotIter != plots.end();
               ++plotIter)
            {
               if (*plotIter != NULL)
               {
                  View* pView = (*plotIter)->getPlot();
                  if (pView != NULL)
                  {
                     views.push_back(pView);
                  }
               }
            }
         }
      }

      return views;
   }
}

template<class T>
bool GetSessionItemBase<T>::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   if (isBatch() == false)
   {
      if (DesktopItems::getInputSpecification(pArgList) == false || pArgList == NULL)
      {
         return false;
      }

      VERIFY(pArgList->addArg<std::string>("Dialog Caption", mDialogCaption,
         "Dialog caption to display when selecting a session item."));
   }

   return true;
}

template<class T>
bool GetSessionItemBase<T>::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   if (isBatch() == false)
   {
      VERIFY(pArgList = Service<PlugInManagerServices>()->getPlugInArgList());
      VERIFY(pArgList->addArg<T>("Session Item", NULL, std::string(TypeConverter::toString<T>()) + " chosen by the Get operation."));
   }

   return true;
}

template<class T>
bool GetSessionItemBase<T>::extractInputArgs(PlugInArgList* pInArgList)
{
   VERIFY(Service<ApplicationServices>()->isBatch() == false);
   return WizardItems::extractInputArgs(pInArgList) &&
      pInArgList->getPlugInArgValue("Dialog Caption", mDialogCaption) && mDialogCaption.empty() == false;
}

template<class T>
bool GetSessionItemBase<T>::populateOutputArgs(PlugInArgList* pOutArgList)
{
   VERIFY(pOutArgList != NULL);
   return pOutArgList->setPlugInArgValue("Session Item", mpSessionItem);
}

template<class T>
bool GetSessionItemBase<T>::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "{234E6866-C61D-4ca8-9152-8CA3DCEFC3C0}");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      reportError("Invalid arguments.", "{0012BA62-EE8E-451e-B725-26D7335436AC}");
      return false;
   }

   if (extractInputArgs(pInArgList) == false)
   {
      reportError("Unable to extract input arguments.", "{365B8383-651C-421f-87D4-01238F4E3398}");
      return false;
   }

   // Create the dialog.
   QDialog dialog(Service<DesktopServices>()->getMainWidget());

   // Tree view containing available session items.
   QStringList columnNames;
   columnNames << "Name" << "Type";

   QTreeWidget* pTree = new QTreeWidget(&dialog);
   pTree->setColumnCount(columnNames.count());
   pTree->setHeaderLabels(columnNames);

   std::auto_ptr<QTreeWidgetItem> pRoot(new QTreeWidgetItem);
   pRoot->setFlags(Qt::NoItemFlags);
   pRoot->setText(GetSessionItemBase<T>::NameColumn, "No items available");
   pRoot->setData(GetSessionItemBase<T>::NameColumn,
      GetSessionItemBase<T>::SessionItemRole, QVariant::fromValue<void*>(NULL));
   populateTreeWidgetItem(pRoot.get());

   if (pRoot->childCount() > 0)
   {
      pTree->addTopLevelItems(pRoot->takeChildren());
   }
   else
   {
      pTree->addTopLevelItem(pRoot.release());
   }

   pTree->expandAll();
   pTree->resizeColumnToContents(0);

   // Buttons.
   QFrame* pLine = new QFrame(&dialog);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   QDialogButtonBox* pButtonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);

   // Layout.
   QVBoxLayout* pLayout = new QVBoxLayout(&dialog);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pTree);
   pLayout->addWidget(pLine);
   pLayout->addWidget(pButtonBox);
   dialog.setWindowTitle(QString::fromStdString(mDialogCaption));
   dialog.resize(400, 400);

   // Connections.
   VERIFY(QObject::connect(pButtonBox, SIGNAL(accepted()), &dialog, SLOT(accept())));
   VERIFY(QObject::connect(pButtonBox, SIGNAL(rejected()), &dialog, SLOT(reject())));
   if (dialog.exec() != QDialog::Accepted)
   {
      reportError("User cancelled.", "{27E33A95-0DFB-486b-ABAE-BFC849418201}");
      return false;
   }

   QTreeWidgetItem* pItem = pTree->currentItem();
   if (pItem == NULL)
   {
      reportError("No item selected.", "{27B21666-19BB-4932-BF08-A81E340F1A54}");
      return false;
   }

   QVariant value = pItem->data(GetSessionItemBase<T>::NameColumn, GetSessionItemBase<T>::SessionItemRole);
   SessionItem* pSessionItem = reinterpret_cast<SessionItem*>(value.value<void*>());
   mpSessionItem = dynamic_cast<T*>(pSessionItem);
   if (mpSessionItem == NULL)
   {
      reportError("Wrong item type selected.", "{E6D3E131-4E71-4989-9D34-BC9A1157AB8E}");
      return false;
   }

   if (populateOutputArgs(pOutArgList) == false)
   {
      reportError("Unable to populate the output argument list.", "{C3AB6771-50C4-4091-BA39-3D44C82C93A8}");
      return false;
   }

   reportComplete();
   return true;
}

template<class T>
GetSessionItemBase<T>::GetSessionItemBase(const std::string& descriptorId) :
   mDialogCaption("Select " + std::string(TypeConverter::toString<T>())),
   mpSessionItem(NULL)
{
   setName("Get " + std::string(TypeConverter::toString<T>()));
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Gets a " + std::string(TypeConverter::toString<T>()));
   setDescriptorId(descriptorId);
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

template<class T>
GetSessionItemBase<T>::~GetSessionItemBase()
{}

template<class T>
GetDataElement<T>::GetDataElement(const std::string& descriptorId) :
   GetSessionItemBase<T>(descriptorId)
{}

template<class T>
GetDataElement<T>::~GetDataElement()
{}

template<class T>
void GetDataElement<T>::populateTreeWidgetItem(QTreeWidgetItem* pRoot)
{
   VERIFYNR(pRoot != NULL);
   QVariant value = pRoot->data(GetSessionItemBase<T>::NameColumn, GetSessionItemBase<T>::SessionItemRole);
   void* pValue = value.value<void*>();
   DataElement* const pRootElement = reinterpret_cast<DataElement*>(pValue);
   const std::vector<DataElement*> elements = Service<ModelServices>()->getElements(pRootElement, std::string());
   for (std::vector<DataElement*>::const_iterator iter = elements.begin(); iter != elements.end(); ++iter)
   {
      DataElement* pChildElement = *iter;
      if (pChildElement == NULL)
      {
         // Disallow NULL elements since that would result in infinite recursion.
         continue;
      }

      std::auto_ptr<QTreeWidgetItem> pChild(new QTreeWidgetItem);
      std::string name = pChildElement->getDisplayName();
      if (name.empty() == true)
      {
         name = pChildElement->getName();
      }

      pChild->setText(GetSessionItemBase<T>::NameColumn, QString::fromStdString(name));
      pChild->setData(GetSessionItemBase<T>::NameColumn,
         GetSessionItemBase<T>::SessionItemRole, QVariant::fromValue<void*>(pChildElement));
      pChild->setText(GetSessionItemBase<T>::TypeColumn, QString::fromStdString(pChildElement->getType()));
      populateTreeWidgetItem(pChild.get());

      const bool isValid = pChildElement->isKindOf(TypeConverter::toString<T>());
      pChild->setFlags(isValid ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags);
      if (isValid == true || pChild->childCount() > 0)
      {
         pRoot->addChild(pChild.release());
      }
   }
}

template<class T>
GetLayer<T>::GetLayer(const std::string& descriptorId) :
   GetSessionItemBase<T>(descriptorId)
{}

template<class T>
GetLayer<T>::~GetLayer()
{}

template<class T>
void GetLayer<T>::populateTreeWidgetItem(QTreeWidgetItem* pRoot)
{
   VERIFYNR(pRoot != NULL);
   const std::vector<View*> views = getViews();
   for (std::vector<View*>::const_iterator iter = views.begin(); iter != views.end(); ++iter)
   {
      View* pView = *iter;
      VERIFYNR(pView != NULL);

      std::auto_ptr<QTreeWidgetItem> pChild(new QTreeWidgetItem);
      std::string name = pView->getDisplayName();
      if (name.empty() == true)
      {
         name = pView->getName();
      }

      std::vector<std::string> classList;
      Service<DesktopServices>()->getViewTypes(pView->getObjectType(), classList);
      VERIFYNR(classList.empty() == false);

      pChild->setText(GetSessionItemBase<T>::NameColumn, QString::fromStdString(name));
      pChild->setData(GetSessionItemBase<T>::NameColumn,
         GetSessionItemBase<T>::SessionItemRole, QVariant::fromValue<void*>(pView));
      pChild->setText(GetSessionItemBase<T>::TypeColumn, QString::fromStdString(classList.front()));
      pChild->setFlags(Qt::NoItemFlags);

      populateTreeWidgetItemWithLayers(pChild.get());
      if (pChild->childCount() > 0)
      {
         pRoot->addChild(pChild.release());
      }
   }
}

template<class T>
void GetLayer<T>::populateTreeWidgetItemWithLayers(QTreeWidgetItem* pRoot)
{
   VERIFYNR(pRoot != NULL);
   QVariant value = pRoot->data(GetSessionItemBase<T>::NameColumn, GetSessionItemBase<T>::SessionItemRole);
   void* pValue = value.value<void*>();
   View* const pView = reinterpret_cast<View*>(pValue);
   VERIFYNR(pView != NULL);

   std::vector<Layer*> layers;
   SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
   if (pSpatialDataView != NULL)
   {
      LayerList* pLayerList = pSpatialDataView->getLayerList();
      if (pLayerList != NULL)
      {
         pLayerList->getLayers(layers);
      }
   }

   ProductView* pProductView = dynamic_cast<ProductView*>(pView);
   if (pProductView != NULL)
   {
      layers.push_back(pProductView->getLayoutLayer());
      layers.push_back(pProductView->getClassificationLayer());
   }

   PlotView* pPlotView = dynamic_cast<PlotView*>(pView);
   if (pPlotView != NULL)
   {
      layers.push_back(pPlotView->getAnnotationLayer());
   }

   // Add the layer items in reverse order so that the top-most layer is added first
   for (std::vector<Layer*>::reverse_iterator iter = layers.rbegin(); iter != layers.rend(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer == NULL || pLayer->isKindOf(TypeConverter::toString<T>()) == false)
      {
         continue;
      }

      QTreeWidgetItem* pChild = new QTreeWidgetItem;
      std::string name = pLayer->getDisplayName();
      if (name.empty() == true)
      {
         name = pLayer->getName();
      }

      pChild->setText(GetSessionItemBase<T>::NameColumn, QString::fromStdString(name));
      pChild->setData(GetSessionItemBase<T>::NameColumn,
         GetSessionItemBase<T>::SessionItemRole, QVariant::fromValue<void*>(pLayer));
      pChild->setText(GetSessionItemBase<T>::TypeColumn, QString::fromStdString(StringUtilities::toDisplayString(pLayer->getLayerType())));
      pChild->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      pRoot->addChild(pChild);
   }
}

template<class T>
GetView<T>::GetView(const std::string& descriptorId) :
   GetSessionItemBase<T>(descriptorId)
{}

template<class T>
GetView<T>::~GetView()
{}

template<class T>
void GetView<T>::populateTreeWidgetItem(QTreeWidgetItem* pRoot)
{
   VERIFYNR(pRoot != NULL);
   const std::vector<View*> views = getViews();
   for (std::vector<View*>::const_iterator iter = views.begin(); iter != views.end(); ++iter)
   {
      View* pView = *iter;
      VERIFYNR(pView != NULL);
      if (pView->isKindOf(TypeConverter::toString<T>()) == false)
      {
         continue;
      }

      QTreeWidgetItem* pChild = new QTreeWidgetItem;
      std::string name = pView->getDisplayName();
      if (name.empty() == true)
      {
         name = pView->getName();
      }

      std::vector<std::string> classList;
      Service<DesktopServices>()->getViewTypes(pView->getObjectType(), classList);
      VERIFYNR(classList.empty() == false);

      pChild->setText(GetSessionItemBase<T>::NameColumn, QString::fromStdString(name));
      pChild->setData(GetSessionItemBase<T>::NameColumn,
         GetSessionItemBase<T>::SessionItemRole, QVariant::fromValue<void*>(pView));
      pChild->setText(GetSessionItemBase<T>::TypeColumn, QString::fromStdString(classList.front()));
      pChild->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      pRoot->addChild(pChild);
   }
}

GetPlotWidget::GetPlotWidget(const std::string& descriptorId) :
   GetSessionItemBase<PlotWidget>(descriptorId)
{}

GetPlotWidget::~GetPlotWidget()
{}

void GetPlotWidget::populateTreeWidgetItem(QTreeWidgetItem* pRoot)
{
   VERIFYNR(pRoot != NULL);
   std::vector<Window*> windows;
   Service<DesktopServices>()->getWindows(PLOT_WINDOW, windows);
   for (std::vector<Window*>::const_iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      PlotWindow* pPlotWindow = dynamic_cast<PlotWindow*>(*iter);
      if (pPlotWindow == NULL)
      {
         continue;
      }

      std::vector<PlotWidget*> plots;
      pPlotWindow->getPlots(plots);
      for (std::vector<PlotWidget*>::const_iterator plotIter = plots.begin(); plotIter != plots.end(); ++plotIter)
      {
         PlotWidget* pPlotWidget = *plotIter;
         if (pPlotWidget == NULL)
         {
            continue;
         }

         QTreeWidgetItem* pChild = new QTreeWidgetItem;
         std::string name = pPlotWidget->getDisplayName();
         if (name.empty() == true)
         {
            name = pPlotWidget->getName();
         }

         pChild->setText(GetSessionItemBase<PlotWidget>::NameColumn, QString::fromStdString(name));
         pChild->setData(GetSessionItemBase<PlotWidget>::NameColumn,
            GetSessionItemBase<PlotWidget>::SessionItemRole, QVariant::fromValue<void*>(pPlotWidget));
         pChild->setText(GetSessionItemBase<PlotWidget>::TypeColumn,
            QString::fromStdString(TypeConverter::toString<PlotWidget>()));
         pChild->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
         pRoot->addChild(pChild);
      }
   }
}

// DataElement types
#define REGISTER_GET_DATA_ELEMENT(name__, guid__) \
   REGISTER_PLUGIN(OpticksWizardItems, Get##name__, GetDataElement<name__>(guid__))

REGISTER_GET_DATA_ELEMENT(Any, "{6333C3FC-EACD-4350-AB91-64F788681214}");
REGISTER_GET_DATA_ELEMENT(AnnotationElement, "{FE60A02F-ECDB-4635-B98A-F08D563348AC}");
REGISTER_GET_DATA_ELEMENT(AoiElement, "{9D267D4E-ED0C-43a5-841F-4DF73A54FDF5}");
REGISTER_GET_DATA_ELEMENT(GcpList, "{F5E3F5E2-71CB-4e4c-A262-C2DDBD07B8F7}");
REGISTER_GET_DATA_ELEMENT(DataElement, "{160C8DD7-B36C-4e87-9763-CEFB7563B115}");
REGISTER_GET_DATA_ELEMENT(DataElementGroup, "{316B0641-FACD-4165-BCA0-E8DFECA51BBD}");
REGISTER_GET_DATA_ELEMENT(GraphicElement, "{152A5FBD-6A44-4650-8FFC-581B17019C74}");
REGISTER_GET_DATA_ELEMENT(RasterElement, "{4E81EB14-8211-4a3d-8B5C-010AB09EBCC9}");
REGISTER_GET_DATA_ELEMENT(Signature, "{7B77F0B8-A744-4e10-AE5C-229AA265C01C}");
REGISTER_GET_DATA_ELEMENT(SignatureSet, "{7676CF21-9F22-4159-B6F9-F147D7D74A9C}");
REGISTER_GET_DATA_ELEMENT(SignatureLibrary, "{B8CE5181-F4C9-4453-B41C-CE72FE2F289B}");
REGISTER_GET_DATA_ELEMENT(TiePointList, "{20050D77-DBAE-46f7-9A1B-2C1F16AF6872}");

// Layer types
#define REGISTER_GET_LAYER(name__, guid__) \
   REGISTER_PLUGIN(OpticksWizardItems, Get##name__, GetLayer<name__>(guid__))

REGISTER_GET_LAYER(AnnotationLayer, "{990F3151-996D-4732-8706-B6D1AB7B142E}");
REGISTER_GET_LAYER(AoiLayer, "{3272F3ED-9123-48b2-B2DD-3F2AF53DC538}");
REGISTER_GET_LAYER(ClassificationLayer, "{C3024C92-7012-42d6-9AF0-E5D412834137}");
REGISTER_GET_LAYER(GcpLayer, "{F1ED9D09-5BCE-4f8b-A4B1-F412A573A553}");
REGISTER_GET_LAYER(GraphicLayer, "{4B71A94C-DC77-478b-81B4-B55A47E7FA76}");
REGISTER_GET_LAYER(LatLonLayer, "{52ACEFF4-DB58-4492-A7C0-72ED2A0D87C2}");
REGISTER_GET_LAYER(Layer, "{DBCCDA29-6F96-4cac-A8DE-495F1E548843}");
REGISTER_GET_LAYER(MeasurementLayer, "{704E78D3-CF80-4f6e-B8A7-03837DB3F6F0}");
REGISTER_GET_LAYER(PseudocolorLayer, "{E62D0CCE-88DD-4905-928A-B3245A857CF7}");
REGISTER_GET_LAYER(RasterLayer, "{D3DCFCA3-94D5-4c3d-A056-A11149B763EB}");
REGISTER_GET_LAYER(ThresholdLayer, "{93DE93E5-5EE0-41b5-82C1-E19D0AB83D48}");
REGISTER_GET_LAYER(TiePointLayer, "{AFB6094B-5911-446b-9C13-5097B9237232}");

// View types
#define REGISTER_GET_VIEW(name__, guid__) \
   REGISTER_PLUGIN(OpticksWizardItems, Get##name__, GetView<name__>(guid__))

REGISTER_GET_VIEW(CartesianPlot, "{16193698-4032-4625-82D5-87EF97B89CCA}");
REGISTER_GET_VIEW(HistogramPlot, "{E3A1BAD3-85E4-4eca-B030-01D8F16B1968}");
REGISTER_GET_VIEW(OrthographicView, "{4D7E7ED9-2E30-4319-BEB7-C01BBF13C5E0}");
REGISTER_GET_VIEW(PerspectiveView, "{E65C893E-B92A-429b-9B68-DEA2FC0F8BEB}");
REGISTER_GET_VIEW(PlotView, "{CCD84C18-7A5C-4ddf-9AF0-E09F196F33E7}");
REGISTER_GET_VIEW(PolarPlot, "{8942067E-73F5-4370-A47E-0F07EBC31AF8}");
REGISTER_GET_VIEW(ProductView, "{E5658625-A3FB-46cc-9245-4B0FC1AB6A4A}");
REGISTER_GET_VIEW(SignaturePlot, "{767E784C-FC4A-47df-9988-26C4A8367ACD}");
REGISTER_GET_VIEW(SpatialDataView, "{28ED71B1-AE20-4a3a-8268-23F737218A4D}");
REGISTER_GET_VIEW(View, "{05B26D94-0BAA-4050-AE1A-EF159441CB84}");

// Miscellaneous types
REGISTER_PLUGIN(OpticksWizardItems, GetPlotWidget, GetPlotWidget("{826E7186-E387-40c9-8C86-303E94C38B6E}"));