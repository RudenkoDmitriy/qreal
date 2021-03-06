#pragma once

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtWidgets/QWidget>

#include <qrkernel/ids.h>
#include <qrgui/plugins/toolPluginInterface/actionInfo.h>
#include <qrgui/plugins/toolPluginInterface/hotKeyActionInfo.h>
#include <qrgui/plugins/toolPluginInterface/systemEvents.h>
#include <qrgui/plugins/toolPluginInterface/usedInterfaces/mainWindowInterpretersInterface.h>

#include <kitBase/kitPluginConfigurator.h>
#include <kitBase/additionalPreferences.h>
#include <kitBase/devicesConfigurationProvider.h>
#include <kitBase/blocksBase/blocksFactoryInterface.h>

namespace kitBase {

/// Plugin interface for a kit support plugin.
class KitPluginInterface
{
public:
	virtual ~KitPluginInterface() {}

	/// Passes to kit plugin objects that allow it to communicate with engine.
	virtual void init(const KitPluginConfigurator &configurator)
	{
		Q_UNUSED(configurator)
	}

	/// An identifier of constructor kit. Kit plugins with same kitId are automaticly groupped
	/// together extending each other.
	virtual QString kitId() const = 0;

	/// String that will be displayed to users as the name of the kit. If more than one kit plugins with the same
	/// id implemented in system friendly name can be specified only in one of them.
	virtual QString friendlyKitName() const = 0;

	/// Returns a list of robot models supported by this kit plugin.
	virtual QList<robotModel::RobotModelInterface *> robotModels() = 0;

	/// Returns a blocks factory for a given robot model. Nullptr can be returned and means that no factory is provided
	/// by this plugin. Ownership must be transfered.
	virtual blocksBase::BlocksFactoryInterface *blocksFactoryFor(const robotModel::RobotModelInterface *model) = 0;

	/// If overrided and returns some model as value that model will be used as default selected model for this kit
	/// @todo is it possible that plugin does not provide default model? Maybe first model in robotModels() list will be
	/// enough?
	virtual robotModel::RobotModelInterface *defaultRobotModel()
	{
		return nullptr;
	}

	/// returned no widget is added on settings tab
	/// Widget with specific settings for a plugin. save() method is called when user saves all settings,
	/// restoreSettings() - each time when used selects other robot model for this kit. If nullptr is
	/// Transfers ownership.
	virtual QList<AdditionalPreferences *> settingsWidgets() = 0;

	/// Returns a widget that will be placed at devices configuration widget in the left-hand side dock.
	/// The default implementation returns nullptr.
	/// Transfers ownership.
	virtual QWidget *quickPreferencesFor(const robotModel::RobotModelInterface &model)
	{
		Q_UNUSED(model)
		return nullptr;
	}

	/// List of additional actions supported by plugin, to be added to toolbar and menus. Showing 2d model widget
	/// goes here.
	/// @todo: what about ownership of underlying QAction objects?
	virtual QList<qReal::ActionInfo> customActions() = 0;

	/// Shall be overridden to return QAction instances for their customization in hot key manager.
	virtual QList<qReal::HotKeyActionInfo> hotKeyActions() = 0;

	/// Can be overridden to return a path to a file in INI format with default settings for kit plugin.
	/// If empty string is returned (like in default implementation) default settings are ignored for this kit plugin.
	virtual QString defaultSettingsFile() const
	{
		return QString();
	}

	/// Returns an icon for toolbar action of fast robot model switching.
	/// It is guaranteed that this method will be called with the robot model provided by this kit.
	virtual QIcon iconForFastSelector(const robotModel::RobotModelInterface &robotModel) const = 0;

	/// Returns an integer number that specifies the count of recommended robot models.
	/// Robot models are sorted by their priorities ("importance" for user). Then robot models switching menu is
	/// ordered due to this sorting. Then if the number of robot models provided by kit is too large some of most
	/// important robot models can be highlighted from robot models set as recommended by TRIK Studio.
	/// The number of such highlighted robot models is specified by this method.
	/// Different kit plugins for the same kit id can return different values. The engine will select maximal of them
	/// as the fact value.
	/// Default implementation returns 2.
	virtual int topRecommendedRobotModels() const
	{
		return 2;
	}

	/// Sensor configuration provider object for this plugin that needs to be registered in providers network.
	/// Can be a root of plugin-specific providers network. Null, if there is no need for sensors configuration.
	/// Does not transfer ownership.
	virtual kitBase::DevicesConfigurationProvider * devicesConfigurationProvider()
	{
		return nullptr;
	}
};

}

Q_DECLARE_INTERFACE(kitBase::KitPluginInterface, "ru.spbsu.math.QReal.KitPluginInterface/1")
