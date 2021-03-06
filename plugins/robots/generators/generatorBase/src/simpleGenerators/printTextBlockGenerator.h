#pragma once

#include "generatorBase/simpleGenerators/bindingGenerator.h"

namespace generatorBase {
namespace simple {

/// Generator for 'PrintText' block
class PrintTextBlockGenerator : public BindingGenerator
{
public:
	PrintTextBlockGenerator(const qrRepo::RepoApi &repo
			, GeneratorCustomizer &customizer
			, const qReal::Id &id
			, QObject *parent = 0);
};

}
}
