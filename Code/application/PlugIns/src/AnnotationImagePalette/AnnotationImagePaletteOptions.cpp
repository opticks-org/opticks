/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationImagePalette.h"
#include "AnnotationImagePaletteOptions.h"
#include "AnnotationImagePaletteWidget.h"
#include "AppVerify.h"
#include "CustomTreeWidget.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "Filename.h"
#include "LabeledSection.h"
#include "ObjectResource.h"
#include "PlugInFactory.h"

#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <vector>

PLUGINFACTORY_OPTIONWRAPPER(AnnotationImagePaletteOptions);

AnnotationImagePaletteOptions::AnnotationImagePaletteOptions()
{
   QWidget* pDirContainer = new QWidget(this);
   mpDirWidget = new CustomTreeWidget(pDirContainer);
   mpDirWidget->setColumnCount(1);
   mpDirWidget->header()->hide();
   QAction* pAddAction = new QAction("Add", this);
   QAction* pRemoveAction = new QAction("Remove", this);
   pAddAction->setShortcut(QKeySequence(Qt::Key_Insert));
   pRemoveAction->setShortcuts(QKeySequence::Delete);
   mpDirWidget->addAction(pAddAction);
   mpDirWidget->addAction(pRemoveAction);
   mpDirWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
   mpDirWidget->setRootIsDecorated(false);
   mpDirWidget->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);
   VERIFYNR(connect(pAddAction, SIGNAL(triggered()), this, SLOT(addEntry())));
   VERIFYNR(connect(pRemoveAction, SIGNAL(triggered()), this, SLOT(removeEntry())));

   QPushButton* pAddButton = new QPushButton("Add", pDirContainer);
   QPushButton* pRemoveButton = new QPushButton("Remove", pDirContainer);
   VERIFYNR(connect(pAddButton, SIGNAL(clicked()), this, SLOT(addEntry())));
   VERIFYNR(connect(pRemoveButton, SIGNAL(clicked()), this, SLOT(removeEntry())));

   QVBoxLayout* pDirLayout = new QVBoxLayout(pDirContainer);
   pDirLayout->setMargin(0);
   pDirLayout->setSpacing(5);
   pDirLayout->addWidget(mpDirWidget, 10);
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pDirLayout->addLayout(pButtonLayout);
   pButtonLayout->addStretch(10);
   pButtonLayout->addWidget(pAddButton);
   pButtonLayout->addWidget(pRemoveButton);

   LabeledSection* pPalettePathsSection = new LabeledSection(pDirContainer, "Palette Paths", this);
   setPalettePaths(AnnotationImagePaletteOptions::getSettingPaletteDirs());

   QWidget* pInitialWidthWidget = new QWidget(this);
   QLabel* pInitialWidthLabel = new QLabel("Initial Image Width", pInitialWidthWidget);
   mpInitialWidth = new QSpinBox(pInitialWidthWidget);
   mpInitialWidth->setToolTip("If enabled, the image will be resized to this width when added to the view. The same aspect ratio will be maintained.");
   mpInitialWidth->setMinimum(1);
   mpFullSize = new QCheckBox("Full Size", pInitialWidthWidget);
   mpFullSize->setToolTip("If checked, the image will not be resized when added to the view.");
   VERIFYNR(connect(mpFullSize, SIGNAL(toggled(bool)), mpInitialWidth, SLOT(setDisabled(bool))));
   QHBoxLayout* pInitialWidthLayout = new QHBoxLayout(pInitialWidthWidget);
   pInitialWidthLayout->addWidget(pInitialWidthLabel);
   pInitialWidthLayout->addWidget(mpInitialWidth);
   pInitialWidthLayout->addWidget(mpFullSize);
   pInitialWidthLayout->addStretch();

   LabeledSection* pInitialWidthSection = new LabeledSection(pInitialWidthWidget, "Initial Image Width", this);
   setInitialWidth(AnnotationImagePaletteOptions::getSettingInitialWidth());

   // Initialization
   addSection(pPalettePathsSection, 1000);
   addSection(pInitialWidthSection, 100);
   addStretch(1);
}

AnnotationImagePaletteOptions::~AnnotationImagePaletteOptions()
{
}

void AnnotationImagePaletteOptions::applyChanges()
{
   AnnotationImagePaletteOptions::setSettingPaletteDirs(getPalettePaths());
   AnnotationImagePaletteOptions::setSettingInitialWidth(mpFullSize->isChecked() ? 0 : mpInitialWidth->value());

   // get the image palette and force a refresh
   DockWindow* pWindow = static_cast<DockWindow*>(
      Service<DesktopServices>()->getWindow(AnnotationImagePalette::getWindowName(), DOCK_WINDOW));
   AnnotationImagePaletteWidget* pWidget = (pWindow == NULL)
      ? NULL : qobject_cast<AnnotationImagePaletteWidget*>(pWindow->getWidget());
   if (pWidget != NULL)
   {
      pWidget->refresh();
   }
}

void AnnotationImagePaletteOptions::setPalettePaths(const std::vector<Filename*>& paths)
{
   mpDirWidget->clear();
   for (std::vector<Filename*>::const_iterator path = paths.begin(); path != paths.end(); ++path)
   {
      if (*path == NULL)
      {
         continue;
      }
      QTreeWidgetItem* pItem = new QTreeWidgetItem(
         QStringList() << QString::fromStdString((*path)->getFullPathAndName()));
      mpDirWidget->addTopLevelItem(pItem);
      mpDirWidget->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_DIR_EDIT);
   }
}

std::vector<Filename*> AnnotationImagePaletteOptions::getPalettePaths() const
{
   std::vector<Filename*> paths;
   for (int idx = 0; idx < mpDirWidget->topLevelItemCount(); idx++)
   {
      QTreeWidgetItem* pItem = mpDirWidget->topLevelItem(idx);
      if (pItem == NULL)
      {
         continue;
      }
      QString path = pItem->text(0);
      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(path.toStdString());
      if (path.isEmpty() || !pFilename->isDirectory())
      {
         continue;
      }
      paths.push_back(pFilename.release());
   }
   return paths;
}

void AnnotationImagePaletteOptions::setInitialWidth(int width)
{
   if (width <= 0)
   {
      mpInitialWidth->setValue(32);
      mpFullSize->setChecked(true);
      mpInitialWidth->setEnabled(false);
   }
   else
   {
      mpInitialWidth->setValue(width);
      mpFullSize->setChecked(false);
      mpInitialWidth->setEnabled(true);
   }
}

void AnnotationImagePaletteOptions::addEntry()
{
   QTreeWidgetItem* pItem = new QTreeWidgetItem();
   mpDirWidget->addTopLevelItem(pItem);
   mpDirWidget->setCellWidgetType(pItem, 0, CustomTreeWidget::BROWSE_DIR_EDIT);
}

void AnnotationImagePaletteOptions::removeEntry()
{
   QList<QTreeWidgetItem*> selected = mpDirWidget->selectedItems();
   foreach(QTreeWidgetItem* pItem, selected)
   {
      mpDirWidget->invisibleRootItem()->removeChild(pItem);
      delete pItem;
   }
}
