// qtractorOptionsForm.cpp
//
/****************************************************************************
   Copyright (C) 2005-2008, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorOptionsForm.h"

#include "qtractorAbout.h"
#include "qtractorOptions.h"

#include "qtractorAudioFile.h"
#include "qtractorMidiEditor.h"
#include "qtractorTimeScale.h"
#include "qtractorPlugin.h"

#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QValidator>
#include <QHeaderView>


//----------------------------------------------------------------------------
// qtractorOptionsForm -- UI wrapper form.

// Constructor.
qtractorOptionsForm::qtractorOptionsForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// No settings descriptor initially (the caller will set it).
	m_pOptions = NULL;

	// Populate the capture file type combo-box.
	m_ui.AudioCaptureTypeComboBox->clear();
	int iFormat = 0;
	const qtractorAudioFileFactory::FileFormats& list
		= qtractorAudioFileFactory::formats();
	QListIterator<qtractorAudioFileFactory::FileFormat *> iter(list);
	while (iter.hasNext()) {
		qtractorAudioFileFactory::FileFormat *pFormat = iter.next();
		if (pFormat->type != qtractorAudioFileFactory::MadFile)
			m_ui.AudioCaptureTypeComboBox->addItem(pFormat->name, iFormat);
		++iFormat;
	}

	// Populate the audio capture sample format combo-box.
	m_ui.AudioCaptureFormatComboBox->clear();
	m_ui.AudioCaptureFormatComboBox->addItem(tr("Signed 16-Bit"));
	m_ui.AudioCaptureFormatComboBox->addItem(tr("Signed 24-Bit"));
	m_ui.AudioCaptureFormatComboBox->addItem(tr("Signed 32-Bit"));
	m_ui.AudioCaptureFormatComboBox->addItem(tr("Float  32-Bit"));
	m_ui.AudioCaptureFormatComboBox->addItem(tr("Float  64-Bit"));

	// Populate the MIDI capture file format combo-box.
	m_ui.MidiCaptureFormatComboBox->clear();
	m_ui.MidiCaptureFormatComboBox->addItem(tr("SMF Format 0"));
	m_ui.MidiCaptureFormatComboBox->addItem(tr("SMF Format 1"));

	QStringList items = qtractorTimeScale::snapItems(0);
	m_ui.MidiCaptureQuantizeComboBox->clear();
	m_ui.MidiCaptureQuantizeComboBox->insertItems(0, items);

//	updateMetroNoteNames();

	m_ui.PluginTypeComboBox->clear();
#ifdef CONFIG_LADSPA
	m_ui.PluginTypeComboBox->addItem(
		qtractorPluginType::textFromHint(qtractorPluginType::Ladspa));
#endif
#ifdef CONFIG_DSSI
	m_ui.PluginTypeComboBox->addItem(
		qtractorPluginType::textFromHint(qtractorPluginType::Dssi));
#endif
#ifdef CONFIG_VST
	m_ui.PluginTypeComboBox->addItem(
		qtractorPluginType::textFromHint(qtractorPluginType::Vst));
#endif

	// Initialize dirty control state.
	m_iDirtyCount = 0;

	// Try to restore old window positioning.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.AudioCaptureTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioCaptureFormatComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioCaptureQualitySpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioResampleTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioAutoTimeStretchCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioQuickSeekCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioPlayerBusCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.AudioMetronomeCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBarFilenameComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBarFilenameToolButton,
		SIGNAL(clicked()),
		SLOT(chooseMetroBarFilename()));
	QObject::connect(m_ui.MetroBeatFilenameComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBeatFilenameToolButton,
		SIGNAL(clicked()),
		SLOT(chooseMetroBeatFilename()));
	QObject::connect(m_ui.AudioMetroBusCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MidiCaptureFormatComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MidiCaptureQuantizeComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MidiControlBusCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MidiMetronomeCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroChannelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(updateMetroNoteNames()));
	QObject::connect(m_ui.MetroBarNoteComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBarVelocitySpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBarDurationSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBeatNoteComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBeatVelocitySpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MetroBeatDurationSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MidiMetroBusCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.ConfirmRemoveCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.StdoutCaptureCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.CompletePathCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.PeakAutoRemoveCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.KeepToolsOnTopCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.TrackViewDropSpanCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.DisplayFormatComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MaxRecentFilesSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.PluginTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(choosePluginType(int)));
	QObject::connect(m_ui.PluginPathComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(changePluginPath(const QString&)));
	QObject::connect(m_ui.PluginPathBrowseToolButton,
		SIGNAL(clicked()),
		SLOT(choosePluginPath()));
	QObject::connect(m_ui.PluginPathAddToolButton,
		SIGNAL(clicked()),
		SLOT(addPluginPath()));
	QObject::connect(m_ui.PluginPathListWidget,
		SIGNAL(itemSelectionChanged()),
		SLOT(selectPluginPath()));
	QObject::connect(m_ui.PluginPathRemoveToolButton,
		SIGNAL(clicked()),
		SLOT(removePluginPath()));
	QObject::connect(m_ui.PluginPathUpToolButton,
		SIGNAL(clicked()),
		SLOT(moveUpPluginPath()));
	QObject::connect(m_ui.PluginPathDownToolButton,
		SIGNAL(clicked()),
		SLOT(moveDownPluginPath()));
	QObject::connect(m_ui.MessagesFontPushButton,
		SIGNAL(clicked()),
		SLOT(chooseMessagesFont()));
	QObject::connect(m_ui.MessagesLimitCheckBox,
		SIGNAL(stateChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.MessagesLimitLinesSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.OkPushButton,
		SIGNAL(clicked()),
		SLOT(accept()));
	QObject::connect(m_ui.CancelPushButton,
		SIGNAL(clicked()),
		SLOT(reject()));
}


// Destructor.
qtractorOptionsForm::~qtractorOptionsForm (void)
{
}


// Populate (setup) dialog controls from settings descriptors.
void qtractorOptionsForm::setOptions ( qtractorOptions *pOptions )
{
	// Set reference descriptor.
	m_pOptions = pOptions;

	// Initialize conveniency options...
	m_pOptions->loadComboBoxHistory(m_ui.MetroBarFilenameComboBox);
	m_pOptions->loadComboBoxHistory(m_ui.MetroBeatFilenameComboBox);
	m_pOptions->loadComboBoxHistory(m_ui.PluginPathComboBox);

	// Audio options.
	int iIndex  = 0;
	int iFormat = 0;
	const qtractorAudioFileFactory::FileFormats& list
		= qtractorAudioFileFactory::formats();
	QListIterator<qtractorAudioFileFactory::FileFormat *> iter(list);
	while (iter.hasNext()) {
		qtractorAudioFileFactory::FileFormat *pFormat = iter.next();
		if (m_pOptions->sAudioCaptureExt == pFormat->ext
			&& m_pOptions->iAudioCaptureType == pFormat->data) {
			iIndex = m_ui.AudioCaptureTypeComboBox->findData(iFormat);
			break;
		}
		++iFormat;
	}
	m_ui.AudioCaptureTypeComboBox->setCurrentIndex(iIndex);
	m_ui.AudioCaptureFormatComboBox->setCurrentIndex(m_pOptions->iAudioCaptureFormat);
	m_ui.AudioCaptureQualitySpinBox->setValue(m_pOptions->iAudioCaptureQuality);
	m_ui.AudioResampleTypeComboBox->setCurrentIndex(m_pOptions->iAudioResampleType);
	m_ui.AudioAutoTimeStretchCheckBox->setChecked(m_pOptions->bAudioAutoTimeStretch);
	m_ui.AudioQuickSeekCheckBox->setChecked(m_pOptions->bAudioQuickSeek);
	m_ui.AudioPlayerBusCheckBox->setChecked(m_pOptions->bAudioPlayerBus);

#ifndef CONFIG_LIBSAMPLERATE
	m_ui.AudioResampleTypeTextLabel->setEnabled(false);
	m_ui.AudioResampleTypeComboBox->setEnabled(false);
#endif

	// Audio metronome options.
	m_ui.AudioMetronomeCheckBox->setChecked(m_pOptions->bAudioMetronome);
	m_ui.MetroBarFilenameComboBox->setEditText(m_pOptions->sMetroBarFilename);
	m_ui.MetroBeatFilenameComboBox->setEditText(m_pOptions->sMetroBeatFilename);
	m_ui.AudioMetroBusCheckBox->setChecked(m_pOptions->bAudioMetroBus);

	// MIDI options.
	m_ui.MidiCaptureFormatComboBox->setCurrentIndex(m_pOptions->iMidiCaptureFormat);
	m_ui.MidiCaptureQuantizeComboBox->setCurrentIndex(m_pOptions->iMidiCaptureQuantize);
	m_ui.MidiControlBusCheckBox->setChecked(m_pOptions->bMidiControlBus);

	// MIDI metronome options.
	m_ui.MidiMetronomeCheckBox->setChecked(m_pOptions->bMidiMetronome);
	m_ui.MetroChannelSpinBox->setValue(m_pOptions->iMetroChannel + 1);
	updateMetroNoteNames();
	m_ui.MetroBarNoteComboBox->setCurrentIndex(m_pOptions->iMetroBarNote);
	m_ui.MetroBarVelocitySpinBox->setValue(m_pOptions->iMetroBarVelocity);
	m_ui.MetroBarDurationSpinBox->setValue(m_pOptions->iMetroBarDuration);
	m_ui.MetroBeatNoteComboBox->setCurrentIndex(m_pOptions->iMetroBeatNote);
	m_ui.MetroBeatVelocitySpinBox->setValue(m_pOptions->iMetroBeatVelocity);
	m_ui.MetroBeatDurationSpinBox->setValue(m_pOptions->iMetroBeatDuration);
	m_ui.MidiMetroBusCheckBox->setChecked(m_pOptions->bMidiMetroBus);

	// Load Display options...
	QFont font;
	// Messages font.
	if (m_pOptions->sMessagesFont.isEmpty()
		|| !font.fromString(m_pOptions->sMessagesFont))
		font = QFont("Monospace", 8);
	QPalette pal(m_ui.MessagesFontTextLabel->palette());
	pal.setColor(m_ui.MessagesFontTextLabel->backgroundRole(), Qt::white);
	m_ui.MessagesFontTextLabel->setPalette(pal);
	m_ui.MessagesFontTextLabel->setFont(font);
	m_ui.MessagesFontTextLabel->setText(
		font.family() + " " + QString::number(font.pointSize()));

	// Messages limit option.
	m_ui.MessagesLimitCheckBox->setChecked(m_pOptions->bMessagesLimit);
	m_ui.MessagesLimitLinesSpinBox->setValue(m_pOptions->iMessagesLimitLines);

	// Other options finally.
	m_ui.ConfirmRemoveCheckBox->setChecked(m_pOptions->bConfirmRemove);
	m_ui.StdoutCaptureCheckBox->setChecked(m_pOptions->bStdoutCapture);
	m_ui.CompletePathCheckBox->setChecked(m_pOptions->bCompletePath);
	m_ui.PeakAutoRemoveCheckBox->setChecked(m_pOptions->bPeakAutoRemove);
	m_ui.KeepToolsOnTopCheckBox->setChecked(m_pOptions->bKeepToolsOnTop);
	m_ui.TrackViewDropSpanCheckBox->setChecked(m_pOptions->bTrackViewDropSpan);
	m_ui.MaxRecentFilesSpinBox->setValue(m_pOptions->iMaxRecentFiles);
	m_ui.DisplayFormatComboBox->setCurrentIndex(m_pOptions->iDisplayFormat);

	// Plugin path initialization...
	m_ladspaPaths = m_pOptions->ladspaPaths;
	m_dssiPaths   = m_pOptions->dssiPaths;
	m_vstPaths    = m_pOptions->vstPaths;

	int iPluginType = m_pOptions->iPluginType - 1;
	if (iPluginType < 0)
		iPluginType = 0;
	m_ui.PluginTypeComboBox->setCurrentIndex(iPluginType);
	m_ui.PluginPathComboBox->setEditText(QString());

	choosePluginType(iPluginType);

	// Done. Restart clean.
	m_iDirtyCount = 0;
	stabilizeForm();
}


// Retrieve the editing options, if the case arises.
qtractorOptions *qtractorOptionsForm::options (void) const
{
	return m_pOptions;
}


// Accept settings (OK button slot).
void qtractorOptionsForm::accept (void)
{
	// Save options...
	if (m_iDirtyCount > 0) {
		// Audio options...
		int iFormat	= m_ui.AudioCaptureTypeComboBox->itemData(
			m_ui.AudioCaptureTypeComboBox->currentIndex()).toInt();
		const qtractorAudioFileFactory::FileFormat *pFormat
			= qtractorAudioFileFactory::formats().at(iFormat);
		m_pOptions->sAudioCaptureExt     = pFormat->ext;
		m_pOptions->iAudioCaptureType    = pFormat->data;
		m_pOptions->iAudioCaptureFormat  = m_ui.AudioCaptureFormatComboBox->currentIndex();
		m_pOptions->iAudioCaptureQuality = m_ui.AudioCaptureQualitySpinBox->value();
		m_pOptions->iAudioResampleType   = m_ui.AudioResampleTypeComboBox->currentIndex();
		m_pOptions->bAudioAutoTimeStretch = m_ui.AudioAutoTimeStretchCheckBox->isChecked();
		m_pOptions->bAudioQuickSeek      = m_ui.AudioQuickSeekCheckBox->isChecked();
		m_pOptions->bAudioPlayerBus      = m_ui.AudioPlayerBusCheckBox->isChecked();
		// Audio metronome options.
		m_pOptions->bAudioMetronome      = m_ui.AudioMetronomeCheckBox->isChecked();
		m_pOptions->sMetroBarFilename    = m_ui.MetroBarFilenameComboBox->currentText();
		m_pOptions->sMetroBeatFilename   = m_ui.MetroBeatFilenameComboBox->currentText();
		m_pOptions->bAudioMetroBus       = m_ui.AudioMetroBusCheckBox->isChecked();
		// MIDI options...
		m_pOptions->iMidiCaptureFormat   = m_ui.MidiCaptureFormatComboBox->currentIndex();
		m_pOptions->iMidiCaptureQuantize = m_ui.MidiCaptureQuantizeComboBox->currentIndex();
		m_pOptions->bMidiControlBus      = m_ui.MidiControlBusCheckBox->isChecked();
		// MIDI metronome options.
		m_pOptions->bMidiMetronome       = m_ui.MidiMetronomeCheckBox->isChecked();
		m_pOptions->iMetroChannel        = m_ui.MetroChannelSpinBox->value() - 1;
		m_pOptions->iMetroBarNote        = m_ui.MetroBarNoteComboBox->currentIndex();
		m_pOptions->iMetroBarVelocity    = m_ui.MetroBarVelocitySpinBox->value();
		m_pOptions->iMetroBarDuration    = m_ui.MetroBarDurationSpinBox->value();
		m_pOptions->iMetroBeatNote       = m_ui.MetroBeatNoteComboBox->currentIndex();
		m_pOptions->iMetroBeatVelocity   = m_ui.MetroBeatVelocitySpinBox->value();
		m_pOptions->iMetroBeatDuration   = m_ui.MetroBeatDurationSpinBox->value();
		m_pOptions->bMidiMetroBus        = m_ui.MidiMetroBusCheckBox->isChecked();
		// Display options...
		m_pOptions->bConfirmRemove       = m_ui.ConfirmRemoveCheckBox->isChecked();
		m_pOptions->bStdoutCapture       = m_ui.StdoutCaptureCheckBox->isChecked();
		m_pOptions->bCompletePath        = m_ui.CompletePathCheckBox->isChecked();
		m_pOptions->bPeakAutoRemove      = m_ui.PeakAutoRemoveCheckBox->isChecked();
		m_pOptions->bKeepToolsOnTop      = m_ui.KeepToolsOnTopCheckBox->isChecked();
		m_pOptions->bTrackViewDropSpan   = m_ui.TrackViewDropSpanCheckBox->isChecked();
		m_pOptions->iMaxRecentFiles      = m_ui.MaxRecentFilesSpinBox->value();
		m_pOptions->iDisplayFormat       = m_ui.DisplayFormatComboBox->currentIndex();
		// Plugin paths...
		m_pOptions->iPluginType          = m_ui.PluginTypeComboBox->currentIndex() + 1;
		m_pOptions->ladspaPaths          = m_ladspaPaths;
		m_pOptions->dssiPaths            = m_dssiPaths;
		m_pOptions->vstPaths             = m_vstPaths;
		// Messages options...
		m_pOptions->sMessagesFont        = m_ui.MessagesFontTextLabel->font().toString();
		m_pOptions->bMessagesLimit       = m_ui.MessagesLimitCheckBox->isChecked();
		m_pOptions->iMessagesLimitLines  = m_ui.MessagesLimitLinesSpinBox->value();
		// Reset dirty flag.
		m_iDirtyCount = 0;
	}

	// Save other conveniency options...
	m_pOptions->saveComboBoxHistory(m_ui.MetroBarFilenameComboBox);
	m_pOptions->saveComboBoxHistory(m_ui.MetroBeatFilenameComboBox);
	m_pOptions->saveComboBoxHistory(m_ui.PluginPathComboBox);

	// Just go with dialog acceptance
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void qtractorOptionsForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			tr("Apply"), tr("Discard"), tr("Cancel"))) {
		case 0:     // Apply...
			accept();
			return;
		case 1:     // Discard
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Dirty up settings.
void qtractorOptionsForm::changed (void)
{
	m_iDirtyCount++;
	stabilizeForm();
}


// Choose audio metronome filenames.
void qtractorOptionsForm::chooseMetroBarFilename (void)
{
	QString sFilename = getOpenAudioFileName(
		tr("Metronome Bar Audio File"),
		m_ui.MetroBarFilenameComboBox->currentText());

	if (sFilename.isEmpty())
		return;

	m_ui.MetroBarFilenameComboBox->setEditText(sFilename);

	changed();
}

void qtractorOptionsForm::chooseMetroBeatFilename (void)
{
	QString sFilename = getOpenAudioFileName(
		tr("Metronome Beat Audio File"),
		m_ui.MetroBeatFilenameComboBox->currentText());

	if (sFilename.isEmpty())
		return;

	m_ui.MetroBeatFilenameComboBox->setEditText(sFilename);

	changed();
}


// The metronome note names changer.
void qtractorOptionsForm::updateMetroNoteNames (void)
{
	// Save current selection...
	int iOldBarNote  = m_ui.MetroBarNoteComboBox->currentIndex();
	int iOldBeatNote = m_ui.MetroBeatNoteComboBox->currentIndex();

	// Populate the Metronome notes.
	m_ui.MetroBarNoteComboBox->clear();
	m_ui.MetroBeatNoteComboBox->clear();
	bool bDrums = (m_ui.MetroChannelSpinBox->value() == 10);
	QStringList items;
	const QString sItem("%1 (%2)");
	for (int i = 0; i < 128; ++i) {
		items.append(sItem
			.arg(qtractorMidiEditor::defaultNoteName(i, bDrums)).arg(i));
	}
	m_ui.MetroBarNoteComboBox->insertItems(0, items);
	m_ui.MetroBeatNoteComboBox->insertItems(0, items);

	// Restore old selection...
	m_ui.MetroBarNoteComboBox->setCurrentIndex(iOldBarNote);
	m_ui.MetroBeatNoteComboBox->setCurrentIndex(iOldBeatNote);

	changed();
}


// Change plugin type.
void qtractorOptionsForm::choosePluginType ( int iPluginType )
{
	if (m_pOptions == NULL)
		return;

	QStringList paths;
	qtractorPluginType::Hint typeHint
		= qtractorPluginType::hintFromText(
			m_ui.PluginTypeComboBox->itemText(iPluginType));
	switch (typeHint) {
	case qtractorPluginType::Ladspa:
		paths = m_ladspaPaths;
		break;
	case qtractorPluginType::Dssi:
		paths = m_dssiPaths;
		break;
	case qtractorPluginType::Vst:
		paths = m_vstPaths;
		break;
	default:
		break;
	}

	m_ui.PluginPathListWidget->clear();
	QStringListIterator iter(paths);
	while (iter.hasNext())
		m_ui.PluginPathListWidget->addItem(iter.next());

	selectPluginPath();
	stabilizeForm();
}


// Change plugin path.
void qtractorOptionsForm::changePluginPath ( const QString& /*sPluginPath*/ )
{
	selectPluginPath();
	stabilizeForm();
}


