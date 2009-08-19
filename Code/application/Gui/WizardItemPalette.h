/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDITEMPALETTE_H
#define WIZARDITEMPALETTE_H

#include <QtCore/QMimeData>
#include <QtGui/QListWidget>
#include <QtGui/QToolBox>

#include "ConfigurationSettings.h"

class WizardItemPalette : public QToolBox
{
public:
   WizardItemPalette(QWidget* pParent = NULL);
   virtual ~WizardItemPalette();

protected:
   SETTING(DisplayMode, WizardItemPalette, int, 1)

   virtual void contextMenuEvent(QContextMenuEvent* pEvent);

private:
   class WizardItemList : public QListWidget
   {
   public:
      WizardItemList(QWidget* pParent = NULL);
      virtual ~WizardItemList();

      void enableIconMode();
      void enableListMode();

      virtual QSize sizeHint() const;

   protected:
      virtual QStringList mimeTypes() const;
      virtual QMimeData* mimeData(const QList<QListWidgetItem*> items) const;
   };
};

#endif
