/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebIo.h"

#include "Aeb.h"
#include "ApplicationServices.h"
#include "Rdf.h"
#include "StringUtilities.h"

#include <numeric>

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QQueue>
#include <QtCore/QUrl>
#include <memory>
#include <unzip.h>

std::string AebIo::sAeblPrefix = "urn:2008:03:aebl-syntax-ns#";
std::string AebIo::sAeblTopSubject = "urn:aebl:install-manifest";
std::string AebIo::sOpticksPrefix = "urn:2008:03:opticks-aebl-extension-ns#";

const unsigned int COPY_BUF_SIZE = 1 * 1024 * 1024; //1 MB

AebEntry::AebEntry(QString path, size_t size) : 
   mFilePath(path), mFileSize(size)
{
}

AebEntry::~AebEntry()
{
}

QString AebEntry::getFilePath() const
{
   return mFilePath;
}

size_t AebEntry::getFileSize() const
{
   return mFileSize;
}

class ZipAebEntry : public AebEntry
{
public:
   ZipAebEntry(QString path, size_t size, unz_file_pos pos) :
      AebEntry(path, size), mPos(pos)
   {}

protected:
   unz_file_pos mPos;
   friend class AebIo;
};

class FsAebEntry : public AebEntry
{
public:
   FsAebEntry(QString path, size_t size, QString absolutePath) :
      AebEntry(path, size), mAbsoluteFilePath(absolutePath)
   {}

protected:
   QString mAbsoluteFilePath;
   friend class AebIo;
};


AebIo::AebIo(Aeb& obj) : mObj(obj), mLoadedContentPaths(false)
{
   mBuf.resize(COPY_BUF_SIZE);
   mBuf2.resize(COPY_BUF_SIZE);
}

AebIo::~AebIo()
{
   foreach (const AebEntry* pEntry, mContentPaths)
   {
      delete pEntry;
   }
}

#define V(p__, msg__) if(!(p__)) { errMsg = msg__; return false; }

#define GET_SINGLE_OBJ(target__, topsubj__, id__, msg__) \
   objs = pRdf->getAllObjects(topsubj__, id__); \
   V(objs.size() == 1, msg__); \
   target__ = objs.front();

#define GET_SINGLE_OPT_OBJ(target__, topsubj__, id__, msg__) \
   objs = pRdf->getAllObjects(topsubj__, id__); \
   if (!objs.empty()) \
   { \
      V(objs.size() == 1, msg__); \
      target__ = objs.front(); \
   }

