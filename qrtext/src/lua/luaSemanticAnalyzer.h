#pragma once

#include <QtCore/QScopedPointer>

#include "qrtext/core/semantics/semanticAnalyzer.h"
#include "qrtext/lua/types/function.h"

#include "qrtext/declSpec.h"

namespace qrtext {
namespace lua {
namespace details {

/// Semantic analyzer for Lua language.
class QRTEXT_EXPORT LuaSemanticAnalyzer : public core::SemanticAnalyzer
{
public:
	/// Constructor.
	/// @param errors - error stream to report errors to.
	explicit LuaSemanticAnalyzer(QList<core::Error> &errors);

	/// Registers external (intrinsic for a language) function in interpreter.
	/// @param name - name of a function.
	/// @param type - type of a function.
	void addIntrinsicFunction(const QString &name, const QSharedPointer<types::Function> &type);

	/// Override that excludes intrinsic function identifiers.
	QMap<QString, QSharedPointer<core::types::TypeExpression>> variableTypes() const;

private:
	void analyzeNode(const QSharedPointer<core::ast::Node> &node) override;
	void analyzeUnaryOperator(const QSharedPointer<core::ast::Node> &node);
	void analyzeBinaryOperator(const QSharedPointer<core::ast::Node> &node);

	void constrainAssignment(const QSharedPointer<core::ast::Node> &operation
			, const QSharedPointer<core::ast::Node> &lhs, const QSharedPointer<core::ast::Node> &rhs);

	void analyzeFunctionCall(const QSharedPointer<core::ast::Node> &node);

	/// Reports errors about all identifiers first declared in a given subtree. Supposed to be called to check
	/// right-hand side of an assignment, where shall not be any declarations.
	/// It looks hacky, but when we analyze assignment we've already visited identifiers inside, but had no information
	/// about side of assignment operator there (SemanticAnalyser knows nothing about assignment, so can not pass
	/// such information down to recursive tree traversal).
	void checkForUndeclaredIdentifiers(const QSharedPointer<core::ast::Node> &node);

	QSharedPointer<core::types::TypeExpression> mBoolean;
	QSharedPointer<core::types::TypeExpression> mFloat;
	QSharedPointer<core::types::TypeExpression> mInteger;
	QSharedPointer<core::types::TypeExpression> mNil;
	QSharedPointer<core::types::TypeExpression> mString;

	QHash<QString, QSharedPointer<types::Function>> mIntrinsicFunctions;
};

}
}
}
