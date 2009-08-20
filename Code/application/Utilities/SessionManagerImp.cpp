/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationAdapter.h"
#include "AnimationController.h"
#include "AnimationServicesImp.h"
#include "AnnotationLayer.h"
#include "AnnotationLayerAdapter.h"
#include "AoiLayerAdapter.h"
#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "CartesianPlotAdapter.h"
#include "ClassificationLayerAdapter.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "DesktopServicesImp.h"
#include "Filename.h"
#include "GcpLayerAdapter.h"
#include "GraphicLayerAdapter.h"
#include "HistogramPlotAdapter.h"
#include "LatLonLayerAdapter.h"
#include "Layer.h"
#include "LayerList.h"
#include "MeasurementLayerAdapter.h"
#include "MessageLogMgrImp.h"
#include "ModelServicesImp.h"
#include "ModuleDescriptor.h"
#include "ObjectResource.h"
#include "PlotSet.h"
#include "PlotView.h"
#include "PlotSet.h"
#include "PlotSetAdapter.h"
#include "PlotWidget.h"
#include "PlotWidgetAdapter.h"
#include "PlotWindow.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInManagerServicesImp.h"
#include "PolarPlotAdapter.h"
#include "ProductViewAdapter.h"
#include "ProductView.h"
#include "Progress.h"
#include "PseudocolorLayerAdapter.h"
#include "RasterLayerAdapter.h"
#include "RasterDataDescriptor.h"
#include "SessionInfoItem.h"
#include "SessionItem.h"
#include "SessionItemDeserializerImp.h"
#include "SessionItemImp.h"
#include "SessionItemSerializerImp.h"
#include "SessionManagerImp.h"
#include "SessionResource.h"
#include "SignaturePlotAdapter.h"
#include "SpatialDataViewAdapter.h"
#include "SpatialDataViewImp.h"
#include "ThresholdLayerAdapter.h"
#include "TiePointLayerAdapter.h"
#include "TypeAwareObject.h"
#include "TypeConverter.h"
#include "View.h"
#include "ViewObjectImp.h"
#include "Window.h"
#include "WorkspaceWindow.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <algorithm>
#include <boost/bind.hpp>
#if defined(WIN_API)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUuid>
#include <QtGui/QWidget>


XERCES_CPP_NAMESPACE_USE
using namespace std;

struct ItemFilename
{
   string operator()(const SessionManagerImp::IndexFileItem &item)
   {
      if (item.mpItem == NULL)
      {
         return string();
      }
      else
      {
         return item.mId + ".sessionItem";
      }
   }
};

SessionManagerImp* SessionManagerImp::spInstance = NULL;
bool SessionManagerImp::mDestroyed = false;

SessionManagerImp* SessionManagerImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw logic_error("Attempting to use SessionManager after "
            "destroying it.");
      }
      spInstance = new SessionManagerImp;
   }

   return spInstance;
}

