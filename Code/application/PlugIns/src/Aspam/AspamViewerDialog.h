/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMVIEWERDIALOG_H
#define ASPAMVIEWERDIALOG_H

#include "Aspam.h"
#include "ModelServices.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QFrame>

class Any;
class AspamViewer;
class ParagraphAgui;
class ParagraphBgui;
class ParagraphDgui;
class ParagraphFgui;
class ParagraphGgui;
class ParagraphHgui;
class ParagraphJgui;
class PlotView;
class Point;
class QButtonGroup;
class QComboBox;
class QDateTimeEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTabWidget;
class QTextEdit;
class QStackedWidget;

/**
 *  GUI dialog box which displays Aspam data.
 */
class AspamViewerDialog : public QDialog
{
   Q_OBJECT

   AspamViewer *mpViewer;
   QComboBox *mpAspamList;
   QPushButton *mpUnloadButton;
   QTabWidget *mpDataWidget;
   QLabel *mpErrorWidget;
   QStackedWidget *mpDataStack;

   QMap<QString,Any*> mAspams;

   ParagraphAgui *mpParagraphAgui;
   ParagraphBgui *mpParagraphBgui;
   ParagraphDgui *mpParagraphDgui;
   ParagraphFgui *mpParagraphFgui;
   ParagraphGgui *mpParagraphGgui;
   ParagraphHgui *mpParagraphHgui;
   ParagraphJgui *mpParagraphJgui;

   Service<ModelServices> mpModelServices;

protected:
   QMap<QString, unsigned int> getPlotSelectionList(QPushButton*& pButton) const;
   void getPlotSelectionData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary) const;
   virtual void closeEvent(QCloseEvent *pEvent);

public:
   AspamViewerDialog(AspamViewer *pViewer, QWidget *pParent=NULL);

public slots:
   void populateAspamList();
   void setDataSet(const QString &dataset);
   void plot();
   void checkPlotStatus();
   void unloadAspam();
};

/**
 *  Interface for the widget representing specific paragraph data.
 */
class ParagraphGui  : public QFrame
{
   Q_OBJECT

public:
   /**
    *  Accessor for the QTableWidget which displays tabular data for the paragraph.
    *  Data in this table can be plotted.
    *
    *  @return Pointer to the table widget containing the data.
    */
   virtual QTableWidget *getTable() const { return NULL; }

   /**
    *  Accessor for the "Plot" button if this paragraph has data which can be plotted.
    *
    *  @return Pointer to the plot button widget.
    */
   virtual QPushButton *getButton() const { return NULL; }

   /**
    *  Accessor for the plot data for a paragraph.
    *
    *  @param data
    *         Plot points for the requested data. This is an output argument.
    *
    *  @param pView
    *         The PlotView which will contain the requested data.
    *
    *  @param primary
    *         Index for the data column which will be on the primary axis.
    *
    *  @param secondary
    *         Index for the data column which will be on the secondary axis.
    *
    *  @todo Use the Qt 4 QAbstractItemModel for accessing data.
    */
   virtual void getPlotData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary) {}

protected:
   ParagraphGui(QWidget *pParent = NULL) : QFrame(pParent) {}
};

/**
 *  GUI for Paragraph A data.
 *  This paragraph has no tabular data.
 */
class ParagraphAgui : public ParagraphGui
{
   Q_OBJECT

