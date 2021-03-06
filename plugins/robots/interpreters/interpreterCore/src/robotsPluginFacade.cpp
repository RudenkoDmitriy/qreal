#include "interpreterCore/robotsPluginFacade.h"

#include <qrkernel/settingsManager.h>
#include <kitBase/robotModel/portInfo.h>

#include "src/coreBlocks/coreBlocksFactory.h"
#include "src/ui/robotsSettingsPage.h"
#include "interpreterCore/managers/paletteUpdateManager.h"
#include "interpreterCore/managers/kitAutoSwitcher.h"

using namespace interpreterCore;

RobotsPluginFacade::RobotsPluginFacade()
	: mInterpreter(nullptr)
	, mKitPluginManager("plugins/tools/kitPlugins")
	, mActionsManager(mKitPluginManager, mRobotModelManager)
	, mDockDevicesConfigurer(nullptr)
	, mGraphicsWatcherManager(nullptr)
{
	connect(&mRobotModelManager, &RobotModelManager::robotModelChanged
			, &mActionsManager, &ActionsManager::onRobotModelChanged);
}

RobotsPluginFacade::~RobotsPluginFacade()
{
	delete mInterpreter;
}

void RobotsPluginFacade::init(const qReal::PluginConfigurator &configurer)
{
	mRobotSettingsPage = new ui::RobotsSettingsPage(mKitPluginManager, mRobotModelManager);

	mDevicesConfigurationManager.reset(new DevicesConfigurationManager(
			configurer.graphicalModelApi()
			, configurer.logicalModelApi()
			, configurer.mainWindowInterpretersInterface()
			, configurer.systemEvents()
			));

	if (!selectKit(configurer)) {
		/// @todo Correctly handle unselected kit.
		return;
	}

	mParser.reset(new textLanguage::RobotsBlockParser(mRobotModelManager
			, [this]() { return mInterpreter ? mInterpreter->timeElapsed() : 0; }));

	kitBase::blocksBase::BlocksFactoryInterface * const coreFactory = new coreBlocks::CoreBlocksFactory();
	coreFactory->configure(configurer.graphicalModelApi()
			, configurer.logicalModelApi()
			, mRobotModelManager
			, *configurer.mainWindowInterpretersInterface().errorReporter()
			, *mParser
			);

	mBlocksFactoryManager.addFactory(coreFactory);

	interpreter::Interpreter *interpreter = new interpreter::Interpreter(
			configurer.graphicalModelApi()
			, configurer.logicalModelApi()
			, configurer.mainWindowInterpretersInterface()
			, configurer.projectManager()
			, mBlocksFactoryManager
			, mRobotModelManager
			, *mParser
			, mActionsManager.connectToRobotAction()
			);

	mInterpreter = interpreter;

	connect(&configurer.systemEvents(), &qReal::SystemEvents::closedMainWindow
			, mInterpreter, &interpreter::InterpreterInterface::stopRobot);
	connect(&mRobotModelManager, &RobotModelManager::robotModelChanged
			, mInterpreter, &interpreter::InterpreterInterface::stopRobot);

	initKitPlugins(configurer);

	initSensorWidgets();

	auto paletteUpdateManager = new PaletteUpdateManager(configurer.mainWindowInterpretersInterface()
			, mBlocksFactoryManager, this);
	connect(&mRobotModelManager, &RobotModelManager::robotModelChanged
			, paletteUpdateManager, &PaletteUpdateManager::updatePalette);
	mDevicesConfigurationManager->connectDevicesConfigurationProvider(interpreter);

	// It will subscribe to all signals itself and free memory too.
	new KitAutoSwitcher(configurer.projectManager(), configurer.logicalModelApi()
			, mBlocksFactoryManager, mKitPluginManager, mRobotModelManager, this);

	connectInterpreterToActions();

	connectEventsForKitPlugin();

	connect(&mActionsManager.robotSettingsAction(), &QAction::triggered
			, [=] () { configurer.mainWindowInterpretersInterface().openSettingsDialog(tr("Robots")); });
	connect(&configurer.systemEvents(), &qReal::SystemEvents::activeTabChanged
			, &mActionsManager, &ActionsManager::onActiveTabChanged);

	sync();
}

