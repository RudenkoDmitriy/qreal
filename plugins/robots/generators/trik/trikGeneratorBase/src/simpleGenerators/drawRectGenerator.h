#pragma once

#include <generatorBase/simpleGenerators/bindingGenerator.h>

namespace trik {
namespace simple {

/// Generator for Draw Rect block.
class DrawRectGenerator : public generatorBase::simple::BindingGenerator
{
public:
	DrawRectGenerator(const qrRepo::RepoApi &repo
			, generatorBase::GeneratorCustomizer &customizer
			, const qReal::Id &id
			, QObject *parent);
};

}
}
