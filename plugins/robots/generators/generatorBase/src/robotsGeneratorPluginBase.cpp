#include "generatorBase/robotsGeneratorPluginBase.h"

#include <QtCore/QDateTime>

#include <qrutils/inFile.h>
#include <qrutils/nameNormalizer.h>
#include <qrutils/parserErrorReporter.h>
#include <qrgui/textEditor/qscintillaTextEdit.h>

using namespace generatorBase;
using namespace qReal;
using namespace utils;

/// If file info creation and modification timestamps differ less than on this value it is considered
/// that file was created and filled at the same time.
const int maxTimestampsDifference = 3000;

RobotsGeneratorPluginBase::RobotsGeneratorPluginBase()
{
}

RobotsGeneratorPluginBase::~RobotsGeneratorPluginBase()
{
}

QString RobotsGeneratorPluginBase::defaultFilePath(const QString &projectName) const
{
	return projectName;
}

QString RobotsGeneratorPluginBase::generatorName() const
{
	return QString();
}

QString RobotsGeneratorPluginBase::defaultProjectName() const
{
	const QString filePath = mProjectManager->saveFilePath();
	return filePath.isEmpty() ? "example" : QFileInfo(filePath).baseName();
}

bool RobotsGeneratorPluginBase::canGenerateTo(const QString &project)
{
	const QFileInfo fileInfo(QApplication::applicationDirPath() + "/" + defaultFilePath(project));
	const int difference = fileInfo.lastModified().toMSecsSinceEpoch() - fileInfo.created().toMSecsSinceEpoch();
	return !fileInfo.exists() || difference < maxTimestampsDifference;
}

void RobotsGeneratorPluginBase::onCurrentRobotModelChanged(kitBase::robotModel::RobotModelInterface &model)
{
	if (robotModels().count() == 1) {
		kitBase::robotModel::RobotModelInterface * const ourModel = robotModels()[0];;
		for (const ActionInfo &action : customActions()) {
			if (action.isAction()) {
				action.action()->setVisible(ourModel == &model);
			} else {
				action.menu()->setVisible(ourModel == &model);
			}
		}
	}
}

void RobotsGeneratorPluginBase::onCurrentDiagramChanged(const Id &id)
{
	for (const ActionInfo &action : customActions()) {
		const bool enabled = !id.isNull();
		if (action.isAction()) {
			action.action()->setEnabled(enabled);
		} else {
			action.menu()->setEnabled(enabled);
		}
	}
}

QFileInfo RobotsGeneratorPluginBase::srcPath()
{
	const Id &activeDiagram = mMainWindowInterface->activeDiagram();

	int exampleNumber = 0;
	QString projectName;

	do {
		projectName = NameNormalizer::normalizeStrongly(defaultProjectName(), false)
				+ (exampleNumber > 0 ? QString::number(exampleNumber) : "");
		++exampleNumber;
	} while (!canGenerateTo(projectName));

	QFileInfo fileInfo = QFileInfo(QApplication::applicationDirPath() + "/" + defaultFilePath(projectName));
	QList<QFileInfo> const pathsList = mCodePath.values(activeDiagram);

	if (!pathsList.isEmpty()) {
		for (const QFileInfo &path : pathsList) {
			if (mTextManager->isDefaultPath(path.absoluteFilePath())
				&& !mTextManager->isModifiedEver(path.absoluteFilePath())
				&& !mTextManager->generatorName(path.absoluteFilePath()).compare(generatorName()))
			{
				fileInfo = path;
				break;
			}
		}
	}

	return fileInfo;
}

QFileInfo RobotsGeneratorPluginBase::generateCodeForProcessing()
{
	QFileInfo fileInfo;
	const Id &activeDiagram = mMainWindowInterface->activeDiagram();

	if (!activeDiagram.isNull()) {
		if (generateCode(false)) {
			foreach (const QFileInfo &path, mCodePath.values(activeDiagram)) {
				if (mTextManager->isDefaultPath(path.absoluteFilePath())
					&& (!mTextManager->isModifiedEver(path.absoluteFilePath()))
					&& !mTextManager->generatorName(path.absoluteFilePath()).compare(generatorName())) {
					fileInfo = path;
					break;
				}
			}
		} else {
			return QFileInfo();
		}
	} else if (auto code = dynamic_cast<text::QScintillaTextEdit *>(mMainWindowInterface->currentTab())) {
		fileInfo = QFileInfo(mTextManager->path(code));
		mTextManager->saveText(false);
	}

	return fileInfo;
}

