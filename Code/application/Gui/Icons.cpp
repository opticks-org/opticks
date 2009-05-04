/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>

#include "Icons.h"
#include "IconImages.h"

#include "ApplicationServices.h"

Icons* Icons::singleton = NULL;

Icons* Icons::instance()
{
   if (singleton == NULL)
   {
      Service<ApplicationServices> pApp;
      if (pApp->isInteractive())
      {
         singleton = new Icons;
      }
   }

   return singleton;
}

Icons::Icons()
{
   // File actions
   mNew = QPixmap(IconImages::NewIcon);
   mNew.setMask(mNew.createHeuristicMask());

   mOpen = QPixmap(IconImages::OpenIcon);
   mOpen.setMask(mOpen.createHeuristicMask());

   mClose = QPixmap(IconImages::CloseIcon);
   mClose.setMask(mClose.createHeuristicMask());

   mSave = QPixmap(IconImages::SaveIcon);
   mSave.setMask(mSave.createHeuristicMask());

   mPageSetup = QPixmap(IconImages::PageSetupIcon);
   mPageSetup.setMask(mPageSetup.createHeuristicMask());

   mPreview = QPixmap(IconImages::PrintPreviewIcon);
   mPreview.setMask(mPreview.createHeuristicMask());

   mPrint = QPixmap(IconImages::PrintIcon);
   mPrint.setMask(mPrint.createHeuristicMask());

   // Edit actions
   mUndo = QPixmap(IconImages::UndoIcon);
   mUndo.setMask(mUndo.createHeuristicMask());

   mRedo = QPixmap(IconImages::RedoIcon);
   mRedo.setMask(mRedo.createHeuristicMask());

   mCut = QPixmap(IconImages::CutIcon);
   mCut.setMask(mCut.createHeuristicMask());

   mCopy = QPixmap(IconImages::CopyIcon);
   mCopy.setMask(mCopy.createHeuristicMask());

   mPaste = QPixmap(IconImages::PasteIcon);
   mPaste.setMask(mPaste.createHeuristicMask());

   mFind = QPixmap(IconImages::FindIcon);
   mFind.setMask(mFind.createHeuristicMask());

   mFindNext = QPixmap(IconImages::FindNextIcon);
   mFindNext.setMask(mFindNext.createHeuristicMask());

   // Pan actions
   mPan = QPixmap(IconImages::PanIcon);
   mPan.setMask(mPan.createHeuristicMask());

   mZoomAndPanToPoint = QPixmap(IconImages::ZoomAndPanToPointIcon);
   mZoomAndPanToPoint.setMask(mZoomAndPanToPoint.createHeuristicMask());

   // View actions
   mRefresh = QPixmap(IconImages::RefreshIcon);
   mRefresh.setMask(mRefresh.createHeuristicMask());

   mDisplayMode = QPixmap(IconImages::DisplayModeIcon);
   mDisplayMode.setMask(mDisplayMode.createHeuristicMask());

   mGenerate = QPixmap(IconImages::GenerateIcon);
   mGenerate.setMask(mGenerate.createHeuristicMask());

   // Rotate actions
   mRotateLeft = QPixmap(IconImages::RotateLeftIcon);
   mRotateLeft.setMask(mRotateLeft.createHeuristicMask());

   mRotateRight = QPixmap(IconImages::RotateRightIcon);
   mRotateRight.setMask(mRotateRight.createHeuristicMask());

   mFlipHoriz = QPixmap(IconImages::FlipHorizontallyIcon);
   mFlipHoriz.setMask(mFlipHoriz.createHeuristicMask());

   mFlipVert = QPixmap(IconImages::FlipVerticallyIcon);
   mFlipVert.setMask(mFlipVert.createHeuristicMask());

   mRotateBy = QPixmap(IconImages::RotateByIcon);
   mRotateBy.setMask(mRotateBy.createHeuristicMask());

   mFreeRotate = QPixmap(IconImages::FreeRotateIcon);
   mFreeRotate.setMask(mFreeRotate.createHeuristicMask());

   mReset = QPixmap(IconImages::ResetOrientationIcon);
   mReset.setMask(mReset.createHeuristicMask());

   // Zoom actions
   mZoomIn = QPixmap(IconImages::ZoomInIcon);
   mZoomIn.setMask(mZoomIn.createHeuristicMask());

   mZoomOut = QPixmap(IconImages::ZoomOutIcon);
   mZoomOut.setMask(mZoomOut.createHeuristicMask());

   mZoomPointIn = QPixmap(IconImages::ZoomPointInIcon);
   mZoomPointIn.setMask(mZoomPointIn.createHeuristicMask());

   mZoomPointOut = QPixmap(IconImages::ZoomPointOutIcon);
   mZoomPointOut.setMask(mZoomPointOut.createHeuristicMask());

   mZoomRect = QPixmap(IconImages::ZoomRectIcon);
   mZoomRect.setMask(mZoomRect.createHeuristicMask());

   mZoomToFit = QPixmap(IconImages::ZoomToFitIcon);
   mZoomToFit.setMask(mZoomToFit.createHeuristicMask());

   mUndoZoom = QPixmap(IconImages::ZoomUndoIcon);
   mUndoZoom.setMask(mUndoZoom.createHeuristicMask());

   mRedoZoom = QPixmap(IconImages::ZoomRedoIcon);
   mRedoZoom.setMask(mRedoZoom.createHeuristicMask());

   // Properties actions
   mProperties = QPixmap(IconImages::PropertiesIcon);
   mProperties.setMask(mProperties.createHeuristicMask());

   mDataProp = QPixmap(IconImages::DataPropertiesIcon);
   mDataProp.setMask(mDataProp.createHeuristicMask());

   mLayerProp = QPixmap(IconImages::LayerPropertiesIcon);
   mLayerProp.setMask(mLayerProp.createHeuristicMask());

   // Tools actions
   mSessionExplorer = QPixmap(IconImages::SessionExplorerIcon);
   mSessionExplorer.setMask(mSessionExplorer.createHeuristicMask());

   mSignature = QPixmap(IconImages::SignatureWindowIcon);
   mSignature.setMask(QPixmap(IconImages::SignatureWindowMask));

   mHistogram = QPixmap(IconImages::HistogramWindowIcon);
   mHistogram.setMask(QPixmap(IconImages::HistogramWindowMask));

   mMessageLog = QPixmap(IconImages::MessageLogWindowIcon);
   mMessageLog.setMask(mMessageLog.createHeuristicMask());

   mShortcut = QPixmap(IconImages::ShortcutBarIcon);
   mShortcut.setMask(mShortcut.createHeuristicMask());

   mLayers = QPixmap(IconImages::LayersIcon);
   mLayers.setMask(mLayers.createHeuristicMask());

   mAnimation = QPixmap(IconImages::AnimationIcon);
   mAnimation.setMask(mAnimation.createHeuristicMask());

   mOverview = QPixmap(IconImages::OverviewWindowIcon);
   mOverview.setMask(mOverview.createHeuristicMask());

   mChipImage = QPixmap(IconImages::ChipImageIcon);
   mChipImage.setMask(mChipImage.createHeuristicMask());

   mWizard = QPixmap(IconImages::WizardBuilderIcon);
   mWizard.setMask(mWizard.createHeuristicMask());

   mPixelSpectrum = QPixmap(IconImages::PixelSpectrumIcon);
   mPixelSpectrum.setMask(QPixmap(IconImages::PixelSpectrumMask));

   mBackgroundTask = QPixmap(IconImages::BackgroundTaskIcon);
   mBackgroundTask.setMask(mBackgroundTask.createHeuristicMask());

   // Window actions
   mCascade = QPixmap(IconImages::CascadeIcon);
   mCascade.setMask(mCascade.createHeuristicMask());

   mTile = QPixmap(IconImages::TileIcon);
   mTile.setMask(mTile.createHeuristicMask());

   mTileHoriz = QPixmap(IconImages::TileHorizIcon);
   mTileHoriz.setMask(mTileHoriz.createHeuristicMask());

   mTileVert = QPixmap(IconImages::TileVertIcon);
   mTileVert.setMask(mTileVert.createHeuristicMask());

   mWorkbook = QPixmap(IconImages::WorkbookModeIcon);
   mWorkbook.setMask(mWorkbook.createHeuristicMask());

   mSplitHoriz = QPixmap(IconImages::SplitHorizontallyIcon);
   mSplitHoriz.setMask(mSplitHoriz.createHeuristicMask());

   mSplitVert = QPixmap(IconImages::SplitVerticallyIcon);
   mSplitVert.setMask(mSplitVert.createHeuristicMask());

   mLink = QPixmap(IconImages::LinkIcon);
   mLink.setMask(mLink.createHeuristicMask());

   mWindowList = QPixmap(IconImages::WindowListIcon);
   mWindowList.setMask(mWindowList.createHeuristicMask());

   // Help actions
   mAbout = QPixmap(IconImages::AboutIcon);
   mAbout.setMask(mAbout.createHeuristicMask());

   mWhatsThis = QPixmap(IconImages::WhatsThisIcon);
   mWhatsThis.setMask(mWhatsThis.createHeuristicMask());

   mHelp = QPixmap(IconImages::HelpTopicsIcon);
   mHelp.setMask(mHelp.createHeuristicMask());

   // Layer toolbar
   mHideLayers = QPixmap(IconImages::HideLayersIcon);
   mHideLayers.setMask(mHideLayers.createHeuristicMask());

   mShowLayers = QPixmap(IconImages::ShowLayersIcon);
   mShowLayers.setMask(mShowLayers.createHeuristicMask());

   // AOI toolbar
   mAOINew = QPixmap(IconImages::AoiNewIcon);
   mAOINew.setMask(mAOINew.createHeuristicMask());

   mAOILoad = QPixmap(IconImages::AoiLoadIcon);
   mAOILoad.setMask(mAOILoad.createHeuristicMask());

   mAOISave = QPixmap(IconImages::AoiSaveIcon);
   mAOISave.setMask(mAOISave.createHeuristicMask());

   mDelete = QPixmap(IconImages::DeleteIcon);
   mDelete.setMask(mDelete.createHeuristicMask());

   mAOIRename = QPixmap(IconImages::AoiRenameIcon);
   mAOIRename.setMask(mAOIRename.createHeuristicMask());

   mMerge = QPixmap(IconImages::MergeIcon);
   mMerge.setMask(mMerge.createHeuristicMask());

   mEdit = QPixmap(IconImages::EditIcon);
   mEdit.setMask(mEdit.createHeuristicMask());

   mDrawPixel = QPixmap(IconImages::DrawPixelIcon);
   mDrawPixel.setMask(mDrawPixel.createHeuristicMask());

   mRectangle = QPixmap(IconImages::RectangleIcon);
   mRectangle.setMask(QPixmap(IconImages::RectangleMask));

   mErasePixel = QPixmap(IconImages::ErasePixelIcon);
   mErasePixel.setMask(mErasePixel.createHeuristicMask());

   mEraseRect = QPixmap(IconImages::EraseRectIcon);
   mEraseRect.setMask(mEraseRect.createHeuristicMask());

   mErasePolygon = QPixmap(IconImages::ErasePolygonIcon);
   mErasePolygon.setMask(mErasePolygon.createHeuristicMask());

   mEraseRow = QPixmap(IconImages::EraseRowIcon);
   mEraseRow.setMask(mEraseRow.createHeuristicMask());

   mEraseCol = QPixmap(IconImages::EraseColumnIcon);
   mEraseCol.setMask(mEraseCol.createHeuristicMask());

   mEraseAll = QPixmap(IconImages::EraseAllIcon);
   mEraseAll.setMask(mEraseAll.createHeuristicMask());

   mPoint = QPixmap(IconImages::PointIcon);
   mPoint.setMask(mPoint.createHeuristicMask());

   mHLine = QPixmap(IconImages::HLineIcon);
   mHLine.setMask(mHLine.createHeuristicMask());

   mVLine = QPixmap(IconImages::VLineIcon);
   mVLine.setMask(mVLine.createHeuristicMask());

   mRow = QPixmap(IconImages::RowIcon);
   mRow.setMask(mRow.createHeuristicMask());

   mColumn = QPixmap(IconImages::ColumnIcon);
   mColumn.setMask(mColumn.createHeuristicMask());

   mTogglePixel = QPixmap(IconImages::TogglePixelIcon);
   mTogglePixel.setMask(mTogglePixel.createHeuristicMask());

   mToggleRect = QPixmap(IconImages::ToggleRectIcon);
   mToggleRect.setMask(mToggleRect.createHeuristicMask());

   mTogglePolygon = QPixmap(IconImages::TogglePolygonIcon);
   mTogglePolygon.setMask(mTogglePolygon.createHeuristicMask());

   mToggleAll = QPixmap(IconImages::ToggleAllIcon);
   mToggleAll.setMask(mToggleAll.createHeuristicMask());

   mShape = QPixmap(IconImages::ShapeIcon);
   mShape.setMask(mShape.createHeuristicMask());

   mAOIColor = QPixmap(IconImages::AoiColorIcon);
   mAOIColor.setMask(mAOIColor.createHeuristicMask());

   mViewSpectra = QPixmap(IconImages::AoiSpectraIcon);
   mViewSpectra.setMask(mViewSpectra.createHeuristicMask());

   mAOIProp = QPixmap(IconImages::AoiPropertiesIcon);
   mAOIProp.setMask(mAOIProp.createHeuristicMask());

   mAoiAddAppend = QPixmap(IconImages::AoiAddAppendIcon);
   mAoiAddAppend.setMask(mAoiAddAppend.createHeuristicMask());

   mAoiAddReplace = QPixmap(IconImages::AoiAddReplaceIcon);
   mAoiAddReplace.setMask(mAoiAddReplace.createHeuristicMask());

   mAoiAddNew = QPixmap(IconImages::AoiAddNewIcon);
   mAoiAddNew.setMask(mAoiAddNew.createHeuristicMask());

   mAoiShowLabels = QPixmap(IconImages::AoiShowLabelsIcon);
   mAoiShowLabels.setMask(mAoiShowLabels.createHeuristicMask());

   mAoiShowPointLabels = QPixmap(IconImages::AoiShowPointLabelsIcon);
   mAoiShowPointLabels.setMask(mAoiShowPointLabels.createHeuristicMask());

   // Annotation toolbar
   mAnnoNew = QPixmap(IconImages::AnnotationNewIcon);
   mAnnoNew.setMask(mAnnoNew.createHeuristicMask());

   mAnnoLoad = QPixmap(IconImages::AnnotationLoadIcon);
   mAnnoLoad.setMask(mAnnoLoad.createHeuristicMask());

   mAnnoSave = QPixmap(IconImages::AnnotationSaveIcon);
   mAnnoSave.setMask(mAnnoSave.createHeuristicMask());

   mAnnoRename = QPixmap(IconImages::AnnotationRenameIcon);
   mAnnoRename.setMask(mAnnoRename.createHeuristicMask());

   mTextBox = QPixmap(IconImages::TextBoxIcon);
   mTextBox.setMask(mTextBox.createHeuristicMask());

   mLine = QPixmap(IconImages::LineIcon);
   mLine.setMask(mLine.createHeuristicMask());

   mArrow = QPixmap(IconImages::ArrowIcon);
   mArrow.setMask(mArrow.createHeuristicMask());

   mPolyline = QPixmap(IconImages::PolylineIcon);
   mPolyline.setMask(mPolyline.createHeuristicMask());

   mRoundedRect = QPixmap(IconImages::RoundedRectangleIcon);
   mRoundedRect.setMask(QPixmap(IconImages::RoundedRectangleMask));

   mEllipse = QPixmap(IconImages::EllipseIcon);
   mEllipse.setMask(QPixmap(IconImages::EllipseMask));

   mCircle = QPixmap(IconImages::CircleIcon);
   mCircle.setMask(mCircle.createHeuristicMask());

   mTriangle = QPixmap(IconImages::TriangleIcon);
   mTriangle.setMask(QPixmap(IconImages::TriangleMask));

   mPolygon = QPixmap(IconImages::PolygonIcon);
   mPolygon.setMask(QPixmap(IconImages::PolygonMask));

   mArc = QPixmap(IconImages::ArcIcon);
   mArc.setMask(mArc.createHeuristicMask());

   mCircleArc = QPixmap(IconImages::CircleArcIcon);
   mCircleArc.setMask(mCircleArc.createHeuristicMask());

   mScaleBar = QPixmap(IconImages::ScaleBarIcon);
   mScaleBar.setMask(mScaleBar.createHeuristicMask());

   mLatLonInsert = QPixmap(IconImages::LatLonInsertIcon);
   mLatLonInsert.setMask(mLatLonInsert.createHeuristicMask());

   mNorthArrow = QPixmap(IconImages::NorthArrowIcon);
   mNorthArrow.setMask(mNorthArrow.createHeuristicMask());

   mEastArrow = QPixmap(IconImages::EastArrowIcon);
   mEastArrow.setMask(mEastArrow.createHeuristicMask());

   mTextColor = QPixmap(IconImages::TextColorIcon);
   mTextColor.setMask(mTextColor.createHeuristicMask());

   mBold = QPixmap(IconImages::BoldIcon);
   mBold.setMask(mBold.createHeuristicMask());

   mItalics = QPixmap(IconImages::ItalicsIcon);
   mItalics.setMask(mItalics.createHeuristicMask());

   mUnderline = QPixmap(IconImages::UnderlineIcon);
   mUnderline.setMask(mUnderline.createHeuristicMask());

   mLineWidth = QPixmap(IconImages::LineWidthIcon);
   mLineWidth.setMask(mLineWidth.createHeuristicMask());

   mLineColor = QPixmap(IconImages::LineColorIcon);
   mLineColor.setMask(QPixmap(IconImages::LineColorMask));

   mFillColor = QPixmap(IconImages::FillColorIcon);
   mFillColor.setMask(mFillColor.createHeuristicMask());

   mShadow = QPixmap(IconImages::ShadowIcon);
   mShadow.setMask(mShadow.createHeuristicMask());

   mShadowColor = QPixmap(IconImages::ShadowColorIcon);
   mShadowColor.setMask(mShadowColor.createHeuristicMask());

   mGroup = QPixmap(IconImages::GroupIcon);
   mGroup.setMask(mGroup.createHeuristicMask());

   mUngroup = QPixmap(IconImages::UngroupIcon);
   mUngroup.setMask(mUngroup.createHeuristicMask());

   mAutoAnno = QPixmap(IconImages::AutoAnnotateIcon);
   mAutoAnno.setMask(mAutoAnno.createHeuristicMask());

   mPopFront = QPixmap(IconImages::PopFrontIcon);
   mPopFront.setMask(mPopFront.createHeuristicMask());

   mPushBack = QPixmap(IconImages::PushBackIcon);
   mPushBack.setMask(mPushBack.createHeuristicMask());

   mSnapGrid = QPixmap(IconImages::SnapGridIcon);
   mSnapGrid.setMask(mSnapGrid.createHeuristicMask());

   mSnapObject = QPixmap(IconImages::SnapObjectIcon);
   mSnapObject.setMask(mSnapObject.createHeuristicMask());

   mNudgeUp = QPixmap(IconImages::NudgeUpIcon);
   mNudgeUp.setMask(mNudgeUp.createHeuristicMask());

   mNudgeDown = QPixmap(IconImages::NudgeDownIcon);
   mNudgeDown.setMask(mNudgeDown.createHeuristicMask());

   mNudgeLeft = QPixmap(IconImages::NudgeLeftIcon);
   mNudgeLeft.setMask(mNudgeLeft.createHeuristicMask());

   mNudgeRight = QPixmap(IconImages::NudgeRightIcon);
   mNudgeRight.setMask(mNudgeRight.createHeuristicMask());

   mAlignLeft = QPixmap(IconImages::AlignLeftIcon);
   mAlignLeft.setMask(mAlignLeft.createHeuristicMask());

   mAlignCenter = QPixmap(IconImages::AlignCenterIcon);
   mAlignCenter.setMask(mAlignCenter.createHeuristicMask());

   mAlignRight = QPixmap(IconImages::AlignRightIcon);
   mAlignRight.setMask(mAlignRight.createHeuristicMask());

   mAlignTop = QPixmap(IconImages::AlignTopIcon);
   mAlignTop.setMask(mAlignTop.createHeuristicMask());

   mAlignMiddle = QPixmap(IconImages::AlignMiddleIcon);
   mAlignMiddle.setMask(mAlignMiddle.createHeuristicMask());

   mAlignBottom = QPixmap(IconImages::AlignBottomIcon);
   mAlignBottom.setMask(mAlignBottom.createHeuristicMask());

   mDistributeHorizontally = QPixmap(IconImages::DistributeHorizontallyIcon);
   mDistributeHorizontally.setMask(mDistributeHorizontally.createHeuristicMask());

   mDistributeVertically = QPixmap(IconImages::DistributeVerticallyIcon);
   mDistributeVertically.setMask(mDistributeVertically.createHeuristicMask());

   mInsertPict = QPixmap(IconImages::InsertPictureIcon);
   mInsertPict.setMask(QPixmap(IconImages::InsertPictureMask));

   mInsertGLView = QPixmap(IconImages::InsertGLViewIcon);
   mInsertGLView.setMask(mInsertGLView.createHeuristicMask());

   mAnnoProp = QPixmap(IconImages::AnnotationPropertiesIcon);
   mAnnoProp.setMask(mAnnoProp.createHeuristicMask());

   // Brightness toolbar
   mResetStretch = QPixmap(IconImages::ResetStretchIcon);
   mResetStretch.setMask(mResetStretch.createHeuristicMask());

   // GCP toolbar
   mGCPNew = QPixmap(IconImages::GcpNewIcon);
   mGCPNew.setMask(mGCPNew.createHeuristicMask());

   mGCPLoad = QPixmap(IconImages::GcpLoadIcon);
   mGCPLoad.setMask(mGCPLoad.createHeuristicMask());

   mGCPSave = QPixmap(IconImages::GcpSaveIcon);
   mGCPSave.setMask(mGCPSave.createHeuristicMask());

   mGCPRename = QPixmap(IconImages::GcpRenameIcon);
   mGCPRename.setMask(mGCPRename.createHeuristicMask());

   mGCPColor = QPixmap(IconImages::GcpColorIcon);
   mGCPColor.setMask(mGCPColor.createHeuristicMask());

   mGCPProp = QPixmap(IconImages::GcpPropertiesIcon);
   mGCPProp.setMask(mGCPProp.createHeuristicMask());

   mGcpEditor = QPixmap(IconImages::GcpEditorIcon);
   mGcpEditor.setMask(mGcpEditor.createHeuristicMask());

   // Measurement toolbar
   mMeasurementColor = QPixmap(IconImages::MeasurementColorIcon);
   mMeasurementColor.setMask(mMeasurementColor.createHeuristicMask());

   mMeasurementStyle = QPixmap(IconImages::MeasurementStyleIcon);
   mMeasurementStyle.setMask(mMeasurementStyle.createHeuristicMask());

   mMeasurementLocationUnits = QPixmap(IconImages::MeasurementLocationUnitsIcon);
   mMeasurementLocationUnits.setMask(mMeasurementLocationUnits.createHeuristicMask());

   mMeasurementDistanceUnits = QPixmap(IconImages::MeasurementDistanceUnitsIcon);
   mMeasurementDistanceUnits.setMask(mMeasurementDistanceUnits.createHeuristicMask());

   mMeasurementDistanceLabelOnOff = QPixmap(IconImages::MeasurementDistanceLabelOnOffIcon);
   mMeasurementDistanceLabelOnOff.setMask(mMeasurementDistanceLabelOnOff.createHeuristicMask());

   mMeasurementBearingLabelOnOff = QPixmap(IconImages::MeasurementBearingLabelOnOffIcon);
   mMeasurementBearingLabelOnOff.setMask(mMeasurementBearingLabelOnOff.createHeuristicMask());

   mMeasurementEndPtsLabelOnOff = QPixmap(IconImages::MeasurementEndPtsLabelOnOffIcon);
   mMeasurementEndPtsLabelOnOff.setMask(mMeasurementEndPtsLabelOnOff.createHeuristicMask());

   mMeasurementNoLocUnit = QPixmap(IconImages::MeasurementNoLocUnitIcon);
   mMeasurementNoLocUnit.setMask(mMeasurementNoLocUnit.createHeuristicMask());

   mMeasurementDecDeg = QPixmap(IconImages::MeasurementDecDegIcon);
   mMeasurementDecDeg.setMask(mMeasurementDecDeg.createHeuristicMask());

   mMeasurementDecMin = QPixmap(IconImages::MeasurementDecMinIcon);
   mMeasurementDecMin.setMask(mMeasurementDecMin.createHeuristicMask());

   mMeasurementDms = QPixmap(IconImages::MeasurementDmsIcon);
   mMeasurementDms.setMask(mMeasurementDms.createHeuristicMask());

   mMeasurementMgrs = QPixmap(IconImages::MeasurementMgrsIcon);
   mMeasurementMgrs.setMask(mMeasurementMgrs.createHeuristicMask());

   mMeasurementUtm = QPixmap(IconImages::MeasurementUtmIcon);
   mMeasurementUtm.setMask(mMeasurementUtm.createHeuristicMask());

   mMeasurementNoDistUnit = QPixmap(IconImages::MeasurementNoDistUnitIcon);
   mMeasurementNoDistUnit.setMask(mMeasurementNoDistUnit.createHeuristicMask());

   mMeasurementKm = QPixmap(IconImages::MeasurementKmIcon);
   mMeasurementKm.setMask(mMeasurementKm.createHeuristicMask());

   mMeasurementStatMile = QPixmap(IconImages::MeasurementStatMileIcon);
   mMeasurementStatMile.setMask(mMeasurementStatMile.createHeuristicMask());

   mMeasurementNautMile = QPixmap(IconImages::MeasurementNautMileIcon);
   mMeasurementNautMile.setMask(mMeasurementNautMile.createHeuristicMask());

   mMeasurementMeter = QPixmap(IconImages::MeasurementMeterIcon);
   mMeasurementMeter.setMask(mMeasurementMeter.createHeuristicMask());

   mMeasurementYard = QPixmap(IconImages::MeasurementYardIcon);
   mMeasurementYard.setMask(mMeasurementYard.createHeuristicMask());

   mMeasurementFoot = QPixmap(IconImages::MeasurementFootIcon);
   mMeasurementFoot.setMask(mMeasurementFoot.createHeuristicMask());

   // TiePoint toolbar
   mTiePointColor = QPixmap(IconImages::TiePointColorIcon);
   mTiePointColor.setMask(mTiePointColor.createHeuristicMask());

   mTiePointEditor = QPixmap(IconImages::TiePointEditorIcon);
   mTiePointEditor.setMask(mTiePointEditor.createHeuristicMask());

   mTiePointLabels = QPixmap(IconImages::TiePointLabelsIcon);
   mTiePointLabels.setMask(mTiePointLabels.createHeuristicMask());

   // Docking windows
   mHide = QPixmap(IconImages::HideIcon);
   mVert_Expand = QPixmap(IconImages::VerticalExpandIcon);
   mHoriz_Expand = QPixmap(IconImages::HorizontalExpandIcon);

   // Spectral Data windows
   mSpectralData = QPixmap(IconImages::SpectralDataIcon);
   mSpectralData.setMask(mSpectralData.createHeuristicMask());

   mAnnoCursor = QPixmap(IconImages::AnnotationCursor);
   mAnnoMask = QPixmap(IconImages::AnnotationMask);
   mAoiCursor = QPixmap(IconImages::AoiCursor);
   mAoiMask = QPixmap(IconImages::AoiMask);
   mGcpCursor = QPixmap(IconImages::GcpMarkerCursor);
   mGcpMask = QPixmap(IconImages::GcpMarkerMask);
   mFreeRotateCursor = QPixmap(IconImages::FreeRotateCursor);
   mFreeRotateMask = QPixmap(IconImages::FreeRotateMask);
   mTiePointCursor = QPixmap(IconImages::TiePointMarkerCursor);
   mTiePointMask = QPixmap(IconImages::TiePointMarkerMask);
   mZoomInCursor = QPixmap(IconImages::ZoomInCursor);
   mZoomInMask = QPixmap(IconImages::ZoomInMask);
   mZoomOutCursor = QPixmap(IconImages::ZoomOutCursor);
   mZoomOutMask = QPixmap(IconImages::ZoomOutMask);
   mZoomRectCursor = QPixmap(IconImages::ZoomRectCursor);
   mZoomRectMask = QPixmap(IconImages::ZoomRectMask);
   mMeasurementCursor = QPixmap(IconImages::MeasurementCursor);
   mMeasurementMask = QPixmap(IconImages::MeasurementMask);

   // About dialog
   mApplicationLarge = QPixmap(":/images/application-large");
   mApplicationLarge.setMask(mApplicationLarge.createHeuristicMask());

   // Layer info window
   mAnnotation = QPixmap(IconImages::AnnotationIcon);
   mAnnotation.setMask(mAnnotation.createHeuristicMask());

   mGCPMarker = QPixmap(IconImages::GcpMarkerIcon);
   mGCPMarker.setMask(mGCPMarker.createHeuristicMask());

   mTiePointMarker = QPixmap(IconImages::TiePointMarkerIcon);
   mTiePointMarker.setMask(mTiePointMarker.createHeuristicMask());

   mMeasurementMarker = QPixmap(IconImages::MeasurementMarkerIcon);
   mMeasurementMarker.setMask(mMeasurementMarker.createHeuristicMask());

   // Spectrum window
   mOpenSig = QPixmap(IconImages::OpenSigIcon);
   mOpenSig.setMask(mOpenSig.createHeuristicMask());

   mSaveSig = QPixmap(IconImages::SaveSigIcon);
   mSaveSig.setMask(mSaveSig.createHeuristicMask());

   mMajorHorizGrid = QPixmap(IconImages::MajorHorizontalGridlinesIcon);
   mMajorHorizGrid.setMask(mMajorHorizGrid.createHeuristicMask());

   mMajorVertGrid = QPixmap(IconImages::MajorVerticalGridlinesIcon);
   mMajorVertGrid.setMask(mMajorVertGrid.createHeuristicMask());

   mMinorHorizGrid = QPixmap(IconImages::MinorHorizontalGridlinesIcon);
   mMinorHorizGrid.setMask(mMinorHorizGrid.createHeuristicMask());

   mMinorVertGrid = QPixmap(IconImages::MinorVerticalGridlinesIcon);
   mMinorVertGrid.setMask(mMinorVertGrid.createHeuristicMask());

   // Histogram window
   mSaveHistogram = QPixmap(IconImages::SaveHistogramIcon);
   mSaveHistogram.setMask(QPixmap(IconImages::SaveHistogramMask));

   // Script window
   mScript = QPixmap(IconImages::ScriptIcon);
   mScript.setMask(mScript.createHeuristicMask());

   // Animation toolbar
   mAnimationSpeedUp = QPixmap(IconImages::SpeedUpIcon);
   mAnimationSpeedUp.setMask(mAnimationSpeedUp.createHeuristicMask());

   mClock = QPixmap(IconImages::ClockIcon);
   mClock.setMask(mClock.createHeuristicMask());

   mAnimationAdvanceBackward = QPixmap(IconImages::AdvanceBackwardIcon);
   mAnimationAdvanceBackward.setMask(mAnimationAdvanceBackward.createHeuristicMask());

   mAnimationPlayForward = QPixmap(IconImages::PlayForwardIcon);
   mAnimationPlayForward.setMask(mAnimationPlayForward.createHeuristicMask());

   mAnimationAdvanceForward = QPixmap(IconImages::AdvanceForwardIcon);
   mAnimationAdvanceForward.setMask(mAnimationAdvanceForward.createHeuristicMask());

   mAnimationPause = QPixmap(IconImages::PauseIcon);
   mAnimationPause.setMask(mAnimationPause.createHeuristicMask());

   mAnimationBackwardDirection = QPixmap(IconImages::DirectionBackwardIcon);
   mAnimationBackwardDirection.setMask(mAnimationBackwardDirection.createMaskFromColor(QColor(255, 255, 255)));

   mAnimationChangeDirection = QPixmap(IconImages::ChangeDirectionIcon);
   mAnimationChangeDirection.setMask(mAnimationChangeDirection.createHeuristicMask());

   mAnimationForwardDirection = QPixmap(IconImages::DirectionForwardIcon);
   mAnimationForwardDirection.setMask(mAnimationForwardDirection.createMaskFromColor(QColor(255, 255, 255)));
   
   mAnimationStop = QPixmap(IconImages::StopIcon);
   mAnimationStop.setMask(mAnimationStop.createHeuristicMask());

   mAnimationSlowDown = QPixmap(IconImages::SlowDownIcon);
   mAnimationSlowDown.setMask(mAnimationSlowDown.createHeuristicMask());

   mPlayOnce = QPixmap(IconImages::PlayOnceIcon);
   mPlayOnce.setMask(mPlayOnce.createHeuristicMask());

   mBouncePlay = QPixmap(IconImages::PlayBounceIcon);
   mBouncePlay.setMask(mBouncePlay.createHeuristicMask());

   mRepeatPlay = QPixmap(IconImages::PlayRepeatIcon);
   mRepeatPlay.setMask(mRepeatPlay.createHeuristicMask());

   mAnimationBumpers = QPixmap(IconImages::AnimationBumpersIcon);
   mAnimationBumpers.setMask(mAnimationBumpers.createHeuristicMask());

   // Wizard builder
   mDesktop = QPixmap(IconImages::DesktopIcon);
   mDesktop.setMask(mDesktop.createHeuristicMask());

   mPlugIn = QPixmap(IconImages::PlugInIcon);
   mPlugIn.setMask(mPlugIn.createHeuristicMask());

   mValue = QPixmap(IconImages::ValueItemIcon);
   mValue.setMask(mValue.createHeuristicMask());

   mValueEdit = QPixmap(IconImages::ValueEditIcon);
   mValueEdit.setMask(mValueEdit.createHeuristicMask());

   mIncrease = QPixmap(IconImages::IncreaseIcon);
   mIncrease.setMask(mIncrease.createHeuristicMask());

   mDecrease = QPixmap(IconImages::DecreaseIcon);
   mDecrease.setMask(mDecrease.createHeuristicMask());

   // Modules
   mModule = QPixmap(IconImages::ModuleIcon);
   mModule.setMask(QPixmap(IconImages::ModuleMask));

   // Widgets
   mChecked = QPixmap(IconImages::CheckedIcon);
   mSemiChecked = QPixmap(IconImages::SemiCheckedIcon);
   mUnchecked = QPixmap(IconImages::UncheckedIcon);

   // Miscellaneous
   mForbidden = QPixmap(IconImages::ForbiddenIcon);
   mForbidden.setMask(mForbidden.createMaskFromColor(Qt::white));

   mGcpX = QPixmap(IconImages::GcpXIcon);
   mGcpX.setMask(mGcpX.createMaskFromColor(Qt::white));

   mGcpPlus = QPixmap(IconImages::GcpPlusIcon);
   mGcpPlus.setMask(mGcpPlus.createMaskFromColor(Qt::white));

   mGcpBlank = QPixmap(IconImages::GcpBlankIcon);
   mGcpBlank.setMask(mGcpBlank.createMaskFromColor(Qt::white));

   mOk = QPixmap(IconImages::OkIcon);
   mOk.setMask(mOk.createHeuristicMask());

   mCritical = QPixmap(IconImages::CriticalIcon);
   mCritical.setMask(mCritical.createHeuristicMask());

   // Layers
   mRasterLayer = QPixmap(IconImages::RasterLayerIcon);

   mThresholdLayer = QPixmap(IconImages::ThresholdLayerIcon);

   mPseudocolorLayer = QPixmap(IconImages::PseudocolorLayerIcon);
}

Icons::~Icons()
{
}

QPixmap Icons::getLineWidthPixmap(int iWidth)
{
   QPixmap pix(100, iWidth);
   pix.fill(Qt::black);

   return pix;
}
