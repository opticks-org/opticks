/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ICONS_H
#define ICONS_H

#include <QtGui/QBitmap>
#include <QtGui/QPixmap>

#include "TypesFile.h"

/**
 *  Repository of menu command and toolbar button icons.
 *
 *  The Icons class is a singleton class that contains QPixmap and QBitmap objects.
 *  These icons are stored here in a central location to provide easy access for other
 *  GUI objects.  The icons are divided into categories based on their functionality.
 *  The instance() static method provides access to the class.
 */
class Icons
{
public:
   /**
    *  Returns a pointer to the only instance of the class.
    *
    *  @return  A pointer to the class instance.
    */
   static Icons* instance();

   /**
    *  Returns a pixmap to display a line width
    *
    *  @param   iWidth
    *           The line width for which to get a pixmap.
    *
    *  @return  The line width pixmap.  The pixmap has a white background with a
    *           black line.
    */
   QPixmap getLineWidthPixmap(int iWidth);

   // File actions
   QPixmap mNew;
   QPixmap mOpen;
   QPixmap mClose;
   QPixmap mSave;
   QPixmap mPageSetup;
   QPixmap mPreview;
   QPixmap mPrint;

   // Edit actions
   QPixmap mUndo;
   QPixmap mRedo;
   QPixmap mCut;
   QPixmap mCopy;
   QPixmap mPaste;
   QPixmap mFind;
   QPixmap mFindNext;

   // Pan actions
   QPixmap mPan;
   QPixmap mZoomAndPanToPoint;

   // View actions
   QPixmap mRefresh;
   QPixmap mDisplayMode;
   QPixmap mGenerate;

   // Rotate actions
   QPixmap mRotateLeft;
   QPixmap mRotateRight;
   QPixmap mFlipHoriz;
   QPixmap mFlipVert;
   QPixmap mRotateBy;
   QPixmap mFreeRotate;
   QPixmap mReset;

   // Zoom actions
   QPixmap mZoomIn;
   QPixmap mZoomOut;
   QPixmap mZoomPointIn;
   QPixmap mZoomPointOut;
   QPixmap mZoomRect;
   QPixmap mZoomToFit;
   QPixmap mUndoZoom;
   QPixmap mRedoZoom;

   // Properties actions
   QPixmap mProperties;
   QPixmap mDataProp;
   QPixmap mLayerProp;

   // Tools actions
   QPixmap mSessionExplorer;
   QPixmap mSignature;
   QPixmap mHistogram;
   QPixmap mMessageLog;
   QPixmap mShortcut;
   QPixmap mLayers;
   QPixmap mScript;
   QPixmap mAnimation;
   QPixmap mOverview;
   QPixmap mChipImage;
   QPixmap mWizard;
   QPixmap mPixelSpectrum;
   QPixmap mBackgroundTask;

   // Window actions
   QPixmap mCascade;
   QPixmap mTile;
   QPixmap mTileHoriz;
   QPixmap mTileVert;
   QPixmap mWorkbook;
   QPixmap mSplitHoriz;
   QPixmap mSplitVert;
   QPixmap mLink;
   QPixmap mWindowList;

   // Help actions
   QPixmap mAbout;
   QPixmap mWhatsThis;
   QPixmap mHelp;

   // Layer actions
   QPixmap mHideLayers;
   QPixmap mShowLayers;

   // AOI toolbar
   QPixmap mAOINew;
   QPixmap mAOILoad;
   QPixmap mAOISave;
   QPixmap mDelete;
   QPixmap mAOIRename;
   QPixmap mMerge;
   QPixmap mEdit;
   QPixmap mDrawPixel;
   QPixmap mRectangle;
   QPixmap mErasePixel;
   QPixmap mEraseRect;
   QPixmap mErasePolygon;
   QPixmap mEraseRow;
   QPixmap mEraseCol;
   QPixmap mEraseAll;
   QPixmap mPoint;
   QPixmap mHLine;
   QPixmap mVLine;
   QPixmap mRow;
   QPixmap mColumn;
   QPixmap mTogglePixel;
   QPixmap mToggleRect;
   QPixmap mTogglePolygon;
   QPixmap mToggleAll;
   QPixmap mShape;
   QPixmap mAOIColor;
   QPixmap mViewSpectra;
   QPixmap mAOIProp;
   QPixmap mAoiAddAppend;
   QPixmap mAoiAddReplace;
   QPixmap mAoiAddNew;
   QPixmap mAoiShowLabels;
   QPixmap mAoiShowPointLabels;

   // Annotation toolbar
   QPixmap mAnnoNew;
   QPixmap mAnnoLoad;
   QPixmap mAnnoSave;
   QPixmap mAnnoRename;
   QPixmap mTextBox;
   QPixmap mLine;
   QPixmap mArrow;
   QPixmap mPolyline;
   QPixmap mRoundedRect;
   QPixmap mEllipse;
   QPixmap mCircle;
   QPixmap mTriangle;
   QPixmap mPolygon;
   QPixmap mArc;
   QPixmap mCircleArc;
   QPixmap mScaleBar;
   QPixmap mLatLonInsert;
   QPixmap mNorthArrow;
   QPixmap mEastArrow;
   QPixmap mTextColor;
   QPixmap mBold;
   QPixmap mItalics;
   QPixmap mUnderline;
   QPixmap mLineWidth;
   QPixmap mLineColor;
   QPixmap mFillColor;
   QPixmap mShadow;
   QPixmap mShadowColor;
   QPixmap mGroup;
   QPixmap mUngroup;
   QPixmap mAutoAnno;
   QPixmap mPopFront;
   QPixmap mPushBack;
   QPixmap mSnapGrid;
   QPixmap mSnapObject;
   QPixmap mNudgeUp;
   QPixmap mNudgeDown;
   QPixmap mNudgeLeft;
   QPixmap mNudgeRight;
   QPixmap mAlignLeft;
   QPixmap mAlignCenter;
   QPixmap mAlignRight;
   QPixmap mAlignTop;
   QPixmap mAlignMiddle;
   QPixmap mAlignBottom;
   QPixmap mDistributeHorizontally;
   QPixmap mDistributeVertically;
   QPixmap mInsertPict;
   QPixmap mInsertGLView;
   QPixmap mAnnoProp;