void SessionManagerImp::destroy()
{
   if (mDestroyed)
   {
      throw logic_error("Attempting to destroy SessionManager after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

SessionManagerImp::SessionManagerImp() :
   mName(SessionItemImp::generateUniqueId()),
   mIsSaveLoad(false),
   mSaveLockCount(0)
{
}

SessionManagerImp::~SessionManagerImp()
{
}

const string& SessionManagerImp::getObjectType() const
{
   static string sType("SessionManagerImp");
   return sType;
}

bool SessionManagerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SessionManager"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void SessionManagerImp::close()
{
   SessionSaveLock lock;
   notify(SIGNAL_NAME(SessionManager, Closed));

   AnimationServicesImp::instance()->clear();
   ModelServicesImp::instance()->clear();
   PlugInManagerServicesImp::instance()->clear();
   DesktopServicesImp::instance()->deleteAllWindows();
   MessageLogMgrImp::instance()->clear();

   mName.clear();
}

SessionItem *SessionManagerImp::createPlotWidget(const string &type, const string &id, const std::string &name)
{
   if (type.empty() || id.empty())
   {
      return NULL;
   }
   Service<DesktopServices> pDesktop;
   VERIFYRV(pDesktop.get() != NULL, NULL);

   PlotWidget* pWidget(NULL);

   size_t first = type.find('/');
   size_t second = type.find('/', first+1);
   if (first != string::npos && second != string::npos)
   {
      string strType = type.substr(first+1, second-first-1);
      if (strType == "CartesianPlotAdapter")
      {
         pWidget = new PlotWidgetAdapter(id, name, CARTESIAN_PLOT, NULL);
      }
      else if (strType == "HistogramPlotAdapter")
      {
         pWidget = new PlotWidgetAdapter(id, name, HISTOGRAM_PLOT, NULL);
      }
      else if (strType == "PolarPlotAdapter")
      {
         pWidget = new PlotWidgetAdapter(id, name, POLAR_PLOT, NULL);
      }
      else if (strType == "SignaturePlotAdapter")
      {
         pWidget = new PlotWidgetAdapter(id, name, SIGNATURE_PLOT, NULL);
      }
   }

   return pWidget;
}

SessionItem *SessionManagerImp::createPlotSet(const string &type, const string &id, const std::string &name)
{
   PlotSet* pPlotSet(NULL);
   pPlotSet = new PlotSetAdapter(id, name, NULL);
   return pPlotSet;
}

SessionItem *SessionManagerImp::createAnimationController(const string &type, const string &id, const std::string &name)
{
   VERIFYRV(type == TypeConverter::toString<AnimationController>(), NULL);
   // create a controller with name == id as a place holder...the name will change during deserialization
   return AnimationServicesImp::instance()->createAnimationController(id, FRAME_ID, id);
}

SessionItem *SessionManagerImp::createAnimation(const string &type, const string &id, const std::string &name)
{
   VERIFYRV(type == TypeConverter::toString<Animation>(), NULL);
   // just create the animation...connections, etc. to a controller will happen during dersialization of the controller
   return new AnimationAdapter(FRAME_ID, id);
}

SessionItem *SessionManagerImp::createAppWindow(const string &type, const string &id, const std::string &name)
{
   VERIFYRV(type == TypeConverter::toString<ApplicationWindow>(), NULL);
   return static_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget());
}

SessionItem *SessionManagerImp::createDataElement(const string &type, const string &id, const std::string &name)
{
   QStringList typeComponents = QString::fromStdString(type).split("/");
   QString deType = typeComponents[0];
   if (typeComponents.size() > 1)
   {
      VERIFYRV(typeComponents.size() == 2 && typeComponents[0] == "DataElement", NULL);
      deType = typeComponents[1];
   }
   if (deType.endsWith("Adapter"))
   {
      deType.chop(7);
   }
   if (deType.endsWith("Imp"))
   {
      deType.chop(3);
   }
   // create a DataDescriptor with name == id...the name will be set during DataElement deserialization
   DataDescriptorResource<DataDescriptor> pDescriptor(id, deType.toStdString(), NULL);
   VERIFYRV(pDescriptor.get() != NULL, NULL);
   if (deType == "RasterElement")
   {
      // need to fool ModelServices since it will be unable to create an in-memory pager right now
      static_cast<RasterDataDescriptor*>(pDescriptor.get())->setProcessingLocation(ON_DISK_READ_ONLY);
   }
   return ModelServicesImp::instance()->createElement(pDescriptor.get(), id);
}

SessionItem *SessionManagerImp::createPlugInInstance(const string &type, const string &id, const std::string &name)
{
   int pos = type.find("/");
   if (pos == string::npos)
   {
      return NULL;
   }
   else
   {
      string name = type.substr(pos+1);
      return PlugInManagerServicesImp::instance()->createPlugInInstance(name, id);
   }
}

SessionItem *SessionManagerImp::createModule(const string &type, const string &id, const std::string &name)
{
   const vector<ModuleDescriptor*>& modules = PlugInManagerServicesImp::instance()->getModuleDescriptors();
   vector<ModuleDescriptor*>::const_iterator ppModule;
   for (ppModule = modules.begin(); ppModule != modules.end(); ++ppModule)
   {
      ModuleDescriptor* pModule = *ppModule;
      LOG_IF(pModule == NULL, continue);
      if ("Module/" + pModule->getName() == type)
      {
         return pModule;
      }
   }
   return NULL;
}

SessionItem *SessionManagerImp::createPlugInDescriptor(const string &type, const string &id, const std::string &name)
{
   string localType = type;
   int pos = localType.find_first_of("/");
   if (pos != string::npos)
   {
      localType = localType.substr(pos+1);
   }
   return PlugInManagerServicesImp::instance()->getPlugInDescriptor(localType);
}

SessionItem *SessionManagerImp::createLayer(const string &type, const string &id, const std::string &name)
{
   if (type.empty() || id.empty())
   {
      return NULL;
   }
   Service<DesktopServices> pDesktop;
   VERIFYRV(pDesktop.get() != NULL, NULL);

   Layer* pLayer(NULL);

   size_t first = type.find('/');
   size_t second = type.length();
   if (first != string::npos && second != string::npos)
   {
      string strType = type.substr(first+1, second-first-1);
      if (strType == "AoiLayerAdapter")
      {
         pLayer = new AoiLayerAdapter(id, name, NULL);
      }
      else if (strType == "AnnotationLayerAdapter")
      {
         pLayer = new AnnotationLayerAdapter(id, name, NULL);
      }
      else if (strType == "ClassificationLayerAdapter")
      {
         pLayer = new ClassificationLayerAdapter(id, name, NULL);
      }
      else if (strType == "GcpLayerAdapter")
      {
         pLayer = new GcpLayerAdapter(id, name, NULL);
      }
      else if (strType == "GraphicLayerAdapter")
      {
         pLayer = new GraphicLayerAdapter(id, name, NULL);
      }
      else if (strType == "LatLonLayerAdapter")
      {
         pLayer = new LatLonLayerAdapter(id, name, NULL);
      }
      else if (strType == "MeasurementLayerAdapter")
      {
         pLayer = new MeasurementLayerAdapter(id, name, NULL);
      }
      else if (strType == "PseudocolorLayerAdapter")
      {
         pLayer = new PseudocolorLayerAdapter(id, name, NULL);
      }
      else if (strType == "RasterLayerAdapter")
      {
         pLayer = new RasterLayerAdapter(id, name, NULL);
      }
      else if (strType == "ThresholdLayerAdapter")
      {
         pLayer = new ThresholdLayerAdapter(id, name, NULL);
      }
      else if (strType == "TiePointLayerAdapter")
      {
         pLayer = new TiePointLayerAdapter(id, name, NULL);
      }
   }

   return pLayer;
}

SessionItem *SessionManagerImp::createWindow(const string &type, const string &id, const std::string &name)
{
   if (type.empty() || id.empty())
   {
      return NULL;
   }
   Service<DesktopServices> pDesktop;
   VERIFYRV(pDesktop.get() != NULL, NULL);

   Window* pWindow(NULL);
   pWindow = pDesktop->getWindow(id);
   if (pWindow == NULL)
   {
      // get window type
      size_t first = type.find('/');
      size_t second = type.find('/', first+1);
      if (first != string::npos && second != string::npos)
      {
         string strType = type.substr(first+1, second-first-1);
         bool bError = true;
         WindowType eType = StringUtilities::fromXmlString<WindowType>(strType, &bError);
         if (bError == false)
         {
            // check for window by name and type - windows that don't have same id each time
            pWindow = pDesktop->getWindow(name, eType);
            if (pWindow == NULL)
            {
               // check for PlotWindow stored as DockWindow in ApplicationWindow
               if (eType == PLOT_WINDOW)
               {
                  pWindow = pDesktop->getWindow(name, DOCK_WINDOW);
               }

               // now if not found, create
               if (pWindow == NULL)
               {
                  pWindow = pDesktop->createWindow(name, eType);
               }
            }
         }
      }
   }

   return pWindow;
}

SessionItem *SessionManagerImp::createView(const string &type, const string &id, const std::string &name)
{
   if (type.empty() || id.empty())
   {
      return NULL;
   }

   View* pView(NULL);

   size_t first = type.find('/');
   size_t second = type.length();
   if (first != string::npos && second != string::npos)
   {
      string strType = type.substr(first+1, second-first-1);
      if (strType == "SpatialDataViewAdapter")
      {
         pView = new SpatialDataViewAdapter(id, name, NULL, 0);
      }
      else if (strType == "ProductViewAdapter")
      {
         pView = new ProductViewAdapter(id, name, NULL, 0);
      }
      else if (strType == "CartesianPlotAdapter")
      {
         pView = new CartesianPlotAdapter(id, name, NULL, 0);
      }
      else if (strType == "SignaturePlotAdapter")
      {
         pView = new SignaturePlotAdapter(id, name, NULL, 0);
      }
      else if (strType == "HistogramPlotAdapter")
      {
         pView = new HistogramPlotAdapter(id, name, NULL, 0);
      }
      else if (strType == "PolarPlotAdapter")
      {
         pView = new PolarPlotAdapter(id, name, NULL, 0);
      }
   }

   return pView;
}

SessionItem *SessionManagerImp::createSessionInfo(const string &type, const string &id, const std::string &name)
{
   VERIFYRV(type == "SessionInfo", NULL);
   SessionInfoItem* pInfo = new SessionInfoItem(id, name);
   return pInfo;
}

void SessionManagerImp::destroyPlotWidget(SessionItem *pItem)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Needs to be implemented (tjohnson)")
}

void SessionManagerImp::destroyPlotSet(SessionItem *pItem)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Needs to be implemented (tjohnson)")
}