// Browse for plugin path.
void qtractorOptionsForm::choosePluginPath (void)
{
	QString sPluginPath = QFileDialog::getExistingDirectory(
		this,                                  // Parent.
		tr("Plug-in Directory:"),              // Caption.
		m_ui.PluginPathComboBox->currentText() // Start here.
	);

	if (!sPluginPath.isEmpty()) {
		m_ui.PluginPathComboBox->setEditText(sPluginPath);
		m_ui.PluginPathComboBox->setFocus();
	}

	selectPluginPath();
	stabilizeForm();
}


// Add chosen plugin path.
void qtractorOptionsForm::addPluginPath (void)
{
	const QString& sPluginPath = m_ui.PluginPathComboBox->currentText();
	if (sPluginPath.isEmpty())
		return;

	if (!QDir(sPluginPath).exists())
		return;

	qtractorPluginType::Hint typeHint
		= qtractorPluginType::hintFromText(
			m_ui.PluginTypeComboBox->currentText());
	switch (typeHint) {
	case qtractorPluginType::Ladspa:
		m_ladspaPaths.append(sPluginPath);
		break;
	case qtractorPluginType::Dssi:
		m_dssiPaths.append(sPluginPath);
		break;
	case qtractorPluginType::Vst:
		m_vstPaths.append(sPluginPath);
		break;
	default:
		return;
	}

	m_ui.PluginPathListWidget->addItem(sPluginPath);
	m_ui.PluginPathListWidget->setCurrentRow(
		m_ui.PluginPathListWidget->count() - 1);

	int i = m_ui.PluginPathComboBox->findText(sPluginPath);
	if (i >= 0)
		m_ui.PluginPathComboBox->removeItem(i);
	m_ui.PluginPathComboBox->insertItem(0, sPluginPath);
	m_ui.PluginPathComboBox->setEditText(QString());

	m_ui.PluginPathListWidget->setFocus();

	selectPluginPath();
	changed();
}


