/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSMOVIEEXPORTER_H
#define OPTIONSMOVIEEXPORTER_H

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "LabeledSectionGroup.h"

#include <boost/rational.hpp>
#include <QtCore/QMetaType>

class AnimationController;
class LabeledSection;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QIcon;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;

Q_DECLARE_METATYPE(boost::rational<int>);

class OptionsMovieExporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsMovieExporter();
   ~OptionsMovieExporter();
   bool initialize(AnimationController* pController);

   /**
    * The video stream bitrate in kbps
    */
   SETTING(Bitrate, MovieExporter, unsigned int, 0);
   SETTING(MeMethod, MovieExporter, std::string, std::string());
   SETTING(GopSize, MovieExporter, int, 0);
   SETTING(QCompress, MovieExporter, float, 0.0);
   SETTING(QBlur, MovieExporter, float, 0.0);
   SETTING(QMinimum, MovieExporter, int, 0);
   SETTING(QMaximum, MovieExporter, int, 0);
   SETTING(QDiffMaximum, MovieExporter, int, 0);
   SETTING(MaxBFrames, MovieExporter, int, 0);
   SETTING(BQuantFactor, MovieExporter, float, 0.0);
   SETTING(BQuantOffset, MovieExporter, float, 0.0);
   SETTING(DiaSize, MovieExporter, int, 0);
   SETTING(Flags, MovieExporter, int, 0);

   void applyChanges();
   void setPromptUserToSaveSettings(bool prompt);

   void getResolution(unsigned int &width, unsigned int &height) const;
   void setResolution(unsigned int width, unsigned int height);

   double getStart() const;
   void setStart(double start);
   double getStop() const;
   void setStop(double stop);
   unsigned int getBitrate() const;
   void setFrameType(FrameType eType);
   void setBitrate(unsigned int bitrate);
   boost::rational<int> getFramerate() const;
   void setFramerate(boost::rational<int> frameRate);
   void setFramerates(std::vector<boost::rational<int> > frameRates);

   std::string getMeMethod() const;
   void setMeMethod(const std::string &method);
   int getGopSize() const;
   void setGopSize(int size);
   float getQCompress() const;
   void setQCompress(float val);
   float getQBlur() const;
   void setQBlur(float val);
   int getQMinimum() const;
   void setQMinimum(int val);
   int getQMaximum() const;
   void setQMaximum(int val);
   int getQDiffMaximum() const;
   void setQDiffMaximum(int val);
   int getMaxBFrames() const;
   void setMaxBFrames(int val);
   float getBQuantFactor() const;
   void setBQuantFactor(float val);
   float getBQuantOffset() const;
   void setBQuantOffset(float val);
   int getDiaSize() const;
   void setDiaSize(int val);
   int getFlags() const;
   void setFlags(int val);

   static const std::string& getName()
   {
      static std::string var = "Movie Product Exporter Options";
      return var;
   }

   static const std::string& getOptionName()
   {
      static std::string var = "Export/Movie Product";
      return var;
   }

   static const std::string& getDescription()
   {
      static std::string var = "Widget to display movie product exporter related options";
      return var;
   }

   static const std::string& getShortDescription()
   {
      static std::string var = "Widget to display movie product exporter related options";
      return var;
   }

   static const std::string& getCreator()
   {
      static std::string var = "Ball Aerospace & Technologies Corp.";
      return var;
   }

   static const std::string& getCopyright()
   {
      static std::string var = APP_COPYRIGHT_MSG;
      return var;
   }

   static const std::string& getVersion()
   {
      static std::string var = APP_VERSION_NUMBER;
      return var;
   }

   static bool isProduction()
   {
      return APP_IS_PRODUCTION_RELEASE;
   }

   static const std::string& getDescriptorId()
   {
      static std::string var = "{77309BDE-ED51-4e83-B2AF-5532823AB9F0}";
      return var;
   }

public slots:
   void setUseViewResolution(bool v);

private slots:
   void checkResolutionX(bool ignoreAspectRatio = false);
   void checkResolutionY(bool ignoreAspectRatio = false);
   void aspectLockToggled(bool state);
   void updateBitrate(int value);
   void frameRateListChanged(const QString &value);

private:
   QCheckBox *mpUseViewResolution;
   QLineEdit *mpResolutionX;
   QLineEdit *mpResolutionY;
   QPushButton *mpResolutionAspectLock;
   QIcon *mpLockIcon;
   QIcon *mpUnlockIcon;
   unsigned int mCurrentResolutionX;
   unsigned int mCurrentResolutionY;

   QSlider *mpBitrate;
   QLabel *mpBitrateValue;
   QLabel *mpStartLabel;
   QDoubleSpinBox *mpStart;
   QDoubleSpinBox *mpStop;
   QLabel *mpStopLabel;
   QSpinBox *mpFramerateNum;
   QSpinBox *mpFramerateDen;
   QComboBox *mpFramerateList;

   QComboBox *mpMeMethod;
   QSpinBox *mpGopSize;
   QDoubleSpinBox *mpQCompress;
   QDoubleSpinBox *mpQBlur;
   QSpinBox *mpQMinimum;
   QSpinBox *mpQMaximum;
   QSpinBox *mpQDiffMaximum;
   QSpinBox *mpMaxBFrames;
   QDoubleSpinBox *mpBQuantFactor;
   QDoubleSpinBox *mpBQuantOffset;
   QSpinBox *mpDiaSize;
   QCheckBox *mpQScale;
   QCheckBox *mpQPel;
   QCheckBox *mpGmc;
   QCheckBox *mpNormalizeAqp;
   QCheckBox *mpTrellis;
   QCheckBox *mpAcPred;
   QCheckBox *mpCbpRd;
   QCheckBox *mpQpRd;
   QCheckBox *mpObmc;
   QCheckBox *mpClosedGop;

   LabeledSection* mpSettingsSection;
   QCheckBox *mpSaveSettings;
};

#endif