void SessionManagerImp::destroyAnimationController(SessionItem *pItem)
{
   AnimationController* pController = dynamic_cast<AnimationController*>(pItem);
   if (pController != NULL)
   {
      Service<AnimationServices>()->destroyAnimationController(pController);
   }
}

void SessionManagerImp::destroyAnimation(SessionItem *pItem)
{
   Animation* pAnimation = dynamic_cast<Animation*>(pItem);
   if (pAnimation != NULL)
   {
      vector<AnimationController*> controllers = Service<AnimationServices>()->getAnimationControllers();
      for (vector<AnimationController*>::iterator controller = controllers.begin();
               controller != controllers.end(); ++controller)
      {
         if ((*controller)->getAnimation(pAnimation->getName()) == pAnimation)
         {
            (*controller)->destroyAnimation(pAnimation);
            return;
         }
      }
      // if this point is reached, the Animation has not been added to any AnimationController
      delete dynamic_cast<AnimationImp*>(pAnimation);
   }
}

void SessionManagerImp::destroyAppWindow(SessionItem *pItem)
{
   // Nothing to do. There is no need to destroy the ApplicationWindow if it fails to deserialize.
}

void SessionManagerImp::destroyDataElement(SessionItem *pItem)
{
   DataElement* pElement = dynamic_cast<DataElement*>(pItem);
   if (pElement != NULL)
   {
      Service<ModelServices>()->destroyElement(pElement);
   }
}

void SessionManagerImp::destroyPlugInInstance(SessionItem *pItem)
{
   PlugIn* pPlugIn = dynamic_cast<PlugIn*>(pItem);
   VERIFYNRV(pPlugIn != NULL);
   Service<PlugInManagerServices>()->destroyPlugIn(pPlugIn);
}

void SessionManagerImp::destroyModule(SessionItem *pItem)
{
   // Nothing to do. There is no need to kill the module if it fails to deserialize.
}

void SessionManagerImp::destroyPlugInDescriptor(SessionItem *pItem)
{
   // Nothing to do. There is no need to kill the descriptor if it fails to deserialize.
}

void SessionManagerImp::destroyLayer(SessionItem *pItem)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Needs to be implemented (tjohnson)")
}

void SessionManagerImp::destroyWindow(SessionItem *pItem)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Needs to be implemented (tjohnson)")
}

void SessionManagerImp::destroyView(SessionItem *pItem)
{
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Needs to be implemented (tjohnson)")
}

void SessionManagerImp::destroySessionInfo(SessionItem *pItem)
{
   SessionInfoItem* pInfo = dynamic_cast<SessionInfoItem*>(pItem);
   delete pInfo;
}

namespace
{
string getBaseType(const string &type)
{
   int pos = type.find_first_of("/");
   if (pos == string::npos)
   {
      return type;
   }
   else
   {
      return type.substr(0, pos);
   }
}
}