qReal::gui::PreferencesPage *RobotsPluginFacade::robotsSettingsPage() const
{
	return mRobotSettingsPage;
}

interpreterCore::Customizer &RobotsPluginFacade::customizer()
{
	return mCustomizer;
}

ActionsManager &RobotsPluginFacade::actionsManager()
{
	return mActionsManager;
}

QStringList RobotsPluginFacade::defaultSettingsFiles() const
{
	QStringList result = { ":/interpreterCoreDefaultSettings.ini" };
	for (const QString &kitId : mKitPluginManager.kitIds()) {
		for (kitBase::KitPluginInterface * const kit : mKitPluginManager.kitsById(kitId)) {
			const QString defaultSettings = kit->defaultSettingsFile();
			if (!defaultSettings.isEmpty()) {
				result << defaultSettings;
			}
		}
	}

	return result;
}

interpreter::InterpreterInterface &RobotsPluginFacade::interpreter() const
{
	return *mInterpreter;
}

void RobotsPluginFacade::connectInterpreterToActions()
{
	QObject::connect(
			&mActionsManager.runAction()
			, &QAction::triggered
			, mInterpreter
			, &interpreter::InterpreterInterface::interpret
			);

	QObject::connect(
			&mActionsManager.stopRobotAction()
			, &QAction::triggered
			, mInterpreter
			, &interpreter::InterpreterInterface::stopRobot
			);

	QObject::connect(
			&mActionsManager.connectToRobotAction()
			, &QAction::triggered
			, mInterpreter
			, &interpreter::InterpreterInterface::connectToRobot
			);
}

bool RobotsPluginFacade::selectKit(const qReal::PluginConfigurator &configurer)
{
	/// @todo reinit it each time when robot model changes
	/// @todo: do we need this method?
	const QString selectedKit = qReal::SettingsManager::value("SelectedRobotKit").toString();
	if (selectedKit.isEmpty() && !mKitPluginManager.kitIds().isEmpty()) {
		qReal::SettingsManager::setValue("SelectedRobotKit", mKitPluginManager.kitIds()[0]);
	} else if (mKitPluginManager.kitIds().isEmpty()) {
		configurer.mainWindowInterpretersInterface().setEnabledForAllElementsInPalette(false);

		/// @todo Correctly handle unselected kit.
		return false;
	}

	return true;
}

void RobotsPluginFacade::initSensorWidgets()
{
	mDockDevicesConfigurer = new kitBase::DevicesConfigurationWidget(nullptr, true);
	mDockDevicesConfigurer->loadRobotModels(mKitPluginManager.allRobotModels());
	connect(&mRobotModelManager, &RobotModelManager::robotModelChanged
			, mDockDevicesConfigurer, &kitBase::DevicesConfigurationWidget::selectRobotModel);
	for (kitBase::robotModel::RobotModelInterface * const model : mKitPluginManager.allRobotModels()) {
		for (kitBase::KitPluginInterface * const kit : mKitPluginManager.kitsById(model->kitId())) {
			mDockDevicesConfigurer->prependCustomWidget(*model, kit->quickPreferencesFor(*model));
		}
	}

	mWatchListWindow = new utils::WatchListWindow(*mParser);

	auto hideVariables = [=]() { mWatchListWindow->hideVariables(mParser->hiddenVariables()); };
	hideVariables();
	connect(&mRobotModelManager, &RobotModelManager::robotModelChanged, hideVariables);

	mGraphicsWatcherManager = new GraphicsWatcherManager(*mParser, this);
	connect(mInterpreter, &interpreter::InterpreterInterface::started
			, mGraphicsWatcherManager, &GraphicsWatcherManager::forceStart);
	connect(mInterpreter, &interpreter::InterpreterInterface::stopped
			, mGraphicsWatcherManager, &GraphicsWatcherManager::forceStop);
	connect(mInterpreter, &interpreter::InterpreterInterface::started, mGraphicsWatcherManager, [=]() {
		mActionsManager.runAction().setVisible(false);
		mActionsManager.stopRobotAction().setVisible(mRobotModelManager.model().interpretedModel());
	});
	connect(mInterpreter, &interpreter::InterpreterInterface::stopped, mGraphicsWatcherManager, [=]() {
		mActionsManager.runAction().setVisible(mRobotModelManager.model().interpretedModel());
		mActionsManager.stopRobotAction().setVisible(false);
	});

	mCustomizer.placeDevicesConfig(mDockDevicesConfigurer);
	mCustomizer.placeWatchPlugins(mWatchListWindow, mGraphicsWatcherManager->widget());

	mDevicesConfigurationManager->connectDevicesConfigurationProvider(mRobotSettingsPage);
	mDevicesConfigurationManager->connectDevicesConfigurationProvider(mDockDevicesConfigurer);
	mDevicesConfigurationManager->connectDevicesConfigurationProvider(mGraphicsWatcherManager);
}