   // Brightness toolbar
   QPixmap mResetStretch;

   // GCP toolbar
   QPixmap mGCPNew;
   QPixmap mGCPLoad;
   QPixmap mGCPSave;
   QPixmap mGCPRename;
   QPixmap mGCPColor;
   QPixmap mGCPProp;
   QPixmap mGcpEditor;

   // Measurement toolbar
   QPixmap mMeasurementColor;
   QPixmap mMeasurementStyle;
   QPixmap mMeasurementLocationUnits;
   QPixmap mMeasurementDistanceUnits;
   QPixmap mMeasurementDistanceLabelOnOff;
   QPixmap mMeasurementBearingLabelOnOff;
   QPixmap mMeasurementEndPtsLabelOnOff;
   QPixmap mMeasurementNoLocUnit;
   QPixmap mMeasurementDecDeg;
   QPixmap mMeasurementDecMin;
   QPixmap mMeasurementDms;
   QPixmap mMeasurementUtm;
   QPixmap mMeasurementMgrs;
   QPixmap mMeasurementNoDistUnit;
   QPixmap mMeasurementKm;
   QPixmap mMeasurementStatMile;
   QPixmap mMeasurementNautMile;
   QPixmap mMeasurementMeter;
   QPixmap mMeasurementYard;
   QPixmap mMeasurementFoot;

   // TiePoint toolbar
   QPixmap mTiePointColor;
   QPixmap mTiePointEditor;
   QPixmap mTiePointLabels;

   // Docking windows
   QPixmap mHide;
   QPixmap mVert_Expand;
   QPixmap mHoriz_Expand;

   // Spectral Data windows
   QPixmap mSpectralData;
   QBitmap mAnnoCursor;
   QBitmap mAnnoMask;
   QBitmap mAoiCursor;
   QBitmap mAoiMask;
   QBitmap mGcpCursor;
   QBitmap mGcpMask;
   QBitmap mFreeRotateCursor;
   QBitmap mFreeRotateMask;
   QBitmap mTiePointCursor;
   QBitmap mTiePointMask;
   QBitmap mZoomInCursor;
   QBitmap mZoomInMask;
   QBitmap mZoomOutCursor;
   QBitmap mZoomOutMask;
   QBitmap mZoomRectCursor;
   QBitmap mZoomRectMask;
   QBitmap mMeasurementCursor;
   QBitmap mMeasurementMask;

   //About dialog
   QPixmap mApplicationLarge;

   // Layers
   QPixmap mAnnotation;
   QPixmap mGCPMarker;
   QPixmap mTiePointMarker;
   QPixmap mMeasurementMarker;

   // Plot window
   QPixmap mOpenSig;
   QPixmap mSaveSig;
   QPixmap mMajorHorizGrid;
   QPixmap mMajorVertGrid;
   QPixmap mMinorHorizGrid;
   QPixmap mMinorVertGrid;

   // Histogram window
   QPixmap mSaveHistogram;

   // Animation toolbar
   QPixmap mAnimationSpeedUp;
   QPixmap mAnimationPlayBackward;
   QPixmap mAnimationAdvanceBackward;
   QPixmap mAnimationPlayForward;
   QPixmap mAnimationAdvanceForward;
   QPixmap mAnimationPause;
   QPixmap mAnimationStop;
   QPixmap mAnimationSlowDown;
   QPixmap mAnimationBackwardDirection;
   QPixmap mAnimationForwardDirection;
   QPixmap mAnimationChangeDirection;
   QPixmap mAnimationBumpers;
   
   QPixmap mPlayOnce;
   QPixmap mBouncePlay;
   QPixmap mRepeatPlay;
   QPixmap mClock;

   // Wizard builder
   QPixmap mDesktop;
   QPixmap mPlugIn;
   QPixmap mValue;
   QPixmap mValueEdit;
   QPixmap mIncrease;
   QPixmap mDecrease;
   QPixmap mWizardItem;

   // Modules
   QPixmap mModule;

   // Widgets
   QPixmap mChecked;
   QPixmap mSemiChecked;
   QPixmap mUnchecked;

   // Miscellaneous
   QPixmap mForbidden;
   QPixmap mGcpX;
   QPixmap mGcpPlus;
   QPixmap mGcpBlank;
   QPixmap mOk;
   QPixmap mCritical;

   // Layers
   QPixmap mRasterLayer;
   QPixmap mThresholdLayer;
   QPixmap mPseudocolorLayer;

protected:
   /**
    *  Creates the Icons object.
    */
   Icons();

   /**
    *  Destroys the only Icons object.
    */
   ~Icons();

private:
   static Icons* singleton;
};

#endif