void SessionManagerImp::createSessionItems(vector<IndexFileItem> &items, Progress *pProgress)
{
   map<string, CreatorProc> creators;
   creators["PlotWidget"] = &SessionManagerImp::createPlotWidget;
   creators["PlotSet"] = &SessionManagerImp::createPlotSet;
   creators["View"] = &SessionManagerImp::createView;
   creators["Window"] = &SessionManagerImp::createWindow;
   creators["Layer"] = &SessionManagerImp::createLayer;
   creators["PlugInDescriptor"] = &SessionManagerImp::createPlugInDescriptor;
   creators["Module"] = &SessionManagerImp::createModule;
   creators["PlugInInstance"] = &SessionManagerImp::createPlugInInstance;
   creators["DataElement"] = &SessionManagerImp::createDataElement;
   creators["ApplicationWindow"] = &SessionManagerImp::createAppWindow;
   creators["Animation"] = &SessionManagerImp::createAnimation;
   creators["AnimationController"] = &SessionManagerImp::createAnimationController;
   creators["SessionInfo"] = &SessionManagerImp::createSessionInfo;
   vector<IndexFileItem>::iterator pItem;
   int count = items.size();
   int i = 0;
   for (pItem = items.begin(), i = 0; pItem != items.end(); ++pItem, ++i)
   {
      string baseType = getBaseType(pItem->mType);
      map<string, CreatorProc>::iterator pNode = creators.find(baseType);
      VERIFYNRV(pNode != creators.end());
      if (pProgress)
      {
         pProgress->updateProgress("Creating session items...", 100*i/count, NORMAL);
      }
      CreatorProc pProc = pNode->second;
      LOG_IF(pProc == NULL, continue);
      pItem->mpItem = (this->*pProc)(pItem->mType, pItem->mId, pItem->mName);
      if (pItem->mpItem == NULL && pProgress != NULL)
      {
         string message = "Error creating:\n  " + pItem->mType + "\n";
         message += "Named:\n  " +pItem->mName;
         pProgress->updateProgress(message, 100 * i / count, WARNING);
      }
   }
}

void SessionManagerImp::deleteObsoleteFiles(const string &dir, const vector<IndexFileItem> &itemsToKeep) const
{
   QDir dirList(QString::fromStdString(dir));
   QStringList files = dirList.entryList(QDir::NoFilter, QDir::Name);
   vector<string>::iterator pFilename;

   vector<string> itemFilenames;
   vector<string> dirFilenames;
   transform(itemsToKeep.begin(), itemsToKeep.end(), back_inserter(itemFilenames), ItemFilename());
   transform(files.begin(), files.end(), back_inserter(dirFilenames), boost::bind(&QString::toStdString, _1));
   sort(itemFilenames.begin(), itemFilenames.end());

   vector<string> obsoleteFiles;
   set_difference(dirFilenames.begin(), dirFilenames.end(), itemFilenames.begin(), itemFilenames.end(),
      back_inserter(obsoleteFiles));

   for (pFilename = obsoleteFiles.begin(); pFilename != obsoleteFiles.end(); ++pFilename)
   {
      dirList.remove(QString::fromStdString(*pFilename));
   }
}

void SessionManagerImp::destroyFailedSessionItem(const string &type, SessionItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   map<string, SessionItem*>::iterator iter = mItems.find(pItem->getId());
   if (iter != mItems.end())
   {
      mItems.erase(iter);
   }

   if (type.empty() == true)
   {
      return;
   }

   map<string, DestroyerProc> destroyers;
   destroyers["PlotWidget"] = &SessionManagerImp::destroyPlotWidget;
   destroyers["PlotSet"] = &SessionManagerImp::destroyPlotSet;
   destroyers["View"] = &SessionManagerImp::destroyView;
   destroyers["Window"] = &SessionManagerImp::destroyWindow;
   destroyers["Layer"] = &SessionManagerImp::destroyLayer;
   destroyers["PlugInDescriptor"] = &SessionManagerImp::destroyPlugInDescriptor;
   destroyers["Module"] = &SessionManagerImp::destroyModule;
   destroyers["PlugInInstance"] = &SessionManagerImp::destroyPlugInInstance;
   destroyers["DataElement"] = &SessionManagerImp::destroyDataElement;
   destroyers["ApplicationWindow"] = &SessionManagerImp::destroyAppWindow;
   destroyers["Animation"] = &SessionManagerImp::destroyAnimation;
   destroyers["AnimationController"] = &SessionManagerImp::destroyAnimationController;
   destroyers["SessionInfo"] = &SessionManagerImp::destroySessionInfo;
   string baseType = getBaseType(type);
   map<string, DestroyerProc>::iterator pNode = destroyers.find(baseType);
   VERIFYNRV(pNode != destroyers.end());
   DestroyerProc pProc = pNode->second;
   VERIFYNRV(pProc != NULL);
   (this->*pProc)(pItem);
}

vector<SessionItem*> SessionManagerImp::getAllSessionItems()
{
   vector<IndexFileItem> fileItems;
   vector<SessionItem*> sessionItems;

   fileItems = getAllIndexFileItems();
   transform(fileItems.begin(), fileItems.end(), back_inserter(sessionItems), 
      boost::bind(&IndexFileItem::getSessionItem, _1));

   return sessionItems;
}

vector<SessionManagerImp::IndexFileItem> SessionManagerImp::getAllIndexFileItems()
{
   vector<IndexFileItem> items;

   getSessionItemsAnimation(items);
   getSessionItemsDataElement(items);
   getSessionItemsPlugIn(items);
   getSessionItemsWindow(items);

   return items;
}

string SessionManagerImp::getName() const
{
   return mName;
}

void SessionManagerImp::lockSessionSave()
{
   mSaveLockCount++;
}

void SessionManagerImp::unlockSessionSave()
{
   if (mSaveLockCount > 0)
   {
      mSaveLockCount--;
   }
}