bool AebIo::fromFile(const std::string& fname, std::string& errMsg)
{
   mObj.mAebFile = QFileInfo(QString::fromStdString(fname));
   if (!mObj.mAebFile.exists())
   {
      errMsg = "AEB does not exist.";
      return false;
   }
   bool wasValidPath = true;
   QByteArray bytes = getBytesFromAeb("aeb:///install.rdf", wasValidPath);
   //KIP - Add better error reporting based upon wasValidPath for all calls to getBytesFromAeb?
   if (!wasValidPath || bytes.isEmpty())
   {
      errMsg = "AEB descriptor file is missing or empty.";
      return false;
   }
   std::string manifest = bytes.data();

   RdfParser parser;
   std::auto_ptr<Rdf> pRdf(parser.parseString(QUrl::fromLocalFile(QString::fromStdString(fname)).toString().toStdString(), manifest));
   V(pRdf.get(), "AEB descriptor file is invalid.");

   std::vector<std::string> objs;

   // id
   GET_SINGLE_OBJ(mObj.mId, sAeblTopSubject, sAeblPrefix+"id", "Invalid AEB ID.");
   V(mObj.mId.isValid(), "Invalid AEB ID.");

   // version
   GET_SINGLE_OBJ(mObj.mVersion, sAeblTopSubject, sAeblPrefix+"version", "Invalid AEB version.");

   // type
   std::string type;
   GET_SINGLE_OBJ(type, sAeblTopSubject, sAeblPrefix+"type", "Invalid AEB type.");
   if (type == "32")
   {
      errMsg = "Multi-AEB is not yet supported.";
      return false;
   }
   V(type == "2", "Unsupported AEB type.");

   // name
   GET_SINGLE_OBJ(mObj.mName, sAeblTopSubject, sAeblPrefix+"name", "Invalid AEB name.");
   V(!mObj.mName.empty(), "Invalid AEB name.");
   
   // targetApplication
   std::vector<RdfObject*> targetApps = pRdf->getSubject(sAeblTopSubject)->getObjectsForPredicate(sAeblPrefix+"targetApplication");
   for (std::vector<RdfObject*>::const_iterator targetApp = targetApps.begin(); targetApp != targetApps.end(); ++targetApp)
   {
      AebId id;
      GET_SINGLE_OBJ(id, (*targetApp)->getObjectName(), sAeblPrefix+"id", "Invalid target application ID.");
      AebVersion min;
      GET_SINGLE_OBJ(min, (*targetApp)->getObjectName(), sAeblPrefix+"minVersion", "Invalid target application minVersion.");
      AebVersion max;
      GET_SINGLE_OBJ(max, (*targetApp)->getObjectName(), sAeblPrefix+"maxVersion", "Invalid target application maxVersion.");
      AebRequirement app(id, min, max);
      V(app.isValid(), "Invalid AEB target application.");
      mObj.mTargetApplication.push_back(app);
   }

   // description
   GET_SINGLE_OPT_OBJ(mObj.mDescription, sAeblTopSubject, sAeblPrefix+"description", "Invalid AEB description.");

   // creator
   GET_SINGLE_OPT_OBJ(mObj.mCreator, sAeblTopSubject, sAeblPrefix+"creator", "Invalid AEB creator.");

   // developer
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"developer");
   mObj.mDevelopers = objs;

   // translator
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"translator");
   mObj.mTranslators = objs;

   // contributor
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"contributor");
   mObj.mContributors = objs;

   // homepageURL
   GET_SINGLE_OPT_OBJ(mObj.mHomepageURL, sAeblTopSubject, sAeblPrefix+"homepageURL", "Invalid AEB homepage URL.");

   // iconURL
   GET_SINGLE_OPT_OBJ(mObj.mIconURL, sAeblTopSubject, sAeblPrefix+"iconURL", "Invalid AEB icon.");

   // licenseURL
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"licenseURL");
   mObj.mLicenseURLs = objs;

   // hidden
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"hidden");
   if (!objs.empty())
   {
      V(objs.size() == 1, "Invalid AEB hidden specification.");
      mObj.mHidden = StringUtilities::fromXmlString<bool>(objs.front());
   }
   else
   {
      mObj.mHidden = false;
   }

   // targetPlatform
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"targetPlatform");
   for (std::vector<std::string>::const_iterator obj = objs.begin(); obj != objs.end(); ++obj)
   {
      AebPlatform platform(*obj);
      if (platform.isValid())
      {
         mObj.mPlatforms.push_back(platform);
      }
   }
   if (!objs.empty() && mObj.mPlatforms.empty())
   {
      errMsg = "The current platform is not supported by this extension.";
      return false;
   }

   // requires
   std::vector<RdfObject*> requires = pRdf->getSubject(sAeblTopSubject)->getObjectsForPredicate(sAeblPrefix+"requires");
   for (std::vector<RdfObject*>::const_iterator require = requires.begin(); require != requires.end(); ++require)
   {
      // parse the requires
      AebId id;
      GET_SINGLE_OBJ(id, (*require)->getObjectName(), sAeblPrefix+"id", "Invalid requirement ID.");
      AebVersion min;
      GET_SINGLE_OBJ(min, (*require)->getObjectName(), sAeblPrefix+"minVersion", "Invalid requirement minVersion.");
      AebVersion max;
      GET_SINGLE_OBJ(max, (*require)->getObjectName(), sAeblPrefix+"maxVersion", "Invalid requirement maxVersion.");
      AebRequirement req(id, min, max);
      V(req.isValid(), "Invalid requirement.");

      // check for a targetApplication, otherwise use an invalid one to indicate "all"
      AebRequirement targetApp;
      objs = pRdf->getAllObjects((*require)->getObjectName(), sAeblPrefix+"targetApplication");
      if (!objs.empty())
      {
         V(objs.size() == 1, "Invalid requirement target applicaiton specification.");
         std::string root = objs.front();
         AebId id;
         GET_SINGLE_OBJ(id, root, sAeblPrefix+"id", "Invalid requirement target application id.");
         AebVersion min;
         GET_SINGLE_OBJ(min, root, sAeblPrefix+"minVersion", "Invalid requirement target application minVersion.");
         AebVersion max;
         GET_SINGLE_OBJ(max, root, sAeblPrefix+"maxVersion", "Invalid requirement target application maxVersion.");
         targetApp = AebRequirement(id, min, max);
         V(targetApp.isValid(), "Invalid requirement target application.");
      }
      mObj.mRequires.insert(std::make_pair(targetApp, req));
   }

   // incompatible
   std::vector<RdfObject*> incompatibles = pRdf->getSubject(sAeblTopSubject)->getObjectsForPredicate(sAeblPrefix+"incompatible");
   for (std::vector<RdfObject*>::const_iterator incompatible = incompatibles.begin(); incompatible != incompatibles.end(); ++incompatible)
   {
      // parse the incompatibles
      AebId id;
      GET_SINGLE_OBJ(id, (*incompatible)->getObjectName(), sAeblPrefix+"id", "Invalid incompatible ID.");
      AebVersion min;
      GET_SINGLE_OBJ(min, (*incompatible)->getObjectName(), sAeblPrefix+"minVersion", "Invalid incompatible minVersion.");
      AebVersion max;
      GET_SINGLE_OBJ(max, (*incompatible)->getObjectName(), sAeblPrefix+"maxVersion", "Invalid incompatible maxVersion.");
      AebRequirement incomp(id, min, max);
      V(incomp.isValid(), "Invalid incompatible.");

      // check for a targetApplication, otherwise use an invalid one to indicate "all"
      AebRequirement targetApp;
      objs = pRdf->getAllObjects((*incompatible)->getObjectName(), sAeblPrefix+"targetApplication");
      if (!objs.empty())
      {
         V(objs.size() == 1, "Invalid incompatible target application.");
         std::string root = objs.front();
         AebId id;
         GET_SINGLE_OBJ(id, root, sAeblPrefix+"id", "Invalid incompatible target application ID.");
         AebVersion min;
         GET_SINGLE_OBJ(min, root, sAeblPrefix+"minVersion", "Invalid incompatible target application minVersion.");
         AebVersion max;
         GET_SINGLE_OBJ(max, root, sAeblPrefix+"maxVersion", "Invalid incompatible target application maxVersion.");
         targetApp = AebRequirement(id, min, max);
         V(targetApp.isValid(), "Invalid incompatible target application.");
      }
      mObj.mIncompatibles.insert(std::make_pair(targetApp, incomp));
   }

   // updateKey
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"updateKey");
   if (!objs.empty())
   {
      mObj.mUpdateKey = objs.front();
   }

   // updateURL
   objs = pRdf->getAllObjects(sAeblTopSubject, sAeblPrefix+"updateURL");
   if (!objs.empty())
   {
      mObj.mUpdateURL = objs.front();
      // validate the URL
      // if valid and scheme is not https, ensure updateKey is not empty
   }

   // splashScreenURL
   objs = pRdf->getAllObjects(sAeblTopSubject, sOpticksPrefix+"splashScreenURL");
   mObj.mSplashScreenURLs = objs;

   // helpEntry
   objs = pRdf->getAllObjects(sAeblTopSubject, sOpticksPrefix+"helpEntry");
   for (std::vector<std::string>::const_iterator obj = objs.begin(); obj != objs.end(); ++obj)
   {
      std::vector<std::string> names = pRdf->getAllObjects(*obj, sOpticksPrefix+"name");
      V(names.size() == 1, "Invalid help entry name.");
      std::vector<std::string> urls = pRdf->getAllObjects(*obj, sOpticksPrefix+"url");
      V(urls.size() == 1, "Invalid help entry URL.");
      mObj.mHelpEntries[names.front()] = urls.front();
   }

   // Load icon and licenses
   if (Service<ApplicationServices>()->isInteractive() && !mObj.mIconURL.empty())
   {
      bool wasValidPath = true;
      QByteArray bytes = getBytesFromAeb(QString::fromStdString(mObj.mIconURL), wasValidPath);
      if (wasValidPath && !bytes.isEmpty())
      {
         QImage img;
         if (img.loadFromData(bytes))
         {
            mObj.mpIcon = new QPixmap(QPixmap::fromImage(img).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
         }
         else
         {
            errMsg = "Unable to load an icon [" + mObj.mIconURL + "]. It may be in an unsupported format.";
            return false;
         }
      }
   }
   if (!mObj.mLicenseURLs.empty())
   {
      for (std::vector<std::string>::const_iterator url = mObj.mLicenseURLs.begin(); url != mObj.mLicenseURLs.end(); ++url)
      {
         bool wasValidPath = true;
         QByteArray bytes = getBytesFromAeb(QString::fromStdString(*url), wasValidPath);
         if (wasValidPath && !bytes.isEmpty())
         {
            mObj.mLicenses.push_back(QString(bytes));
         }
         else
         {
            errMsg = "Invalid or empty license file. [" + *url + "]";
            return false;
         }
      }
   }

   return true;
}