   QLineEdit *mpSiteId;

public:
   ParagraphAgui(QWidget *pParent = NULL);

public slots:
   void setData(const Aspam::ParagraphA &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph B data.
 *  This paragraph has no tabular data.
 */
class ParagraphBgui : public ParagraphGui
{
   Q_OBJECT

   QDateTimeEdit *mpDateTime;

public:
   ParagraphBgui(QWidget *pParent = NULL);

public slots:
   void setData(const Aspam::ParagraphB &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph D data.
 *  This paragraph has tabular data.
 */
class ParagraphDgui : public ParagraphGui
{
   Q_OBJECT

   QLineEdit    *mpSurfaceVisibility;
   QSpinBox     *mpTotalCoverage;
   QLineEdit    *mpWindDirection;
   QLineEdit    *mpWindSpeed;
   QLineEdit    *mpGustSpeed;
   QTextEdit    *mpRemark;
   QTableWidget *mpClouds;
   QPushButton  *mpPlotButton;

   const Aspam::ParagraphD *mpRawData;

public:
   ParagraphDgui(QWidget *pParent = NULL);
   QTableWidget *getTable() const { return mpClouds; }
   QPushButton *getButton() const { return mpPlotButton; }
   virtual void getPlotData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary);

signals:
   void plot();
   void plotDataUpdated();

public slots:
   void setData(const Aspam::ParagraphD &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph F data.
 *  This paragraph has tabular data.
 */
class ParagraphFgui : public ParagraphGui
{
   Q_OBJECT

   QLineEdit    *mpLevel;
   QTableWidget *mpAnalytic;
   QPushButton  *mpPlotButton;

   const Aspam::ParagraphF *mpRawData;

public:
   ParagraphFgui(QWidget *pParent = NULL);
   QTableWidget *getTable() const { return mpAnalytic; }
   QPushButton *getButton() const { return mpPlotButton; }
   virtual void getPlotData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary);

signals:
   void plot();
   void plotDataUpdated();

public slots:
   void setData(const Aspam::ParagraphF &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph G data.
 *  This paragraph has no tabular data.
 */
class ParagraphGgui : public ParagraphGui
{
   Q_OBJECT

   QTextEdit *mpRemarks;

public:
   ParagraphGgui(QWidget *pParent = NULL);

public slots:
   void setData(const Aspam::ParagraphG &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph H data.
 *  This paragraph has tabular data.
 */
class ParagraphHgui : public ParagraphGui
{
   Q_OBJECT

   QLineEdit    *mpLevels;
   QComboBox    *mpSeasonalDependence;
   QComboBox    *mpStratosphericAerosol;
   QComboBox    *mpOzoneProfile;
   QLineEdit    *mpBlpqi;
   QComboBox    *mpPrimaryBlap;
   QLineEdit    *mpAirParcelType;
   QLineEdit    *mpSurfaceVisibility;
   QComboBox    *mpAlternateBlap;
   QLineEdit    *mpAlternateAirParcelType;
   QLineEdit    *mpAlternateSurfaceVisibility;
   QTableWidget *mpAerosol;
   QPushButton  *mpPlotButton;

   const Aspam::ParagraphH *mpRawData;

public:
   ParagraphHgui(QWidget *pParent = NULL);
   QTableWidget *getTable() const { return mpAerosol; }
   QPushButton *getButton() const { return mpPlotButton; }
   virtual void getPlotData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary);

signals:
   void plot();
   void plotDataUpdated();

public slots:
   void setData(const Aspam::ParagraphH &data, QTabWidget *pTabWidget);
};

/**
 *  GUI for Paragraph J data.
 *  This paragraph has tabular data.
 */
class ParagraphJgui : public ParagraphGui
{
   Q_OBJECT

   QLineEdit    *mpMaxTemperature;
   QLineEdit    *mpMinTemperature;
   QLineEdit    *mpSnowDepth;
   QTableWidget *mpSurfaceWeather;
   QPushButton  *mpPlotButton;

   const Aspam::ParagraphJ *mpRawData;

public:
   ParagraphJgui(QWidget *pParent = NULL);
   QTableWidget *getTable() const { return mpSurfaceWeather; }
   QPushButton *getButton() const { return mpPlotButton; }
   virtual void getPlotData(std::vector<Point*> &data, PlotView *pView,
      unsigned int primary, unsigned int secondary);

signals:
   void plot();
   void plotDataUpdated();

public slots:
   void setData(const Aspam::ParagraphJ &data, QTabWidget *pTabWidget);
};

#endif