// Select current plugin path.
void qtractorOptionsForm::selectPluginPath (void)
{
	int iPluginPath = m_ui.PluginPathListWidget->currentRow();

	m_ui.PluginPathRemoveToolButton->setEnabled(iPluginPath >= 0);
	m_ui.PluginPathUpToolButton->setEnabled(iPluginPath > 0);
	m_ui.PluginPathDownToolButton->setEnabled(iPluginPath >= 0
		&& iPluginPath < m_ui.PluginPathListWidget->count() - 1);
}


// Remove current plugin path.
void qtractorOptionsForm::removePluginPath (void)
{
	int iPluginPath = m_ui.PluginPathListWidget->currentRow();
	if (iPluginPath < 0)
		return;

	qtractorPluginType::Hint typeHint
		= qtractorPluginType::hintFromText(
			m_ui.PluginTypeComboBox->currentText());
	switch (typeHint) {
	case qtractorPluginType::Ladspa:
		m_ladspaPaths.removeAt(iPluginPath);
		break;
	case qtractorPluginType::Dssi:
		m_dssiPaths.removeAt(iPluginPath);
		break;
	case qtractorPluginType::Vst:
		m_vstPaths.removeAt(iPluginPath);
		break;
	default:
		return;
	}

	QListWidgetItem *pItem = m_ui.PluginPathListWidget->takeItem(iPluginPath);
	if (pItem)
		delete pItem;

	selectPluginPath();
	changed();
}