bool AebIo::toFile(const std::string& fname)
{
   int tmpCounter = 1;
   Rdf rdf;

   rdf.addTriple(sAeblTopSubject, sAeblPrefix+"id", mObj.mId);
   rdf.addTriple(sAeblTopSubject, sAeblPrefix+"version", mObj.mVersion.toString());
   rdf.addTriple(sAeblTopSubject, sAeblPrefix+"type", "2");
   rdf.addTriple(sAeblTopSubject, sAeblPrefix+"name", mObj.mName);
   for (std::vector<AebRequirement>::const_iterator tapp = mObj.mTargetApplication.begin(); tapp != mObj.mTargetApplication.end(); ++tapp)
   {
      std::string targetAppSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"targetApplication", targetAppSubject);
      rdf.addTriple(targetAppSubject, sAeblPrefix+"id", tapp->getId());
      rdf.addTriple(targetAppSubject, sAeblPrefix+"minVersion", tapp->getMin().toString());
      rdf.addTriple(targetAppSubject, sAeblPrefix+"maxVersion", tapp->getMax().toString());
   }
   if (!mObj.mDescription.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"description", mObj.mDescription);
   }
   if (!mObj.mCreator.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"creator", mObj.mCreator);
   }
   for (std::vector<std::string>::const_iterator dev = mObj.mDevelopers.begin(); dev != mObj.mDevelopers.end(); ++dev)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"developer", *dev);
   }
   for (std::vector<std::string>::const_iterator trn = mObj.mTranslators.begin(); trn != mObj.mTranslators.end(); ++trn)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"translator", *trn);
   }
   for (std::vector<std::string>::const_iterator contrib = mObj.mContributors.begin(); contrib != mObj.mContributors.end(); ++contrib)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"contributor", *contrib);
   }
   if (!mObj.mHomepageURL.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"homepageURL", mObj.mHomepageURL);
   }
   if (!mObj.mIconURL.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"iconURL", mObj.mIconURL);
   }
   for (std::vector<std::string>::const_iterator url = mObj.mLicenseURLs.begin(); url != mObj.mLicenseURLs.end(); ++url)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"licenseURL", *url);
   }
   if (mObj.mHidden)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"hidden", "true");
   }
   for (std::vector<AebPlatform>::const_iterator platform = mObj.mPlatforms.begin(); platform != mObj.mPlatforms.end(); ++platform)
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"targetPlatform", platform->toString());
   }
   for (std::multimap<AebRequirement, AebRequirement>::const_iterator require = mObj.mRequires.begin();
            require != mObj.mRequires.end(); ++require)
   {
      std::string requireSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"requires", requireSubject);
      rdf.addTriple(requireSubject, sAeblPrefix+"id", require->second.getId());
      rdf.addTriple(requireSubject, sAeblPrefix+"minVersion", require->second.getMin().toString());
      rdf.addTriple(requireSubject, sAeblPrefix+"maxVersion", require->second.getMax().toString());
      if (require->first.isValid())
      {
         std::string targetAppSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
         rdf.addTriple(requireSubject, sAeblPrefix+"targetApplication", targetAppSubject);
         rdf.addTriple(targetAppSubject, sAeblPrefix+"id", require->first.getId());
         rdf.addTriple(targetAppSubject, sAeblPrefix+"minVersion", require->first.getMin().toString());
         rdf.addTriple(targetAppSubject, sAeblPrefix+"maxVersion", require->first.getMax().toString());
      }
   }
   for (std::multimap<AebRequirement, AebRequirement>::const_iterator incompat = mObj.mIncompatibles.begin();
            incompat != mObj.mIncompatibles.end(); ++incompat)
   {
      std::string incompatSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"incompatible", incompatSubject);
      rdf.addTriple(incompatSubject, sAeblPrefix+"id", incompat->second.getId());
      rdf.addTriple(incompatSubject, sAeblPrefix+"minVersion", incompat->second.getMin().toString());
      rdf.addTriple(incompatSubject, sAeblPrefix+"maxVersion", incompat->second.getMax().toString());
      if (incompat->first.isValid())
      {
         std::string targetAppSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
         rdf.addTriple(incompatSubject, sAeblPrefix+"targetApplication", targetAppSubject);
         rdf.addTriple(targetAppSubject, sAeblPrefix+"id", incompat->first.getId());
         rdf.addTriple(targetAppSubject, sAeblPrefix+"minVersion", incompat->first.getMin().toString());
         rdf.addTriple(targetAppSubject, sAeblPrefix+"maxVersion", incompat->first.getMax().toString());
      }
   }
   if (!mObj.mUpdateKey.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"updateKey", mObj.mUpdateKey);
   }
   if (!mObj.mUpdateURL.empty())
   {
      rdf.addTriple(sAeblTopSubject, sAeblPrefix+"updateURL", mObj.mUpdateURL);
   }
   for (std::vector<std::string>::const_iterator splash = mObj.mSplashScreenURLs.begin(); splash != mObj.mSplashScreenURLs.end(); ++splash)
   {
      rdf.addTriple(sAeblTopSubject, sOpticksPrefix+"splashScreenURL", *splash);
   }
   for (std::map<std::string,std::string>::const_iterator help = mObj.mHelpEntries.begin(); help != mObj.mHelpEntries.end(); ++help)
   {
      std::string helpSubject = "_:" + StringUtilities::toDisplayString(tmpCounter++);
      rdf.addTriple(sAeblTopSubject, sOpticksPrefix+"helpEntry", helpSubject);
      rdf.addTriple(helpSubject, sOpticksPrefix+"name", help->first);
      rdf.addTriple(helpSubject, sOpticksPrefix+"url", help->second);
   }
   RdfSerializer serializer(rdf);
   return serializer.serializeToFile(fname);
}

