#include "nxtAdditionalPreferences.h"
#include "ui_nxtAdditionalPreferences.h"

#include <qrkernel/settingsManager.h>
#include <plugins/robots/thirdparty/qextserialport/src/qextserialenumerator.h>

using namespace nxt;
using namespace qReal;

NxtAdditionalPreferences::NxtAdditionalPreferences(const QString &realRobotName, QWidget *parent)
	: AdditionalPreferences(parent)
	, mUi(new Ui::NxtAdditionalPreferences)
	, mBluetoothRobotName(realRobotName)
{
	mUi->setupUi(this);
	connect(mUi->manualComPortCheckbox, &QCheckBox::toggled
			, this, &NxtAdditionalPreferences::manualComPortCheckboxChecked);
}

NxtAdditionalPreferences::~NxtAdditionalPreferences()
{
	delete mUi;
}

void NxtAdditionalPreferences::save()
{
	SettingsManager::setValue("NxtBluetoothPortName", selectedPortName());
	SettingsManager::setValue("NxtManualComPortCheckboxChecked", mUi->manualComPortCheckbox->isChecked());
	emit settingsChanged();
}

void NxtAdditionalPreferences::restoreSettings()
{
	QList<QextPortInfo> const ports = QextSerialEnumerator::getPorts();
	const QString defaultPortName = SettingsManager::value("NxtBluetoothPortName").toString();
	mUi->comPortComboBox->clear();

	for (const QextPortInfo &info : ports) {
		const QRegExp portNameRegexp("COM\\d+", Qt::CaseInsensitive);
		if (portNameRegexp.indexIn(info.portName) != -1) {
			const QString portName = portNameRegexp.cap();
			mUi->comPortComboBox->addItem(portName);
		}
	}

	const int defaultIndex = mUi->comPortComboBox->findText(defaultPortName);
	if (defaultIndex != -1) {
		mUi->comPortComboBox->setCurrentIndex(defaultIndex);
	}

	if (mUi->comPortComboBox->count() == 0) {
		mUi->comPortComboBox->hide();
		mUi->comPortLabel->hide();
		mUi->manualComPortCheckbox->hide();
		mUi->noComPortsFoundLabel->show();
		mUi->directInputComPortLabel->show();
		mUi->directInputComPortLineEdit->show();
		mUi->directInputComPortLineEdit->setText(defaultPortName);
	} else {
		mUi->comPortComboBox->show();
		mUi->comPortLabel->show();
		mUi->manualComPortCheckbox->show();
		mUi->directInputComPortLabel->hide();
		mUi->directInputComPortLineEdit->hide();
		mUi->noComPortsFoundLabel->hide();
		mUi->manualComPortCheckbox->setChecked(false);
		mUi->manualComPortCheckbox->setChecked(SettingsManager::value("NxtManualComPortCheckboxChecked").toBool());
	}
}

void NxtAdditionalPreferences::onRobotModelChanged(kitBase::robotModel::RobotModelInterface * const robotModel)
{
	mUi->bluetoothSettingsGroupBox->setVisible(robotModel->name() == mBluetoothRobotName);
}

void NxtAdditionalPreferences::manualComPortCheckboxChecked(bool state)
{
	const QString defaultPortName = SettingsManager::value("NxtBluetoothPortName").toString();

	if (state) {
		mUi->comPortComboBox->hide();
		mUi->comPortLabel->hide();
		mUi->directInputComPortLabel->show();
		mUi->directInputComPortLineEdit->show();
		mUi->directInputComPortLineEdit->setText(defaultPortName);
	} else {
		mUi->comPortComboBox->show();
		mUi->comPortLabel->show();
		mUi->directInputComPortLabel->hide();
		mUi->directInputComPortLineEdit->hide();
		mUi->noComPortsFoundLabel->hide();
	}
}

QString NxtAdditionalPreferences::selectedPortName() const
{
	return mUi->comPortComboBox->isVisible()
			? mUi->comPortComboBox->currentText()
			: mUi->directInputComPortLineEdit->text();
}
