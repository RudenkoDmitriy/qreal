#pragma once

#include "qrtext/lua/ast/unaryOperator.h"

namespace qrtext {
namespace lua {
namespace ast {

/// Represents arithmetic negation operator (-).
class QRTEXT_EXPORT UnaryMinus : public UnaryOperator
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