bool SessionManagerImp::isSessionSaving() const
{
   return mIsSaveLoad;
}

bool SessionManagerImp::isSessionLoading() const
{
   return mIsSaveLoad;
}

bool SessionManagerImp::isSessionSaveLocked() const
{
   return mSaveLockCount > 0;
}

string SessionManagerImp::getPathForItem(const string &dir, const SessionManagerImp::IndexFileItem &item) const
{
   return dir + "/" + item.mId + ".sessionItem";
}

template<class M>
struct IfiInitializer
{
   IfiInitializer(const string &baseType) : mBaseType(baseType) {}
   template<class T> SessionManagerImp::IndexFileItem operator() (T *pT)
   {
      SessionManagerImp::IndexFileItem ifi(pT);
      ifi.mType = mBaseType;
      if (pT != NULL)
      {
         ifi.mType += "/" + pT->getName();
      }
      return ifi;
   }
   const string& mBaseType;
};

template<>
struct IfiInitializer<SessionItem*>
{
   IfiInitializer( const string &baseType) : mBaseType(baseType) {}
   template<class T> SessionManagerImp::IndexFileItem operator() (T *pT)
   {
      SessionManagerImp::IndexFileItem ifi(pT);
      ifi.mType = mBaseType;
      return ifi;
   }
   const string& mBaseType;
};

template<>
struct IfiInitializer<TypeAwareObject*>
{
   IfiInitializer( const string &baseType) : mBaseType(baseType) {}
   template<class T> SessionManagerImp::IndexFileItem operator() (T *pT)
   {
      SessionManagerImp::IndexFileItem ifi(pT);
      ifi.mType = mBaseType;
      if (pT != NULL)
      {
         ifi.mType += "/" + pT->getObjectType();
      }
      return ifi;
   }
   const string& mBaseType;
};

template<class M>
IfiInitializer<M> makeIfiI(const string &baseType = string())
{
   return IfiInitializer<M>(baseType);
}

void SessionManagerImp::getSessionItemsAnimation(vector<IndexFileItem> &items) const
{
   const vector<AnimationController*>& animationControllers = Service<AnimationServices>()->getAnimationControllers();
   transform(animationControllers.begin(), animationControllers.end(), back_inserter(items), 
      makeIfiI<SessionItem*>("AnimationController"));

   vector<AnimationController*>::const_iterator ppController;
   for (ppController = animationControllers.begin(); ppController != animationControllers.end(); ++ppController)
   {
      if (*ppController != NULL)
      {
         const vector<Animation*>& currentAnimations = (*ppController)->getAnimations();
         transform(currentAnimations.begin(), currentAnimations.end(), back_inserter(items),
            makeIfiI<SessionItem*>("Animation"));
      }
   }
}

SessionItem *SessionManagerImp::getSessionItem(const string &id)
{
   if (mIsSaveLoad)
   {
      map<string, SessionItem*>::iterator pNode = mItems.find(id);
      if (pNode != mItems.end())
      {
         return pNode->second;
      }
   }
   else
   {
      vector<IndexFileItem> fileItems = getAllIndexFileItems();
      for (vector<IndexFileItem>::const_iterator it = fileItems.begin(); it != fileItems.end(); ++it)
      {
         if (it->mId == id)
         {
            return it->getSessionItem();
         }
      }
   }
   return NULL;
}

void SessionManagerImp::getSessionItemsDataElement(vector<IndexFileItem> &items) const
{
   vector<DataElement*> dataElements = Service<ModelServices>()->getElements("");
   transform(dataElements.begin(), dataElements.end(), back_inserter(items), 
      makeIfiI<TypeAwareObject*>("DataElement"));
}

void SessionManagerImp::getSessionItemsPlugIn(vector<IndexFileItem> &items) const
{
   PlugInManagerServicesImp* pPims = PlugInManagerServicesImp::instance();
   vector<PlugIn*> plugIns = pPims->getPlugInInstances();
   transform(plugIns.begin(), plugIns.end(), back_inserter(items), 
      makeIfiI<PlugIn*>("PlugInInstance"));

   const vector<ModuleDescriptor*>& modules = pPims->getModuleDescriptors();
   transform(modules.begin(), modules.end(), back_inserter(items), 
      makeIfiI<ModuleDescriptor*>("Module"));

   vector<ModuleDescriptor*>::const_iterator ppModule;
   for (ppModule = modules.begin(); ppModule != modules.end(); ++ppModule)
   {
      ModuleDescriptor* pModule = *ppModule;
      LOG_IF(pModule == NULL, continue);
      vector<PlugInDescriptorImp*> plugInDescriptors = pModule->getPlugInSet();
      transform(plugInDescriptors.begin(), plugInDescriptors.end(), back_inserter(items), 
         makeIfiI<PlugInDescriptor*>("PlugInDescriptor"));
   }
}