// Move up plugin path on search order.
void qtractorOptionsForm::moveUpPluginPath (void)
{
	int iPluginPath = m_ui.PluginPathListWidget->currentRow();
	if (iPluginPath < 1)
		return;

	QString sPluginPath;
	qtractorPluginType::Hint typeHint
		= qtractorPluginType::hintFromText(
			m_ui.PluginTypeComboBox->currentText());
	switch (typeHint) {
	case qtractorPluginType::Ladspa:
		sPluginPath = m_ladspaPaths.takeAt(iPluginPath);
		m_ladspaPaths.insert(iPluginPath - 1, sPluginPath);
		break;
	case qtractorPluginType::Dssi:
		sPluginPath = m_dssiPaths.takeAt(iPluginPath);
		m_dssiPaths.insert(iPluginPath - 1, sPluginPath);
		break;
	case qtractorPluginType::Vst:
		sPluginPath = m_vstPaths.takeAt(iPluginPath);
		m_vstPaths.insert(iPluginPath - 1, sPluginPath);
		break;
	default:
		return;
	}

	QListWidgetItem *pItem = m_ui.PluginPathListWidget->takeItem(iPluginPath);
	if (pItem) {
#if QT_VERSION >= 0x040200
		pItem->setSelected(false);
#else
		m_ui.PluginPathListWidget->setItemSelected(pItem, false);
#endif
		m_ui.PluginPathListWidget->insertItem(iPluginPath - 1, pItem);
#if QT_VERSION >= 0x040200
		pItem->setSelected(true);
#else
		m_ui.PluginPathListWidget->setItemSelected(pItem, true);
#endif
		m_ui.PluginPathListWidget->setCurrentItem(pItem);
	}

	selectPluginPath();
	changed();
}


