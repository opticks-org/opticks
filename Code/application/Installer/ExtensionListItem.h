/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXTENSIONLISTITEM_H__
#define EXTENSIONLISTITEM_H__

#include <QtGui/QListWidgetItem>

class QDialogButtonBox;
class QLabel;
class QPushButton;

class ExtensionListItem : public QWidget
{
   Q_OBJECT
   Q_PROPERTY(QString name READ getName WRITE setName)
   Q_PROPERTY(QString description READ getDescription WRITE setDescription)
   Q_PROPERTY(QString version READ getVersion WRITE setVersion)
   Q_PROPERTY(QPixmap icon READ getIcon WRITE setIcon)
   Q_PROPERTY(QString id READ getId WRITE setId)
   Q_PROPERTY(QString updateInfo READ getUpdateInfo WRITE setUpdateInfo)
   Q_PROPERTY(bool uninstallable READ getUninstallable WRITE setUninstallable)

public:
   ExtensionListItem(bool editor, bool showUpdateInfo, QWidget* pParent = NULL);
   virtual ~ExtensionListItem();

   QString getName() const;
   QString getDescription() const;
   QString getVersion() const;
   QPixmap getIcon() const;
   QString getId() const;
   QString getUpdateInfo() const;
   bool getUninstallable() const;

   void setName(const QString& v);
   void setDescription(const QString& v);
   void setVersion(const QString& v);
   void setIcon(const QPixmap& v);
   void setId(const QString& v);
   void setUpdateInfo(const QString& v);

   void setUninstallable(bool v);

signals:
   void finished(QWidget* pEditor);

private slots:
   void accepted();
   void about();
   void uninstall();

private:
   QLabel* mpIcon;
   QLabel* mpName;
   QLabel* mpVersion;
   QLabel* mpDescription;
   QLabel* mpUpdateInfo;
   QDialogButtonBox* mpButtons;
   QPushButton* mpUninstallButton;
   QString mExtensionId;
};

#endif