void SessionManagerImp::getSessionItemsView(vector<IndexFileItem> &items, View &view) const
{
   SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(&view);
   if (pSpatialDataView != NULL)
   {
      vector<Layer*> layers;
      LayerList* pLayerList = pSpatialDataView->getLayerList();
      pLayerList->getLayers(layers);
      transform(layers.begin(), layers.end(), back_inserter(items), makeIfiI<TypeAwareObject*>("Layer"));
      Layer* pMeasurementLayer = pSpatialDataView->getMeasurementsLayer();
      VERIFYNRV(pMeasurementLayer != NULL);
      DataElement* pElm = pMeasurementLayer->getDataElement();
      if (pElm != NULL)
      {
         items.push_back(makeIfiI<TypeAwareObject*>("DataElement")(pElm));
      }
      items.push_back(makeIfiI<TypeAwareObject*>("Layer")(pMeasurementLayer));
   }
   else
   {
      ProductView* pProductView = dynamic_cast<ProductView*>(&view);
      if (pProductView != NULL)
      {
         AnnotationLayer* pLayoutLayer = pProductView->getLayoutLayer();
         list<GraphicObject*> objects;
         pLayoutLayer->getObjects(objects);
         list<GraphicObject*>::iterator ppObject;
         for (ppObject = objects.begin(); ppObject != objects.end(); ++ppObject)
         {
            GraphicObject* pObject = *ppObject;
            LOG_IF(pObject == NULL, continue);
            ViewObjectImp* pViewObject = dynamic_cast<ViewObjectImp*>(pObject);
            if (pViewObject != NULL)
            {
               View* pLayerView = pViewObject->getView();
               if (pLayerView != NULL)
               {
                  getSessionItemsView(items, *pLayerView);
                  items.push_back(makeIfiI<TypeAwareObject*>("View")(pLayerView));
               }
            }
         }
      }
   }
}

void SessionManagerImp::getSessionItemsWindow(vector<IndexFileItem> &items) const
{
   vector<Window*> windows;
   Service<DesktopServices> pDesktop;

   // add each type of window
   int start(static_cast<int>(WORKSPACE_WINDOW));
   int stop(static_cast<int>(TOOLBAR));
   for (int w = start; w <= stop; ++w)
   {
      WindowTypeEnum wt = static_cast<WindowTypeEnum>(w);
      pDesktop->getWindows(wt, windows);
      string strType("Window/");
      strType += StringUtilities::toXmlString(wt);
      transform(windows.begin(), windows.end(), back_inserter(items), 
         makeIfiI<TypeAwareObject*>(strType));
      windows.clear();
   }

   // get items associated with the windows
   pDesktop->getWindows(windows);
   vector<Window*>::iterator ppWindow;
   for (ppWindow = windows.begin(); ppWindow != windows.end(); ++ppWindow)
   {
      WorkspaceWindow* pWorkspaceWindow = dynamic_cast<WorkspaceWindow*>(*ppWindow);
      if (pWorkspaceWindow != NULL)
      {
         View* pView = pWorkspaceWindow->getView();
         if (pView != NULL)
         {
            getSessionItemsView(items, *pView);
            items.push_back(makeIfiI<TypeAwareObject*>("View")(pView));
         }
      }
      else
      {
         PlotWindow* pPlotWindow = dynamic_cast<PlotWindow*>(*ppWindow);
         if (pPlotWindow != NULL)
         {
            vector<PlotSet*> plotSets;
            pPlotWindow->getPlotSets(plotSets);
            vector<PlotSet*>::iterator ppPlotSet;
            for (ppPlotSet = plotSets.begin(); ppPlotSet != plotSets.end(); ++ppPlotSet)
            {
               PlotSet* pPlotSet = *ppPlotSet;
               LOG_IF(pPlotSet == NULL, continue);
               vector<PlotWidget*> widgets;
               pPlotSet->getPlots(widgets);

               vector<PlotWidget*>::iterator ppPlotWidget;
               for (ppPlotWidget = widgets.begin(); ppPlotWidget != widgets.end(); ++ppPlotWidget)
               {
                  PlotWidget* pPlotWidget = *ppPlotWidget;
                  LOG_IF(pPlotWidget == NULL, continue);
                  PlotView* pPlotView = pPlotWidget->getPlot();
                  LOG_IF(pPlotView == NULL, continue);
                  // need to add annotation layer and element to item list
                  AnnotationLayer* pAnno = pPlotView->getAnnotationLayer();
                  if (pAnno != NULL)
                  {
                     DataElement* pElm = pAnno->getDataElement();
                     if (pElm != NULL)
                     {
                        items.push_back(makeIfiI<TypeAwareObject*>("DataElement")(pElm));
                     }
                     items.push_back(makeIfiI<TypeAwareObject*>("Layer")(pAnno));
                  }
                  // need to know type of plot view when creating plot widget on restore
                  items.push_back(makeIfiI<TypeAwareObject*>("View")(pPlotView));
                  items.push_back(makeIfiI<TypeAwareObject*>("PlotWidget/"+pPlotView->getObjectType())(pPlotWidget));
               }
            }
            // add after plot widgets and views so these items are restored before the plotset
            transform(plotSets.begin(), plotSets.end(), back_inserter(items), makeIfiI<SessionItem*>("PlotSet"));
         }
      }
   }

   items.push_back(makeIfiI<SessionItem*>("ApplicationWindow")
      (static_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget())));
}

SessionManagerImp::IndexFileItem::IndexFileItem(SessionItem* pItem) :
   mpItem(pItem)
{
   if (pItem != NULL)
   {
      mId = pItem->getId();
      mName = pItem->getName();
   }
}

SessionManagerImp::IndexFileItem::IndexFileItem() :
   mpItem(NULL)
{
}

SessionItem* SessionManagerImp::IndexFileItem::getSessionItem() const
{
   return mpItem;
}

void SessionManagerImp::newSession()
{
   SessionSaveLock lock;
   close();
   mName = SessionItemImp::generateUniqueId();
   MessageLogMgrImp::instance()->getLog(mName); //force the session log to be created.
   ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget());
   VERIFYNRV(pAppWindow != NULL);
   pAppWindow->registerPlugIns();
   pAppWindow->updateWizardCommands();

   PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
   if (pManager != NULL)
   {
      pManager->executeStartupPlugIns(NULL);
   }

   notify(SIGNAL_NAME(SessionManagerImp, NameChanged), mName);
}

