#pragma once

#include <QtWidgets/QApplication>

#include <qrgui/plugins/toolPluginInterface/pluginConfigurator.h>
#include <qrgui/textEditor/languageInfo.h>
#include <qrrepo/repoApi.h>
#include <kitBase/kitPluginInterface.h>

#include "robotsGeneratorDeclSpec.h"
#include "masterGeneratorBase.h"

namespace utils {
class ParserErrorReporter;
}

namespace generatorBase {

class ROBOTS_GENERATOR_EXPORT RobotsGeneratorPluginBase : public QObject, public kitBase::KitPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(kitBase::KitPluginInterface)

public:
	RobotsGeneratorPluginBase();
	~RobotsGeneratorPluginBase() override;

	void init(const kitBase::KitPluginConfigurator &configurator) override;

	QString friendlyKitName() const override;

protected slots:
	/// Calls code generator. Returns true if operation was successful.
	/// @param openTab If true after code generation a tab with generated code will be opened.
	virtual bool generateCode(bool openTab = true);

	/// Changes path to code source file and regenerate necessary extra files.
	void regenerateCode(const qReal::Id &diagram, const QFileInfo &oldFileInfo, const QFileInfo &newFileInfo);

	/// Adds new path to another code source file for chosen diagram
	void addNewCode(const qReal::Id &diagram, const QFileInfo &fileInfo);

	/// Remove diagram with paths to code source files.
	void removeDiagram(const qReal::Id &diagram);

	void removeCode(const QFileInfo &fileInfo);

protected:
	/// Override must return a link to concrete master generator instance for
	/// developped plugin. Caller takes ownership so override may forget about it.
	virtual MasterGeneratorBase *masterGenerator() = 0;

	virtual void regenerateExtraFiles(const QFileInfo &newFileInfo) = 0;
	QFileInfo srcPath();
	virtual QString defaultFilePath(const QString &project) const;
	QFileInfo generateCodeForProcessing();
	virtual QString generatorName() const;

	/// Returns an information about the language code on which will be generated;
	/// this information will be used by text editors when user will be edit the generated code.
	virtual qReal::text::LanguageInfo language() const = 0;

	/// Returns default name for generated file.
	virtual QString defaultProjectName() const;

	virtual bool canGenerateTo(const QString &project);

	/// Can be overrided to show or hide concrete actions on toolbars.
	/// Default implemetation shows or hides all customActions() only in case if plugin exports one robot model.
	virtual void onCurrentRobotModelChanged(kitBase::robotModel::RobotModelInterface &model);

	/// Can be overrided to enable or disable concrete actions on toolbars.
	/// Default implemetation enables all customActions() if current diagram is non-null and disables all else.
	virtual void onCurrentDiagramChanged(const qReal::Id &id);

	/// Interface of MainWindow
	qReal::gui::MainWindowInterpretersInterface *mMainWindowInterface;  // Does not have ownership

	/// Interface of project manager (allows to perform open/save activities)
	qReal::ProjectManagementInterface *mProjectManager; // Does not have ownership

	/// Control interface of the repository
	const qrRepo::RepoApi *mRepo;  // Does not have ownership

	const kitBase::robotModel::RobotModelManagerInterface *mRobotModelManager;
	qrtext::LanguageToolboxInterface *mTextLanguage;  // Does not have ownership

	QList<qReal::HotKeyActionInfo> mHotKeyActionInfos;
	qReal::SystemEvents *mSystemEvents; // Does not have ownership
	qReal::TextManagerInterface *mTextManager;
	QMultiHash<qReal::Id, QFileInfo> mCodePath;

	QScopedPointer<utils::ParserErrorReporter> mParserErrorReporter;
};

}
