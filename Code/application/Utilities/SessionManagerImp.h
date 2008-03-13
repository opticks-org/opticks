/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSION_MANAGER_IMP
#define SESSION_MANAGER_IMP

#include "SessionItem.h"
#include "SessionManager.h"
#include "SubjectImp.h"

#include <map>

class Progress;
class SessionSaveLock;
class View;

class SessionManagerImp : public SessionManager, public SubjectImp
{
public:
   /**
    *  Emitted with boost::any<std::string> when the name of the session is changed.
    *  The string will be the new name of the session.
    */
   SIGNAL_METHOD(SessionManagerImp, NameChanged)

   /**
    *  Emitted with a null boost::any when the session has finished restoring itself.
    */
   SIGNAL_METHOD(SessionManagerImp, SessionRestored)

   /**
    *  Emitted with a null boost::any just prior to restoring a session.
    */
   SIGNAL_METHOD(SessionManagerImp, AboutToRestore)

   static SessionManagerImp* instance();
   static void destroy();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

   void close();
   std::vector<SessionItem*> getAllSessionItems();
   std::string getName() const;

   bool isSessionSaving() const { return mIsSaveLoad; }
   bool isSessionLoading() const { return mIsSaveLoad; }
   bool isSessionSaveLocked() const;

   /**
    * Closes the current session and creates a new empty one.
    *
    * This method closes all windows, destroys all animations and controllers,
    * destroys all data elements and destroys all active plug-ins, effectively
    * undoing all changes to the session since the application was opened. It
    * then creates a new empty session with a new name.
    */
   void newSession();

   /**
    * Loads a new session from disk.
    *
    * This method closes closes the current session and loads a new one from
    * disk, restoring all of the data and windows to the states they were in
    * when the session was saved.
    * 
    * @param filename
    *            The full pathname of the file the session is saved in.
    * @param pProgress
    *            If not \c NULL, progress during session restore will be reported
    *            to the supplied Progress object.
    *
    * @return   True if the session was successfully restored, or false 
    *            otherwise.
    */
   bool open(const std::string &filename, Progress *pProgress);
   std::pair<SerializationStatus,std::vector<std::pair<SessionItem*, std::string> > > serialize(const std::string &filename, Progress *pProgress);
   SessionItem *getSessionItem(const std::string &id);

   struct IndexFileItem
   {
      IndexFileItem(SessionItem* pItem);
      IndexFileItem();

      SessionItem* getSessionItem() const { return mpItem; }
      SessionItem* mpItem;
      std::string mId;
      std::string mType;
      std::string mName;
      std::vector<int64_t> mBlockSizes;
   };

protected:
   virtual ~SessionManagerImp();
   void lockSessionSave();
   void unlockSessionSave();
   friend class SessionSaveLock;

private:
   struct Failure
   {
      Failure(const std::string &msg) : mMessage(msg) {}
      std::string mMessage;
   };
   SessionManagerImp();

   typedef SessionItem *(SessionManagerImp::*CreatorProc)(const std::string &type, 
      const std::string &id, const std::string &name);
   SessionItem *createPlotWidget(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createPlotSet(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createAnimationController(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createAnimation(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createAppWindow(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createDataElement(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createPlugInInstance(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createModule(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createPlugInDescriptor(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createLayer(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createWindow(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createView(const std::string &type, const std::string &id, const std::string &name);
   SessionItem *createSessionInfo(const std::string &type, const std::string &id, const std::string &name);
   // These methods are used to destroy SessionItems that fail to deserialize
   typedef void (SessionManagerImp::*DestroyerProc)(SessionItem *pItem);
   void destroyPlotWidget(SessionItem *pItem);
   void destroyPlotSet(SessionItem *pItem);
   void destroyAnimationController(SessionItem *pItem);
   void destroyAnimation(SessionItem *pItem);
   void destroyAppWindow(SessionItem *pItem);
   void destroyDataElement(SessionItem *pItem);
   void destroyPlugInInstance(SessionItem *pItem);
   void destroyModule(SessionItem *pItem);
   void destroyPlugInDescriptor(SessionItem *pItem);
   void destroyLayer(SessionItem *pItem);
   void destroyWindow(SessionItem *pItem);
   void destroyView(SessionItem *pItem);
   void destroySessionInfo(SessionItem *pItem);

   void createSessionItems(std::vector<IndexFileItem> &items, Progress *pProgress);
   void deleteObsoleteFiles(const std::string &dir, const std::vector<IndexFileItem> &itemsToKeep) const;
   void destroyFailedSessionItem(const std::string &type, SessionItem* pItem);
   std::vector<IndexFileItem> getAllIndexFileItems();
   std::string getPathForItem(const std::string &dir, const IndexFileItem &item) const;
   void getSessionItemsAnimation(std::vector<IndexFileItem> &items) const;
   void getSessionItemsDataElement(std::vector<IndexFileItem> &items) const;
   void getSessionItemsPlugIn(std::vector<IndexFileItem> &items) const;
   void getSessionItemsView(std::vector<IndexFileItem> &items, View &view) const;
   void getSessionItemsWindow(std::vector<IndexFileItem> &items) const;
   void populateItemMap(const std::vector<IndexFileItem> &items);
   std::vector<IndexFileItem> readIndexFile(const std::string &filename);
   bool restoreSessionItem(IndexFileItem &item);
   void restoreSessionItems(std::vector<IndexFileItem> &items, Progress *pProgress);
   bool writeIndexFile(const std::string &filename, const std::vector<IndexFileItem> &items);

   static SessionManagerImp* spInstance;
   static bool mDestroyed;
   std::string mName;
   std::string mRestoreSessionPath;
   std::map<std::string,SessionItem*> mItems;
   bool mIsSaveLoad;
   unsigned int mSaveLockCount;
};

#endif