bool SessionManagerImp::open(const string &filename, Progress *pProgress)
{
   SessionSaveLock lock;
   string name = mName;
   try
   {
      if (filename.empty())
      {
         return false;
      }
      mRestoreSessionPath = filename + "Dir";

      QDir sessionDir(QString::fromStdString(mRestoreSessionPath));
      if (sessionDir.exists() == false)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("The '" + mRestoreSessionPath +
               "' session directory does not exist.  The session will not be loaded.", 0, ERRORS);
         }

         return false;
      }

      if (pProgress)
      {
         pProgress->updateProgress("Closing current session...", 1, NORMAL);
      }
      mIsSaveLoad = true;
      vector<IndexFileItem> items = readIndexFile(filename);
      name = mName;
      close();
      mName = name;
      MessageLogMgrImp::instance()->getLog(mName); //force the session log to be created.
      if (pProgress)
      {
         pProgress->updateProgress("Restoring base services...", 1, NORMAL);
      }
      notify(SIGNAL_NAME(SessionManager, AboutToRestore));
      ApplicationWindow* pAppWindow = static_cast<ApplicationWindow*>(Service<DesktopServices>()->getMainWidget());
      if (NN(pAppWindow))
      {
         pAppWindow->registerPlugIns();
         pAppWindow->updateWizardCommands();
         IndexFileItem ifi(ModelServicesImp::instance());
         restoreSessionItem(ifi);
         createSessionItems(items, pProgress);
         populateItemMap(items);
         restoreSessionItems(items, pProgress);
         mIsSaveLoad = false;
         notify(SIGNAL_NAME(SessionManagerImp, NameChanged), mName);
         if (pProgress)
         {
            pProgress->updateProgress("Done.", 100, NORMAL);
         }
      }
      else
      {
         mIsSaveLoad = false;
         return false;
      }
   }
   catch (SessionManagerImp::Failure &e)
   {
      mIsSaveLoad = false;
      if (pProgress)
      {
         pProgress->updateProgress("Failed.", 100, ERRORS);
      }
      mName = name;
      Service<DesktopServices>()->showMessageBox("Session Load Failure", "The session load failed: \n" + e.mMessage, 
         "Ok");
      return false;
   }
   notify(SIGNAL_NAME(SessionManager, SessionRestored));
   return true;
}

void SessionManagerImp::populateItemMap(const vector<IndexFileItem> &items)
{
   vector<IndexFileItem>::const_iterator pItem;
   for (pItem = items.begin(); pItem != items.end(); ++pItem)
   {
      if (pItem->mId.empty() == false && pItem->mpItem != NULL)
      {
         mItems[pItem->mId] = pItem->mpItem;
      }
   }
}

vector<SessionManagerImp::IndexFileItem> SessionManagerImp::readIndexFile(const string &filename)
{
   vector<IndexFileItem> items;
   Service<MessageLogMgr> pLogMgr;
   MessageLog* pLog = pLogMgr->getLog();
   XmlReader xml(pLog, false);

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = NULL;
   pDocument = xml.parse(filename);
   if (pDocument != NULL)
   {
      DOMElement* pRootElement = NULL;
      pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         if (!XMLString::equals(pRootElement->getNodeName(), X("Session")))
         {
            string msg = "Attempting to load a session from a non-session file.";
            throw Failure(msg);
         }
         string savedVersion = A(pRootElement->getAttribute(X("version")));
         if (savedVersion != APP_VERSION_NUMBER)
         {
            string msg = "Can only load sessions saved with " + string(APP_NAME) + " " + APP_VERSION_NUMBER;
            msg += "\nThis session was saved with " + string(APP_NAME) + " " + savedVersion;
            throw Failure(msg); 
         }
         mName = A(pRootElement->getAttribute(X("id")));
         if (mName.empty())
         {
            mName = SessionItemImp::generateUniqueId();
         }
         FOR_EACH_DOMNODE (pRootElement, pChild)
         {
            if (XMLString::equals(pChild->getNodeName(), X("session_item")))
            {
               IndexFileItem item;
               DOMElement* pElement = static_cast<DOMElement*>(pChild);
               item.mId = A(pElement->getAttribute(X("id")));
               item.mType = A(pElement->getAttribute(X("type")));
               item.mName = A(pElement->getAttribute(X("name")));
               FOR_EACH_DOMNODE (pChild, pG1child)
               {
                  if (XMLString::equals(pG1child->getNodeName(), X("block")))
                  {
                     DOMElement* pBlockElement = dynamic_cast<DOMElement*>(pG1child);
                     if (pElement != NULL)
                     {
                        int size = atoi(A(pBlockElement->getAttribute(X("size"))));
                        item.mBlockSizes.push_back(size);
                     }
                  }
               }
               if (item.mType.empty() == false && item.mId.empty() == false)
               {
                  items.push_back(item);
               }
            }
         }
      }
   }
   if (items.empty())
   {
      throw Failure("Unable to read any session items from the session");
   }
   return items;
}

void SessionManagerImp::restoreSessionItems(vector<IndexFileItem> &items, Progress *pProgress)
{
   int count = items.size();
   int i = 0;
   vector<IndexFileItem>::iterator pItem;
   for (pItem = items.begin(), i = 0; pItem != items.end(); ++pItem, ++i)
   {
      if (pProgress)
      {
         pProgress->updateProgress("Restoring session items...", 100*i/count, NORMAL);
      }
      if (pItem->mpItem)
      {
         if (!restoreSessionItem(*pItem))
         {
            if (pProgress != NULL)
            {
               string message = "Error restoring:\n  " + pItem->mType + "\n";
               message += "Named:\n  " +pItem->mName;
               pProgress->updateProgress(message, 100 * i / count, WARNING);
            }
         }
      }
   }
   // remove session info item
   IndexFileItem infoItem = items.back();
   SessionItem* pSessionItem = infoItem.mpItem;   
   destroyFailedSessionItem(infoItem.mType, pSessionItem);
}