class ZipCurrentFileResource
{
public:
   ZipCurrentFileResource(unzFile fileHandle) :
      mFileHandle(fileHandle)
   {
      mRetValue = unzOpenCurrentFile(mFileHandle);
   }
   ~ZipCurrentFileResource()
   {
      unzCloseCurrentFile(mFileHandle);
   }
   int getReturnValue()
   {
      return mRetValue;
   }

private:
   unzFile mFileHandle;
   int mRetValue;
};

class QOpenFileResource
{
public:
   QOpenFileResource(QFile& file, QIODevice::OpenMode flags) :
      mFile(file)
   {
      mRetValue = mFile.open(flags);
   }
   ~QOpenFileResource()
   {
      mFile.close();
   }
   bool getReturnValue()
   {
      return mRetValue;
   }

private:
   QFile& mFile;
   bool mRetValue;
};

bool AebIo::openZipFileIfNeeded() const
{
   if (mZipFile.get() == NULL)
   {
      ZipFileResource zipFile(mObj.mAebFile.absoluteFilePath().toStdString());
      if (zipFile.get() == NULL)
      {
         return false;
      }
      mZipFile = zipFile;
      return true;
   }
   return true;
}

const QList<const AebEntry*>& AebIo::getContentPaths() const
{
   if (mLoadedContentPaths)
   {
      return mContentPaths;
   }

   QString platformStr = QString("platform/%1").arg(QString::fromStdString(AebPlatform().toString()));
   if (mObj.mAebFile.fileName() == "install.rdf")
   {
      QDir root(mObj.mAebFile.absoluteDir());
      QQueue<QDir> process;
      process.enqueue(QDir(root.absoluteFilePath("content")));
      process.enqueue(QDir(root.absoluteFilePath(platformStr)));
      while (!process.isEmpty())
      {
         QDir current = process.dequeue();
         QFileInfoList items = current.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
         foreach (QFileInfo item, items)
         {
            if (item.isDir())
            {
               process.enqueue(QDir(item.absoluteFilePath()));
            }
            else if (item.isFile())
            {
               QString absoluteFilePath = item.absoluteFilePath();
               QString path = root.relativeFilePath(absoluteFilePath);
               mContentPaths.push_back(new FsAebEntry(path, item.size(), absoluteFilePath));
            }
         }
      }
   }
   else
   {
      openZipFileIfNeeded();
      if (!mZipFile.get())
      {
         return mContentPaths;
      }
      unz_file_info info;
      QByteArray filename;
      for (int rval = unzGoToFirstFile(*mZipFile); rval == UNZ_OK; rval = unzGoToNextFile(*mZipFile))
      {
         if (unzGetCurrentFileInfo(*mZipFile, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
         {
            continue;
         }
         filename.resize(info.size_filename);
         if (unzGetCurrentFileInfo(*mZipFile, NULL, filename.data(), filename.size(), NULL, 0, NULL, 0) == UNZ_OK)
         {
            QString path(filename);
            if ((path.startsWith("content") || path.startsWith(platformStr)) && !path.endsWith("/"))
            {
               unz_file_pos zipPosition;
               if (unzGetFilePos(*mZipFile, &zipPosition) != UNZ_OK)
               {
                  continue;
               }
               mContentPaths.push_back(new ZipAebEntry(path, info.uncompressed_size, zipPosition));
            }
         }
      }
   }

   return mContentPaths;
}

const AebEntry* AebIo::getEntry(const QUrl& aebUrl) const
{
   if (!aebUrl.isValid() || aebUrl.scheme() != "aeb")
   {
      return NULL;
   }
   if (mObj.mAebFile.fileName() == "install.rdf")
   {
      QDir rootDir(mObj.mAebFile.absoluteDir());
      QString rootDirStr = rootDir.absolutePath();
      // remove the initial / since an aeb: URL is really relative to the top level directory
      QString upath = aebUrl.path().remove(0, 1);
      QString absoluteFilePath = rootDir.absoluteFilePath(upath);
      QFileInfo file(absoluteFilePath);
      if (!file.exists())
      {
         return NULL;
      }
      return new FsAebEntry(upath, file.size(), absoluteFilePath);
   }
   else
   {
      openZipFileIfNeeded();
      if (!mZipFile.get())
      {
         return NULL;
      }
      // remove the initial / since an aeb: URL is really relative to the zip file
      QString upath = aebUrl.path().remove(0, 1);
      if (unzLocateFile(*mZipFile, upath.toAscii(), 0) != UNZ_OK)
      {
         return NULL;
      }
      unz_file_info file_info;
      if (unzGetCurrentFileInfo(*mZipFile, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
      {
         return NULL;
      }
      unz_file_pos zipPosition;
      if (unzGetFilePos(*mZipFile, &zipPosition) != UNZ_OK)
      {
         return NULL;
      }
      return new ZipAebEntry(upath, file_info.uncompressed_size, zipPosition);
   }
   return NULL;
}

bool AebIo::installFileFromAeb(const AebEntry* pSource, const std::string& destination) const
{
   if (mObj.mAebFile.fileName() == "install.rdf")
   {
      const FsAebEntry* pFsSource = dynamic_cast<const FsAebEntry*>(pSource);
      if (pFsSource == NULL)
      {
         return false;
      }
      QFile file(pFsSource->mAbsoluteFilePath);
      return file.copy(QString::fromStdString(destination));
   }
   else
   {
      const ZipAebEntry* pZipSource = dynamic_cast<const ZipAebEntry*>(pSource);
      if (pZipSource == NULL)
      {
         return false;
      }
      openZipFileIfNeeded();
      if (!*mZipFile)
      {
         return false;
      }
      if (unzGoToFilePos(*mZipFile, const_cast<unz_file_pos*>(&(pZipSource->mPos))) != UNZ_OK)
      {
         return false;
      }
      ZipCurrentFileResource currentFile(*mZipFile);
      if (currentFile.getReturnValue() != UNZ_OK)
      {
         return false;
      }

      QFile outFile(QString::fromStdString(destination));
      QOpenFileResource outFileRes(outFile, QIODevice::WriteOnly);
      if (!outFileRes.getReturnValue())
      {
         return false;
      }
      int bytesRead = 0;
      do
      {
         bytesRead = unzReadCurrentFile(*mZipFile, mBuf.data(), COPY_BUF_SIZE);
         if (bytesRead < 0)
         {
            return false;
         }
         int bytesWritten = outFile.write(mBuf.data(), bytesRead);
         if (bytesRead != bytesWritten)
         {
            return false;
         }
      }
      while (bytesRead == COPY_BUF_SIZE);
      return true;
   }
}

bool AebIo::compareFileInAeb(const AebEntry* pSource, const std::string& destination) const
{
   if (mObj.mAebFile.fileName() == "install.rdf")
   {
      const FsAebEntry* pFsSource = dynamic_cast<const FsAebEntry*>(pSource);
      if (pFsSource == NULL)
      {
         return false;
      }
      QFile destFile(QString::fromStdString(destination));
      if (pFsSource->getFileSize() != destFile.size())
      {
         return false;
      }
      QFile sourceFile(pFsSource->mAbsoluteFilePath);
      QOpenFileResource sourceFileRes(sourceFile, QIODevice::ReadOnly);
      if (!sourceFileRes.getReturnValue())
      {
         return false;
      }
      QOpenFileResource destFileRes(destFile, QIODevice::ReadOnly);
      if (!destFileRes.getReturnValue())
      {
         return false;
      }
      memset(mBuf.data(), 0, COPY_BUF_SIZE); //make buffers equal, so we can be sure
      memset(mBuf2.data(), 0, COPY_BUF_SIZE); //the != compare later works properly
      int sourceBytesRead = 0;
      int destBytesRead = 0;
      do
      {
         sourceBytesRead = sourceFile.read(mBuf.data(), COPY_BUF_SIZE);
         destBytesRead = destFile.read(mBuf2.data(), COPY_BUF_SIZE);
         if (sourceBytesRead != destBytesRead || sourceBytesRead < 0)
         {
            return false;
         }
         if (mBuf != mBuf2)
         {
            return false;
         }
      }
      while (sourceBytesRead == COPY_BUF_SIZE);
      return true;
   }
   else
   {
      const ZipAebEntry* pZipSource = dynamic_cast<const ZipAebEntry*>(pSource);
      if (pZipSource == NULL)
      {
         return false;
      }
      openZipFileIfNeeded();
      if (!mZipFile.get())
      {
         return false;
      }
      if (unzGoToFilePos(*mZipFile, const_cast<unz_file_pos*>(&(pZipSource->mPos))) != UNZ_OK)
      {
         return false;
      }
      QFile destFile(QString::fromStdString(destination));
      if (destFile.size() != pZipSource->getFileSize())
      {
         return false;
      }
      ZipCurrentFileResource currentFile(*mZipFile);
      if (currentFile.getReturnValue() != UNZ_OK)
      {
         return false;
      }
      QOpenFileResource destFileRes(destFile, QIODevice::ReadOnly);
      if (!destFileRes.getReturnValue())
      {
         return false;
      }
      memset(mBuf.data(), 0, COPY_BUF_SIZE); //make buffers equal, so we can be sure
      memset(mBuf2.data(), 0, COPY_BUF_SIZE); //the != compare later works properly
      int sourceBytesRead = 0;
      int destBytesRead = 0;
      do
      {
         sourceBytesRead = unzReadCurrentFile(*mZipFile, mBuf.data(), COPY_BUF_SIZE);
         destBytesRead = destFile.read(mBuf2.data(), COPY_BUF_SIZE);
         if (sourceBytesRead != destBytesRead || sourceBytesRead < 0)
         {
            return false;
         }
         if (mBuf != mBuf2)
         {
            return false;
         }
      }
      while (sourceBytesRead == COPY_BUF_SIZE);
      return true;
   }
}

QByteArray AebIo::getBytesFromAeb(const QString& url, bool& wasValidPath) const
{
   wasValidPath = false;
   QByteArray bytes;   
   QUrl u(url);
   if (!u.isValid() || u.scheme() != "aeb")
   {
      return bytes;
   }
   if (mObj.mAebFile.fileName() == "install.rdf")
   {
      QDir aebDir(mObj.mAebFile.absoluteDir());
      QString aebDirStr = aebDir.absolutePath();
      // remove the initial / since an aeb: URL is really relative to the top level directory
      QString upath = u.path().remove(0, 1);
      QFile file(aebDir.absoluteFilePath(upath));
      if (!file.exists())
      {
         return bytes;
      }
      QOpenFileResource fileRes(file, QIODevice::ReadOnly);
      if (!fileRes.getReturnValue())
      {
         return bytes;
      }
      wasValidPath = true;
      bytes = file.readAll();
   }
   else
   {
      openZipFileIfNeeded();
      if (!mZipFile.get())
      {
         return bytes;
      }
      // remove the initial / since an aeb: URL is really relative to the zip file
      QString upath = u.path().remove(0, 1);
      if (unzLocateFile(*mZipFile, upath.toAscii(), 0) != UNZ_OK)
      {
         return bytes;
      }
      unz_file_info file_info;
      if (unzGetCurrentFileInfo(*mZipFile, &file_info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
      {
         return bytes;
      }
      ZipCurrentFileResource currentFile(*mZipFile);
      if (currentFile.getReturnValue() != UNZ_OK)
      {
         return bytes;
      }
      wasValidPath = true;
      bytes.resize(file_info.uncompressed_size);
      if (unzReadCurrentFile(*mZipFile, bytes.data(), bytes.size()) != bytes.size())
      {
         bytes.clear();
      }
   }
   return bytes;
}
