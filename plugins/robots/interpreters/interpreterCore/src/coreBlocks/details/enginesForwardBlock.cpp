#include "enginesForwardBlock.h"

#include <interpreterBase/robotModel/robotParts/motor.h>

using namespace interpreterCore::coreBlocks::details;

EnginesForwardBlock::EnginesForwardBlock(interpreterBase::robotModel::RobotModelInterface &robotModel)
	: EngineCommandBlock(robotModel)
{
}

void EnginesForwardBlock::run()
{
	int const power = evaluate("Power").toInt();

	/// @todo Makes sense only for NXT.
	bool const breakMode = stringProperty("Mode") != QString::fromUtf8("скользить");

	/// @todo Move to model or some helper.
	QString const ports = stringProperty("Ports");
	QStringList const splitted = ports.split(',', QString::SkipEmptyParts);

	for (QString const &port : splitted) {
		for (interpreterBase::robotModel::PortInfo portInfo : mRobotModel.availablePorts()) {
			if (portInfo.name() == port || portInfo.nameAliases().contains(port)) {
				interpreterBase::robotModel::robotParts::Device *device
						= mRobotModel.configuration().device(portInfo
								, interpreterBase::robotModel::ConfigurationInterface::output);
				interpreterBase::robotModel::robotParts::Motor *motor
						= dynamic_cast<interpreterBase::robotModel::robotParts::Motor *>(device);
				if (motor) {
					motor->on(power, breakMode);
				}
			}
		}
	}

	emit done(mNextBlockId);
}
