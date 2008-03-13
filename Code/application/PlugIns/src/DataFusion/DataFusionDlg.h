/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSION_DLG_H
#define FUSION_DLG_H

#include "DesktopServices.h"
#include "Vector.h"

#include <QtCore/QMap>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>

class DatasetPage;
class FusionAlgorithmInputsPage;
class FusionLayersSelectPage;
class FusionPage;
class GcpPoint;
class PlugIn;
class ProgressTracker;
class RasterLayer;
class RasterElement;
class SpatialDataView;
class SpatialDataWindow;
class TiePointPage;

namespace boost
{
   class any;
}

class DataFusionDlg : public QDialog
{
   Q_OBJECT

 public:
   DataFusionDlg(PlugIn* pPlugIn, ProgressTracker &progressTracker, QWidget* pParent = NULL);
   ~DataFusionDlg();

   RasterElement* getPrimaryChip() const;
   RasterElement* getSecondaryChip() const;

   void windowDeleted(Subject &subject, const std::string &signal, const boost::any &v);

protected:
   void showPage(QWidget* pPage);
   virtual void closeEvent(QCloseEvent* pEvent);

protected slots:
   void accept();
   void reject();
   void updateModifiedFlags();
   void enableWizardButtons();
   void fuse();
   void back();
   void next();

private:
   // copies the color map from one RasterLayer to another
   void copyColorMap(RasterLayer& destination, const RasterLayer& source) const;

   /**
    * Given a filename and a string, produces a new filename.
    *
    * Example: a filename of A.sio and an append string of _flicker produces A_flicker.sio
    *
    * @param  filename
    *         The filename (base name or full path and name) of a file
    * @param  pAppend
    *         The string to append before the extension of filename
    * @return Returns a new string based on the file name and the append string.
    */
   std::string getNewFilename(const std::string& filename, const char* pAppend);

   // creates am image chip given the BitMask, etc.
   RasterElement* createChip(RasterElement* pRaster,
                                  unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2,
                                  const std::string& appendage, int alphaValue, int zoomFactor);

   // creates the RasterElement S', performing the warping and creating the secondary image chip
   RasterElement* createSecondaryImageChip(RasterElement* pSecondaryRaster,
                                         const Vector<double>& P, const Vector<double>& Q,
                                         unsigned int newCols, unsigned int newRows, int llX, int llY, int zoomFactor,
                                         bool inMemory);

   /**
    * Copies layers into new view, with a scale factor
    *
    * Throws AssertException if a bug occurs.
    *
    * @param  view
    *         The destination view to place layers. The layers are in line with the primary image chip.
    * @param  scaleFactor
    *         The zoom factor that was applied to the primary image chip when the fused scenes were created.
    *
    * @return TRUE if the operation succeeded, FALSE if the user aborted the process if prompted.
    */
   bool copyLayersToView(SpatialDataView& view, int scaleFactor) const;

   Service<DesktopServices> mpDesktop;

   // Pages
   QLabel* mpDescriptionLabel;
   QStackedWidget* mpStack;
   DatasetPage* mpDatasetPage;
   FusionAlgorithmInputsPage* mpInputsPage;
   FusionLayersSelectPage* mpLayersPage;
   TiePointPage* mpTiePointPage;
   QPushButton* mpBackButton;
   QPushButton* mpNextButton;
   QPushButton* mpFinishButton;

   SpatialDataWindow* mpFlicker;
   WorkspaceWindow* mpSbs;

   PlugIn* mpPlugIn;
   ProgressTracker& mProgressTracker;

   // modified map
   QMap<FusionPage*, bool> mModified;
   
   // These mouse modes are the mouse modes after the views are selected on the first page.
   // They are restored when the dialog goes away. I am storing strings instead of pointers
   // since they are safer.
   std::string mPrimaryMouseMode, mSecondaryMouseMode;

   static const std::string PRIMARY_CHIP_WIDGET_NAME, SECONDARY_CHIP_WIDGET_NAME, ANNOTATION_LAYER_NAME;
};

#endif