void RobotsGeneratorPluginBase::init(const kitBase::KitPluginConfigurator &configurator)
{
	mProjectManager = &configurator.qRealConfigurator().projectManager();
	mSystemEvents = &configurator.qRealConfigurator().systemEvents();
	mTextManager = &configurator.qRealConfigurator().textManager();

	mMainWindowInterface = &configurator.qRealConfigurator().mainWindowInterpretersInterface();
	mRepo = dynamic_cast<const qrRepo::RepoApi *>(&configurator.qRealConfigurator().logicalModelApi().logicalRepoApi());
	mProjectManager = &configurator.qRealConfigurator().projectManager();
	mRobotModelManager = &configurator.robotModelManager();
	mTextLanguage = &configurator.textLanguage();

	mParserErrorReporter.reset(new ParserErrorReporter(*mTextLanguage, *mMainWindowInterface->errorReporter()
			, configurator.qRealConfigurator().logicalModelApi().editorManagerInterface()));

	connect(mSystemEvents, SIGNAL(codePathChanged(qReal::Id, QFileInfo, QFileInfo))
			, this, SLOT(regenerateCode(qReal::Id, QFileInfo, QFileInfo)));
	connect(mSystemEvents, SIGNAL(newCodeAppeared(qReal::Id, QFileInfo)), this, SLOT(addNewCode(qReal::Id, QFileInfo)));
	connect(mSystemEvents, SIGNAL(diagramClosed(qReal::Id)), this, SLOT(removeDiagram(qReal::Id)));
	connect(mSystemEvents, SIGNAL(codeTabClosed(QFileInfo)), this, SLOT(removeCode(QFileInfo)));

	connect(mRobotModelManager, &kitBase::robotModel::RobotModelManagerInterface::robotModelChanged
			, this, &RobotsGeneratorPluginBase::onCurrentRobotModelChanged);
	connect(mSystemEvents, &SystemEvents::activeTabChanged, this, &RobotsGeneratorPluginBase::onCurrentDiagramChanged);
}

QString RobotsGeneratorPluginBase::friendlyKitName() const
{
	// Kit friendly name is already defined in interpreter kit plugins.
	return QString();
}

bool RobotsGeneratorPluginBase::generateCode(bool openTab)
{
	mProjectManager->save();
	mMainWindowInterface->errorReporter()->clearErrors();

	MasterGeneratorBase * const generator = masterGenerator();
	const QFileInfo path = srcPath();

	generator->initialize();
	generator->setProjectDir(path);

	const QString generatedSrcPath = generator->generate(language().indent());

	if (mMainWindowInterface->errorReporter()->wereErrors()) {
		delete generator;
		return false;
	}

	const Id activeDiagram = mMainWindowInterface->activeDiagram();

	const QString generatedCode = utils::InFile::readAll(generatedSrcPath);
	if (!generatedCode.isEmpty()) {
		mTextManager->showInTextEditor(path, generatorName(), language());
	}

	if (!openTab) {
		mMainWindowInterface->activateItemOrDiagram(activeDiagram);
	}

	delete generator;
	return true;
}

void RobotsGeneratorPluginBase::regenerateCode(const qReal::Id &diagram
		, const QFileInfo &oldFileInfo
		, const QFileInfo &newFileInfo)
{
	if (!oldFileInfo.completeSuffix().compare(language().extension)) {
		mCodePath.remove(diagram, oldFileInfo);
		mCodePath.insert(diagram, newFileInfo);
		regenerateExtraFiles(newFileInfo);
	}
}

void RobotsGeneratorPluginBase::addNewCode(const Id &diagram, const QFileInfo &fileInfo)
{
	mCodePath.insert(diagram, fileInfo);
}

void RobotsGeneratorPluginBase::removeDiagram(const qReal::Id &diagram)
{
	mCodePath.remove(diagram);
}

void RobotsGeneratorPluginBase::removeCode(const QFileInfo &fileInfo)
{
	const Id &diagram = mCodePath.key(fileInfo);
	mCodePath.remove(diagram, fileInfo);
}
