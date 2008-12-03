/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LabeledSection.h"
#include "OptionsIceExporter.h"
#include "SessionManager.h"
#include "StringUtilities.h"

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>

#include <string>

using namespace std;

OptionsIceExporter::OptionsIceExporter() :
   LabeledSectionGroup(NULL), mSaveSettings(true)
{
   // Compression
   QWidget* pCompressionLayoutWidget = new QWidget(this);

   QLabel* pCompressionLabel = new QLabel("Compression:", pCompressionLayoutWidget);
   mpCompressionTypeCombo = new QComboBox(pCompressionLayoutWidget);
   std::vector<std::string> ctValues = StringUtilities::getAllEnumValuesAsDisplayString<IceCompressionType>();
   for (std::vector<std::string>::iterator ctValue = ctValues.begin(); ctValue != ctValues.end(); ++ctValue)
   {
      mpCompressionTypeCombo->addItem(QString::fromStdString(*ctValue));
   }

   QLabel* pGzipLevelLabel = new QLabel("GZIP Compression Level:", pCompressionLayoutWidget);
   mpGzipCompressionSlider = new QSlider(pCompressionLayoutWidget);
   mpGzipCompressionSlider->setRange(0, 9);
   mpGzipCompressionSlider->setOrientation(Qt::Horizontal);
   mpGzipCompressionSlider->setTickInterval(1);
   mpGzipCompressionSlider->setTickPosition(QSlider::TicksBelow);
   mpGzipCompressionSlider->setSingleStep(1);
   mpGzipCompressionSlider->setPageStep(2);
   mpGzipLevelValue = new QLabel(pCompressionLayoutWidget);

   // Chunk size
   QWidget* pChunkSizeLayoutWidget = new QWidget(this);

   QLabel* pChunkSizeLabel = new QLabel("Chunk size (MB):", pChunkSizeLayoutWidget);
   mpChunkSize = new QSpinBox(pChunkSizeLayoutWidget);
   mpChunkSize->setRange(1, 1024);
   mpChunkSize->setSuffix(" MB");
   mpChunkSize->setAccelerated(true);

   // Layout 
   QGridLayout* pCompressionLayout = new QGridLayout(pCompressionLayoutWidget);
   pCompressionLayout->setMargin(0);
   pCompressionLayout->setSpacing(5);
   pCompressionLayout->addWidget(pCompressionLabel, 0, 0);
   pCompressionLayout->addWidget(mpCompressionTypeCombo, 0, 1);
   pCompressionLayout->addWidget(pGzipLevelLabel, 1, 0);
   pCompressionLayout->addWidget(mpGzipCompressionSlider, 1, 1, 1, 2);
   pCompressionLayout->addWidget(mpGzipLevelValue, 1, 3);
   pCompressionLayout->setColumnStretch(2, 10);

   QHBoxLayout* pChunkSizeLayout = new QHBoxLayout(pChunkSizeLayoutWidget);
   pChunkSizeLayout->addWidget(pChunkSizeLabel);
   pChunkSizeLayout->addWidget(mpChunkSize);

   LabeledSection* pCompressionSection = new LabeledSection(pCompressionLayoutWidget, "Compression Options", this);
   LabeledSection* pChunkSizeSection = new LabeledSection(pChunkSizeLayoutWidget, "Chunk Size Options", this);

   // Initialization
   addSection(pCompressionSection);
   addSection(pChunkSizeSection);
   addStretch(10);
   setSizeHint(300, 150);

   VERIFYNR(connect(mpCompressionTypeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(compressionTypeChanged(const QString&))));
   VERIFYNR(connect(mpGzipCompressionSlider, SIGNAL(valueChanged(int)), this, SLOT(gzipCompressionValueChanged(int))));

   // Initialize From Settings
   IceCompressionType ctype(StringUtilities::fromXmlString<IceCompressionType>(IceWriter::getSettingCompressionType()));
   if (ctype.isValid())
   {
      mpCompressionTypeCombo->setCurrentIndex(mpCompressionTypeCombo->findText(QString::fromStdString(
         StringUtilities::toDisplayString(ctype))));
   }
   int clevel = IceWriter::getSettingGzipCompressionLevel();
   if (clevel >= 0 && clevel <= 9)
   {
      mpGzipCompressionSlider->setValue(clevel);
   }
   int csize = IceWriter::getSettingChunkSize();
   if (csize > 0)
   {
      mpChunkSize->setValue(csize);
   }

   compressionTypeChanged(mpCompressionTypeCombo->currentText());
}

void OptionsIceExporter::applyChanges()
{
   if (mSaveSettings)
   {
      IceWriter::setSettingCompressionType(StringUtilities::toXmlString(getCompressionType()));
      IceWriter::setSettingGzipCompressionLevel(getGzipCompressionLevel());
      IceWriter::setSettingChunkSize(getChunkSize());
   }
}

OptionsIceExporter::~OptionsIceExporter()
{
}

void OptionsIceExporter::setSaveSettings(bool saveSettings)
{
   mSaveSettings = saveSettings;
}

IceCompressionType OptionsIceExporter::getCompressionType()
{
   return StringUtilities::fromDisplayString<IceCompressionType>(mpCompressionTypeCombo->currentText().toStdString());
}

int OptionsIceExporter::getGzipCompressionLevel()
{
   return mpGzipCompressionSlider->value();
}

int OptionsIceExporter::getChunkSize()
{
   return mpChunkSize->value();
}

void OptionsIceExporter::compressionTypeChanged(const QString& value)
{
   IceCompressionType type(StringUtilities::fromDisplayString<IceCompressionType>(value.toStdString()));
   bool enableGzip = false;
   if (type.isValid())
   {
      switch(type)
      {
      case GZIP:
      case SHUFFLE_AND_GZIP:
         enableGzip = true;
         break;
      default:
         ; // do nothing
      }
   }
   mpGzipCompressionSlider->setEnabled(enableGzip);
   mpGzipLevelValue->setEnabled(enableGzip);
}

void OptionsIceExporter::gzipCompressionValueChanged(int value)
{
   mpGzipLevelValue->setText(QString::number(value));
}