// Move down plugin path on search order.
void qtractorOptionsForm::moveDownPluginPath (void)
{
	int iPluginPath = m_ui.PluginPathListWidget->currentRow();
	if (iPluginPath >= m_ui.PluginPathListWidget->count() - 1)
		return;

	QString sPluginPath;
	qtractorPluginType::Hint typeHint
		= qtractorPluginType::hintFromText(
			m_ui.PluginTypeComboBox->currentText());
	switch (typeHint) {
	case qtractorPluginType::Ladspa:
		sPluginPath = m_ladspaPaths.takeAt(iPluginPath);
		m_ladspaPaths.insert(iPluginPath + 1, sPluginPath);
		break;
	case qtractorPluginType::Dssi:
		sPluginPath = m_dssiPaths.takeAt(iPluginPath);
		m_dssiPaths.insert(iPluginPath + 1, sPluginPath);
		break;
	case qtractorPluginType::Vst:
		sPluginPath = m_vstPaths.takeAt(iPluginPath);
		m_vstPaths.insert(iPluginPath + 1, sPluginPath);
		break;
	default:
		return;
	}

	QListWidgetItem *pItem = m_ui.PluginPathListWidget->takeItem(iPluginPath);
	if (pItem) {
#if QT_VERSION >= 0x040200
		pItem->setSelected(false);
#else
		m_ui.PluginPathListWidget->setItemSelected(pItem, false);
#endif
		m_ui.PluginPathListWidget->insertItem(iPluginPath + 1, pItem);
#if QT_VERSION >= 0x040200
		pItem->setSelected(true);
#else
		m_ui.PluginPathListWidget->setItemSelected(pItem, true);
#endif
		m_ui.PluginPathListWidget->setCurrentItem(pItem);
	}

	selectPluginPath();
	changed();
}


