/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICIMAGEWIDGET_H
#define GRAPHICIMAGEWIDGET_H

#include <QtCore/QMap>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

#include "EnumWrapper.h"

class FileBrowser;

class GraphicImageWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicImageWidget(QWidget* pParent = NULL);
   ~GraphicImageWidget();

   enum ImageSourceTypeEnum { FILE, WIDGET, RAW_DATA };
   typedef EnumWrapper<ImageSourceTypeEnum> ImageSourceType;

   void setImageSource(ImageSourceType imageSource);
   ImageSourceType getImageSource() const;

   QString getImageFile() const;
   QWidget* getImageWidget() const;
   int getOpacity() const;

   void setOpacityVisible(bool bVisible);
   bool isOpacityVisible() const;

public slots:
   void setImageFile(const QString& filename);
   void setImageWidget(QWidget* pWidget);
   void setOpacity(int opacity);

signals:
   void imageFileChanged(const QString& filename);
   void imageWidgetChanged(QWidget* pWidget);
   void opacityChanged(int opacity);

protected slots:
   void notifyImageWidgetChange();

private:
   GraphicImageWidget(const GraphicImageWidget& rhs);
   GraphicImageWidget& operator=(const GraphicImageWidget& rhs);
   QLabel* mpFileLabel;
   FileBrowser* mpFileBrowser;
   QLabel* mpWidgetLabel;
   QComboBox* mpWidgetCombo;
   QLabel* mpOpacityLabel;
   QSpinBox* mpOpacitySpin;

   QMap<QString, QWidget*> mWidgets;
};

#endif
