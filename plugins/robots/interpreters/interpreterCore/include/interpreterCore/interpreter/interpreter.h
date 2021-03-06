#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QAction>

#include <qrgui/plugins/toolPluginInterface/usedInterfaces/projectManagementInterface.h>
#include <qrgui/plugins/toolPluginInterface/usedInterfaces/graphicalModelAssistInterface.h>
#include <qrgui/plugins/toolPluginInterface/usedInterfaces/logicalModelAssistInterface.h>
#include <qrutils/watchListWindow.h>
#include <qrutils/interpreter/thread.h>
#include <qrtext/languageToolboxInterface.h>

#include <kitBase/robotModel/robotModelManagerInterface.h>
#include <kitBase/devicesConfigurationProvider.h>

#include "interpreterCore/interpreter/details/blocksTable.h"
#include "interpreterCore/interpreter/details/sensorVariablesUpdater.h"
#include "interpreterCore/interpreter/details/autoconfigurer.h"

#include "interpreterCore/interpreter/interpreterInterface.h"

namespace interpreterCore {
namespace interpreter {

/// Interprets robot diagram by executing blocks and sending commands to robot model. Manages models, connection,
/// threads, parser, can automatically configure robot by used blocks on diagram. It is the main class for
/// all interpretation subsystem.
class Interpreter : public InterpreterInterface, public kitBase::DevicesConfigurationProvider
{
	Q_OBJECT

public:
	/// Constructor,
	/// @param graphicalModelApi - reference to graphical model, main source of information about interpreted diagram.
	/// @param logicalModelApi - reference to logical model, which contains properties of blocks.
	/// @param interpretersInterface - reference to QReal main window. Used to highlight executing blocks and so on.
	/// @param projectManager - class that manages opening/saving diagrams, needed here to receive notifications
	///        about opening a new save file.
	/// @param blocksFactoryManager - blocks factory, needed to create "code behind" objects for blocks on diagram.
	/// @param robotModelManager - reference to currently selected robot model.
	/// @param parser - parser that is used to analyze and evaluate textual expressions inside properties of blocks.
	/// @param connectToRobotAction - reference to action that connects to robot, interpreter manages its state
	///        depending on success or failure of its own connection attempts.
	Interpreter(const qReal::GraphicalModelAssistInterface &graphicalModelApi
			, qReal::LogicalModelAssistInterface &logicalModelApi
			, qReal::gui::MainWindowInterpretersInterface &interpretersInterface
			, const qReal::ProjectManagementInterface &projectManager
			, BlocksFactoryManagerInterface &blocksFactoryManager
			, const kitBase::robotModel::RobotModelManagerInterface &robotModelManager
			, qrtext::LanguageToolboxInterface &languageToolbox
			, QAction &connectToRobotAction
			);

	~Interpreter() override;

public slots:
	void connectToRobot() override;
	void interpret() override;
	void stopRobot() override;
	int timeElapsed() const override;

private slots:
	void threadStopped();
	void newThread(const qReal::Id &startBlockId);

	void connectedSlot(bool success, const QString &errorString);
	void devicesConfiguredSlot();

	/// Actions when robot disconnect
	void disconnectSlot();

private:
	enum InterpreterState {
		interpreting
		, waitingForDevicesConfiguredToLaunch
		, idle
	};

	void addThread(qReal::interpretation::Thread * const thread);

	void reportError(const QString &message);

	const qReal::GraphicalModelAssistInterface &mGraphicalModelApi;
	qReal::LogicalModelAssistInterface &mLogicalModelApi;
	qReal::gui::MainWindowInterpretersInterface &mInterpretersInterface;

	InterpreterState mState;
	quint64 mInterpretationStartedTimestamp;
	QList<qReal::interpretation::Thread *> mThreads;  // Has ownership
	const kitBase::robotModel::RobotModelManagerInterface &mRobotModelManager;
	details::BlocksTable *mBlocksTable;  // Has ownership

	/// Action responsible for the connection to the robot
	QAction &mActionConnectToRobot;

	details::SensorVariablesUpdater mSensorVariablesUpdater;
	details::Autoconfigurer mAutoconfigurer;

	/// Reference to a parser to be able to clear parser state when starting interpretation.
	qrtext::LanguageToolboxInterface &mLanguageToolbox;
};

}
}