// The messages font selection dialog.
void qtractorOptionsForm::chooseMessagesFont (void)
{
	bool  bOk  = false;
	QFont font = QFontDialog::getFont(&bOk,
		m_ui.MessagesFontTextLabel->font(), this);
	if (bOk) {
		m_ui.MessagesFontTextLabel->setFont(font);
		m_ui.MessagesFontTextLabel->setText(
			font.family() + " " + QString::number(font.pointSize()));
		changed();
	}
}


// Stabilize current form state.
void qtractorOptionsForm::stabilizeForm (void)
{
	m_ui.MessagesLimitLinesSpinBox->setEnabled(
		m_ui.MessagesLimitCheckBox->isChecked());

	// Audio options validy check...
	int iIndex  = m_ui.AudioCaptureTypeComboBox->currentIndex();
	int iFormat	= m_ui.AudioCaptureTypeComboBox->itemData(iIndex).toInt();
	const qtractorAudioFileFactory::FileFormat *pFormat
		= qtractorAudioFileFactory::formats().at(iFormat);

	bool bSndFile
		= (pFormat && pFormat->type == qtractorAudioFileFactory::SndFile);
	m_ui.AudioCaptureFormatTextLabel->setEnabled(bSndFile);
	m_ui.AudioCaptureFormatComboBox->setEnabled(bSndFile);

	bool bVorbisFile
		= (pFormat && pFormat->type == qtractorAudioFileFactory::VorbisFile);
	m_ui.AudioCaptureQualityTextLabel->setEnabled(bVorbisFile);
	m_ui.AudioCaptureQualitySpinBox->setEnabled(bVorbisFile);

	bool bValid = (m_iDirtyCount > 0);
	if (bValid) {
		iFormat = m_ui.AudioCaptureFormatComboBox->currentIndex();
		bValid  = qtractorAudioFileFactory::isValidFormat(pFormat, iFormat);
	}

	bool bAudioMetronome = m_ui.AudioMetronomeCheckBox->isChecked();
	m_ui.MetroBarFilenameTextLabel->setEnabled(bAudioMetronome);
	m_ui.MetroBarFilenameComboBox->setEnabled(bAudioMetronome);
	m_ui.MetroBarFilenameToolButton->setEnabled(bAudioMetronome);
	m_ui.MetroBeatFilenameTextLabel->setEnabled(bAudioMetronome);
	m_ui.MetroBeatFilenameComboBox->setEnabled(bAudioMetronome);
	m_ui.MetroBeatFilenameToolButton->setEnabled(bAudioMetronome);
	m_ui.AudioMetroBusCheckBox->setEnabled(bAudioMetronome);

	bool bMidiMetronome = m_ui.MidiMetronomeCheckBox->isChecked();
	m_ui.MetroChannelTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroChannelSpinBox->setEnabled(bMidiMetronome);
	m_ui.MetroBarNoteTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBarNoteComboBox->setEnabled(bMidiMetronome);
	m_ui.MetroBarVelocityTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBarVelocitySpinBox->setEnabled(bMidiMetronome);
	m_ui.MetroBarDurationTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBarDurationSpinBox->setEnabled(bMidiMetronome);
	m_ui.MetroBeatNoteTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBeatNoteComboBox->setEnabled(bMidiMetronome);
	m_ui.MetroBeatVelocityTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBeatVelocitySpinBox->setEnabled(bMidiMetronome);
	m_ui.MetroBeatDurationTextLabel->setEnabled(bMidiMetronome);
	m_ui.MetroBeatDurationSpinBox->setEnabled(bMidiMetronome);
	m_ui.MidiMetroBusCheckBox->setEnabled(bMidiMetronome);

	const QString& sPluginPath = m_ui.PluginPathComboBox->currentText();
	m_ui.PluginPathAddToolButton->setEnabled(
		!sPluginPath.isEmpty() && QDir(sPluginPath).exists()
		&& m_ui.PluginPathListWidget->findItems(
			sPluginPath, Qt::MatchExactly).isEmpty());
		
	m_ui.OkPushButton->setEnabled(bValid);
}


// Browse for an existing audio filename.
QString qtractorOptionsForm::getOpenAudioFileName (
	const QString& sTitle, const QString& sFilename )
{
	// Ask for the filename to open...
	return QFileDialog::getOpenFileName(
		this, sTitle, sFilename, qtractorAudioFileFactory::filters());
}


// end of qtractorOptionsForm.cpp

