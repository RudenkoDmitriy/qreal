#pragma once

#include <interpreterBase/robotModel/robotParts/colorSensorPassive.h>

#include "commonTwoDModel/commonTwoDModelDeclSpec.h"

namespace twoDModel {

namespace engine {
class TwoDModelEngineInterface;
}

namespace robotModel {
namespace parts {

class COMMON_TWO_D_MODEL_EXPORT ColorSensorPassive : public interpreterBase::robotModel::robotParts::ColorSensorPassive
{
	Q_OBJECT
	Q_CLASSINFO("friendlyName", tr("Color sensor (passive)"))

public:
	ColorSensorPassive(interpreterBase::robotModel::DeviceInfo const &info
			, interpreterBase::robotModel::PortInfo const &port
			, engine::TwoDModelEngineInterface &engine);

	void read();

protected:
	void doConfiguration();

private:
	engine::TwoDModelEngineInterface &mEngine;
};

}
}
}