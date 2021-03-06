#pragma once

#include <kitBase/blocksBase/common/displayBlock.h>

namespace nxt {
namespace blocks {
namespace details {

class DrawPixelBlock : public kitBase::blocksBase::common::DisplayBlock
{
	Q_OBJECT

public:
	explicit DrawPixelBlock(kitBase::robotModel::RobotModelInterface &robotModel);

private:
	void doJob(kitBase::robotModel::robotParts::Display &display) override;
};

}
}
}