void RobotsPluginFacade::initKitPlugins(const qReal::PluginConfigurator &configurer)
{
	/// @todo: Check that this code works when different kit is selected
	for (const QString &kitId : mKitPluginManager.kitIds()) {
		for (kitBase::KitPluginInterface * const kit : mKitPluginManager.kitsById(kitId)) {
			kit->init(kitBase::KitPluginConfigurator(configurer
					, mRobotModelManager, *mParser, mEventsForKitPlugin, *mInterpreter));

			for (const kitBase::robotModel::RobotModelInterface *model : kit->robotModels()) {
				initFactoriesFor(kitId, model, configurer);
				connect(&mEventsForKitPlugin, &kitBase::EventsForKitPluginInterface::interpretationStarted
						, model, &kitBase::robotModel::RobotModelInterface::onInterpretationStarted);
			}

			mDevicesConfigurationManager->connectDevicesConfigurationProvider(kit->devicesConfigurationProvider());
		}
	}
}

void RobotsPluginFacade::initFactoriesFor(const QString &kitId
		, const kitBase::robotModel::RobotModelInterface *model
		, const qReal::PluginConfigurator &configurer)
{
	// Pulling each robot model to each kit plugin with same ids. We need it for supporting
	// plugin-based blocks set extension for concrete roobt model.
	for (kitBase::KitPluginInterface * const kit : mKitPluginManager.kitsById(kitId)) {
		kitBase::blocksBase::BlocksFactoryInterface * const factory = kit->blocksFactoryFor(model);
		if (factory) {
			/// @todo Non-obvious dependency on mParser, which may or may not be constructed here.
			///       More functional style will be helpful here.
			factory->configure(configurer.graphicalModelApi()
					, configurer.logicalModelApi()
					, mRobotModelManager
					, *configurer.mainWindowInterpretersInterface().errorReporter()
					, *mParser
					);

			mBlocksFactoryManager.addFactory(factory, model);
		}
	}
}

void RobotsPluginFacade::connectEventsForKitPlugin()
{
	QObject::connect(
			mInterpreter
			, &interpreter::InterpreterInterface::started
			, &mEventsForKitPlugin
			, &kitBase::EventsForKitPluginInterface::interpretationStarted
			);

	QObject::connect(
			mInterpreter
			, &interpreter::InterpreterInterface::stopped
			, &mEventsForKitPlugin
			, &kitBase::EventsForKitPluginInterface::interpretationStopped
			);

	QObject::connect(
			&mRobotModelManager
			, &RobotModelManager::robotModelChanged
			, [this](kitBase::robotModel::RobotModelInterface &model) {
				emit mEventsForKitPlugin.robotModelChanged(model.name());
			}
			);

}

void RobotsPluginFacade::sync()
{
	mRobotModelManager.sync();
}