bool SessionManagerImp::restoreSessionItem(IndexFileItem &item)
{
   SessionItem* pSessionItem = item.mpItem;
   VERIFY_MSG(pSessionItem!=NULL, 
      string("SessionItem '" + item.mType + "' not successfully created").c_str());
   ItemFilename filename;
   SessionItemDeserializerImp deserializer(mRestoreSessionPath + "/" + filename(item), item.mBlockSizes);
   if (pSessionItem->deserialize(deserializer) == false)
   {
      destroyFailedSessionItem(item.mType, pSessionItem);
      item.mpItem = NULL;
      return false;
   }
   return true;
}

pair<SessionManager::SerializationStatus, vector<pair<SessionItem*, string> > >
SessionManagerImp::serialize(const string& filename, Progress* pProgress)
{
   if (isSessionSaveLocked())
   {
      return make_pair(SerializationStatus(LOCKED), vector<pair<SessionItem*, string> >());
   }
   SerializationStatus status = SUCCESS;
   vector<IndexFileItem> items = getAllIndexFileItems();

   //add session info that needs to be restored as last item
   SessionInfoItem* pInfo = new SessionInfoItem("{98043E17-338D-4273-8C7F-C6A5E9FF58A5}", "SessionInfo");
   items.push_back(makeIfiI<SessionItem*>("SessionInfo")(pInfo));

   vector<pair<SessionItem*, string> > failedItems;
   vector<IndexFileItem> successItems;
   QFileInfo fileInfo(filename.c_str());
   string sessionDirPath = fileInfo.absoluteDir().absolutePath().toStdString() + "/" +
      fileInfo.completeBaseName().toStdString() + ".sessionDir";
   QDir sessionDir(QString::fromStdString(sessionDirPath));
   if (sessionDir.exists() == false)
   {
      if (!sessionDir.mkpath(QString::fromStdString(sessionDirPath)))
      {
         status = FAILURE;
      }
   }
   if (status != FAILURE)
   {
      mIsSaveLoad = true;
      deleteObsoleteFiles(sessionDirPath, items);

      SessionItemSerializerImp sis(getPathForItem(sessionDirPath, ModelServicesImp::instance()));
      ModelServicesImp::instance()->serialize(sis);

      int count = items.size();
      int i = 0;
      for (vector<IndexFileItem>::iterator ppItem = items.begin();
         ppItem != items.end();
         ++ppItem, ++i)
      {
         SessionItem* pItem = ppItem->getSessionItem();
         LOG_IF(pItem == NULL, continue);

         // check for session items that should not be saved
         SessionItemExt1* pItemExt1 = dynamic_cast<SessionItemExt1*>(pItem);
         if (pItemExt1 != NULL && pItemExt1->isValidSessionSaveItem() == false)
         {
            // don't serialize this item
            continue;
         }

         string filePath = getPathForItem(sessionDirPath, *ppItem);
         if (pProgress)
         {
            pProgress->updateProgress("Saving session items...", 100*i/count, NORMAL);
         }
         SessionItemSerializerImp itemSerializer(filePath);
         bool itemSuccess = pItem->serialize(itemSerializer);
         if (!itemSuccess)
         {
            status = PARTIAL_SUCCESS;
            failedItems.push_back(make_pair(pItem, ppItem->mType));
            if (pProgress)
            {
               string message = "Error saving:\n  " + ppItem->mType + "\n";
               message += "Named:\n  " + ppItem->mName;
               pProgress->updateProgress(message, 100 * i / count, WARNING);
            }
         }
         else
         {
            ppItem->mBlockSizes = itemSerializer.getBlockSizes();
            successItems.push_back(*ppItem);
         }
      }
      mIsSaveLoad = false;

      if (successItems.size() == 0 || writeIndexFile(filename, successItems) == false)
      {
         failedItems.clear();
         status = FAILURE;
      }
      if (pProgress)
      {
         pProgress->updateProgress("Done.", 100, status == FAILURE ? ERRORS : NORMAL);
      }
   }

   return make_pair(status, failedItems);
}

bool SessionManagerImp::writeIndexFile(const string &filename, const vector<IndexFileItem> &items)
{
   FILE* pFile = fopen(filename.c_str(), "w");
   if (pFile == NULL)
   {
      return false;
   }

   XMLWriter xml("Session");
   xml.addAttr("version", APP_VERSION_NUMBER);
   xml.addAttr("id", mName);
   vector<IndexFileItem>::const_iterator ppItem;
   for (ppItem = items.begin(); ppItem != items.end(); ++ppItem)
   {
      SessionItem* pItem = ppItem->getSessionItem();
      if (pItem != NULL)
      {
         XML_ADD_POINT (xml, session_item)
         {
            xml.addAttr("id", ppItem->mId);
            xml.addAttr("type", ppItem->mType);
            xml.addAttr("name", ppItem->mName);

            for (vector<int64_t>::const_iterator pSize = ppItem->mBlockSizes.begin();
               pSize != ppItem->mBlockSizes.end();
               ++pSize)
            {
               XML_ADD_POINT (xml, block)
               {
                  xml.addAttr("size", *pSize);
               }
            }
         }
      }
   }

   xml.writeToFile(pFile);
   fclose(pFile);

   return true;
}
