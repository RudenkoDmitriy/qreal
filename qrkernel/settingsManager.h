#pragma once

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QVariant>

#include "kernelDeclSpec.h"

namespace qReal {

/// Singleton class that allows to change settings in run-time
/// (replaces QSettings). Purpose of this class is to allow two instances
/// of an application coexist without changing each other's settings,
/// by storing settings separately in memory for each instance and syncing
/// them only on start/exit.
class QRKERNEL_EXPORT SettingsManager : public QObject
{
	Q_OBJECT

public:
	/// Get value associated with given key from settings.
	/// @param key Parameter name.
	/// @returns Variant with parameter value.
	static QVariant value(const QString &key);

	/// Get value associated with given key from settings with given default value.
	/// @param key Parameter name.
	/// @param defaultValue Default value, used when parameter not found.
	/// @returns Variant with parameter value.
	static QVariant value(const QString &key, const QVariant &defaultValue);

	/// Set value associated with given key.
	/// @param key Parameter name.
	/// @param value Parameter value.
	static void setValue(const QString &key, const QVariant &value);

	/// Removes all entries in persistent external storage
	static void clearSettings();

	/// Returns an instance of a singleton.
	static SettingsManager *instance();

	/// Saves settings into persistent external storage (for example, Windows
	/// registry), making them available to new instances of an application.
	void saveData();

	/// Saves settings into selected file with name fileNameForExport.
	void saveSettings(const QString &fileNameForExport);

	/// Loads settings from persistent external storage into SettingsManager.
	void load();

	/// Merges settings from the given file in INI format.
	void loadSettings(const QString &fileNameForImport);

	/// Merges default settings from the given file in INI format.
	static void loadDefaultSettings(const QString &filePath);

signals:
	/// Emitted each time when settings with the given key were modified.
	/// For connection instance() method can be useful.
	void settingsChanged(const QString &name, const QVariant &oldValue, const QVariant &newValue);

private:
	/// Private constructor.
	SettingsManager();
	~SettingsManager();

	void set(const QString &name, const QVariant &value);
	QVariant get(const QString &key, const QVariant &defaultValue = QVariant()) const;

	/// Merges settings from the given file in INI format into the given map.
	void mergeSettings(const QString &fileNameForImport, QHash<QString, QVariant> &target);

	void initDefaultValues();

	/// Singleton sole instance.
	static SettingsManager *mInstance;

	/// In-memory settings storage.
	QHash<QString, QVariant> mData;
	QHash<QString, QVariant> mDefaultValues;
	/// Persistent settings storage.
	QSettings mSettings;
};

}
