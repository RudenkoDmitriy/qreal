#pragma once

#include "qrtext/lua/ast/binaryOperator.h"

namespace qrtext {
namespace lua {
namespace ast {

/// Represents addition operator.
class Addition : public BinaryOperator
{
private:
	void accept(core::AstVisitorInterface &visitor) const override
	{
		static_cast<LuaAstVisitorInterface *>(&visitor)->visit(*this);
	}
};

}
}
}
