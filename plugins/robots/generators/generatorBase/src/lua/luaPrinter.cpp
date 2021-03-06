#include "luaPrinter.h"

#include <qrkernel/logging.h>

#include <qrtext/lua/ast/number.h>
#include <qrtext/lua/ast/unaryMinus.h>
#include <qrtext/lua/ast/not.h>
#include <qrtext/lua/ast/bitwiseNegation.h>
#include <qrtext/lua/ast/length.h>
#include <qrtext/lua/ast/logicalAnd.h>
#include <qrtext/lua/ast/logicalOr.h>
#include <qrtext/lua/ast/addition.h>
#include <qrtext/lua/ast/subtraction.h>
#include <qrtext/lua/ast/multiplication.h>
#include <qrtext/lua/ast/division.h>
#include <qrtext/lua/ast/integerDivision.h>
#include <qrtext/lua/ast/modulo.h>
#include <qrtext/lua/ast/exponentiation.h>
#include <qrtext/lua/ast/bitwiseAnd.h>
#include <qrtext/lua/ast/bitwiseOr.h>
#include <qrtext/lua/ast/bitwiseXor.h>
#include <qrtext/lua/ast/bitwiseLeftShift.h>
#include <qrtext/lua/ast/bitwiseRightShift.h>
#include <qrtext/lua/ast/concatenation.h>
#include <qrtext/lua/ast/equality.h>
#include <qrtext/lua/ast/lessThan.h>
#include <qrtext/lua/ast/lessOrEqual.h>
#include <qrtext/lua/ast/inequality.h>
#include <qrtext/lua/ast/greaterThan.h>
#include <qrtext/lua/ast/greaterOrEqual.h>
#include <qrtext/lua/ast/integerNumber.h>
#include <qrtext/lua/ast/floatNumber.h>
#include <qrtext/lua/ast/fieldInitialization.h>
#include <qrtext/lua/ast/tableConstructor.h>
#include <qrtext/lua/ast/string.h>
#include <qrtext/lua/ast/true.h>
#include <qrtext/lua/ast/false.h>
#include <qrtext/lua/ast/nil.h>
#include <qrtext/lua/ast/identifier.h>
#include <qrtext/lua/ast/functionCall.h>
#include <qrtext/lua/ast/methodCall.h>
#include <qrtext/lua/ast/assignment.h>
#include <qrtext/lua/ast/block.h>
#include <qrtext/lua/ast/indexingExpression.h>
#include <qrtext/languageToolboxInterface.h>
#include <qrtext/lua/types/integer.h>
#include <qrtext/lua/types/float.h>
#include <qrtext/lua/types/boolean.h>
#include <qrtext/lua/types/string.h>

#include "reservedFunctionsConverter.h"

using namespace generatorBase::lua;

LuaPrinter::LuaPrinter(const QString &pathToTemplates
		, const qrtext::LanguageToolboxInterface &textLanguage
		, PrecedenceConverterInterface &precedeceTable
		, const simple::Binding::ConverterInterface *reservedVariablesConverter)
	: TemplateParametrizedEntity(pathToTemplates + "/luaPrinting")
	, mTextLanguage(textLanguage)
	, mPrecedenceTable(precedeceTable)
	, mReservedVariablesConverter(reservedVariablesConverter)
	, mReservedFunctionsConverter(pathToTemplates)
{
}

LuaPrinter::~LuaPrinter()
{
	delete mReservedVariablesConverter;
}

QString LuaPrinter::print(const QSharedPointer<qrtext::lua::ast::Node> &node)
{
	return printWithoutPop(node) ? popResult(*node) : QString();
}

QString LuaPrinter::castToString(const QSharedPointer<qrtext::lua::ast::Node> &node)
{
	return printWithoutPop(node) ? toString(node) : QString();
}

void LuaPrinter::pushResult(const qrtext::lua::ast::Node &node, const QString &generatedCode)
{
	mGeneratedCode[&node] = generatedCode;
}

QString LuaPrinter::popResult(const qrtext::lua::ast::Node &node, bool wrapIntoBrackets)
{
	const QString code = mGeneratedCode.take(&node);
	return wrapIntoBrackets ? "(" + code + ")" : code;
}

QStringList LuaPrinter::popResults(const QList<QSharedPointer<qrtext::lua::ast::Node>> &nodes)
{
	QStringList result;
	for (const QSharedPointer<qrtext::lua::ast::Node> &node : nodes) {
		result << popResult(*node);
	}

	return result;
}

bool LuaPrinter::printWithoutPop(const QSharedPointer<qrtext::lua::ast::Node> &node)
{
	if (!node) {
		return false;
	}

	node->acceptRecursively(*this);
	if (mGeneratedCode.keys().count() != 1 || mGeneratedCode.keys().first() != node.data()) {
		QLOG_WARN() << "Lua printer got into the inconsistent state during printing."
				<< mGeneratedCode.keys().count() << "pieces of code:";
		for (const QString &code : mGeneratedCode.values()) {
			QLOG_INFO() << code;
		}

		mGeneratedCode.clear();
		return false;
	}

	return true;
}

void LuaPrinter::processTemplate(const qrtext::lua::ast::Node &node
		, const QString &templateFileName
		, QMap<QString, QSharedPointer<qrtext::lua::ast::Node>> const &bindings)
{
	QString result = readTemplate(templateFileName);
	for (const QString &toReplace : bindings.keys()) {
		result.replace(toReplace, popResult(*bindings[toReplace]));
	}

	pushResult(node, result);
}

void LuaPrinter::processUnary(const qrtext::core::ast::UnaryOperator &node, const QString &templateFileName)
{
	pushResult(node, readTemplate(templateFileName)
			.replace("@@OPERAND@@", popResult(*node.operand(), needBrackets(node, *node.operand()))));
}

void LuaPrinter::processBinary(const qrtext::core::ast::BinaryOperator &node, const QString &templateFileName)
{
	pushResult(node, readTemplate(templateFileName)
			.replace("@@LEFT@@", popResult(*node.leftOperand(), needBrackets(node
					, *node.leftOperand(), qrtext::core::Associativity::left)))
			.replace("@@RIGHT@@", popResult(*node.rightOperand(), needBrackets(node
					, *node.rightOperand(), qrtext::core::Associativity::right))));
}

bool LuaPrinter::needBrackets(const qrtext::lua::ast::Node &parent
		, const qrtext::lua::ast::Node &child
		, qrtext::core::Associativity childAssociativity) const
{
	const int parentPrecedence = mPrecedenceTable.precedence(parent);
	const int childPrecedence = mPrecedenceTable.precedence(child);
	return parentPrecedence > childPrecedence || (parentPrecedence == childPrecedence
			&& mPrecedenceTable.associativity(parent) != childAssociativity);
}

void LuaPrinter::visit(const qrtext::lua::ast::Number &node)
{
	pushResult(node, node.stringRepresentation());
}

void LuaPrinter::visit(const qrtext::lua::ast::UnaryMinus &node)
{
	processUnary(node, "unaryMinus.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Not &node)
{
	processUnary(node, "not.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseNegation &node)
{
	processUnary(node, "bitwiseNegation.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Length &node)
{
	processUnary(node, "length.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::LogicalAnd &node)
{
	processBinary(node, "logicalAnd.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::LogicalOr &node)
{
	processBinary(node, "logicalOr.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Addition &node)
{
	processBinary(node, "addition.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Subtraction &node)
{
	processBinary(node, "subtraction.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Multiplication &node)
{
	processBinary(node, "multiplication.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Division &node)
{
	processBinary(node, "division.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::IntegerDivision &node)
{
	processBinary(node, "integerDivision.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Modulo &node)
{
	processBinary(node, "modulo.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Exponentiation &node)
{
	processBinary(node, "exponentiation.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseAnd &node)
{
	processBinary(node, "bitwiseAnd.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseOr &node)
{
	processBinary(node, "bitwiseOr.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseXor &node)
{
	processBinary(node, "bitwiseXor.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseLeftShift &node)
{
	processBinary(node, "bitwiseLeftShift.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::BitwiseRightShift &node)
{
	processBinary(node, "bitwiseRightShift.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Concatenation &node)
{
	pushResult(node, readTemplate("concatenation.t")
			.replace("@@LEFT@@", toString(node.leftOperand()))
			.replace("@@RIGHT@@", toString(node.rightOperand())));
}

void LuaPrinter::visit(const qrtext::lua::ast::Equality &node)
{
	processBinary(node, "equality.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::LessThan &node)
{
	processBinary(node, "lessThan.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::LessOrEqual &node)
{
	processBinary(node, "lessOrEqual.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::Inequality &node)
{
	processBinary(node, "inequality.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::GreaterThan &node)
{
	processBinary(node, "greaterThan.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::GreaterOrEqual &node)
{
	processBinary(node, "greaterOrEqual.t");
}

void LuaPrinter::visit(const qrtext::lua::ast::IntegerNumber &node)
{
	pushResult(node, node.stringRepresentation());
}

void LuaPrinter::visit(const qrtext::lua::ast::FloatNumber &node)
{
	pushResult(node, node.stringRepresentation());
}

void LuaPrinter::visit(const qrtext::lua::ast::FieldInitialization &node)
{
	const QString templatePath = node.key().data()
			? "explicitKeyFieldInitialization.t"
			: "implicitKeyFieldInitialization.t";
	processTemplate(node, templatePath, { {"@@KEY@@", node.key()}, {"@@VALUE@@", node.value()} });
}

void LuaPrinter::visit(const qrtext::lua::ast::TableConstructor &node)
{
	const QStringList initializers = popResults(qrtext::as<qrtext::lua::ast::Node>(node.initializers()));
	pushResult(node, readTemplate("tableConstructor.t")
			.replace("@@COUNT@@", QString::number(initializers.count()))
			.replace("@@INITIALIZERS@@", initializers.join(readTemplate("fieldInitializersSeparator.t"))));
}

void LuaPrinter::visit(const qrtext::lua::ast::String &node)
{
	pushResult(node, readTemplate("string.t").replace("@@VALUE@@", node.string()));
}

void LuaPrinter::visit(const qrtext::lua::ast::True &node)
{
	pushResult(node, readTemplate("true.t"));
}

void LuaPrinter::visit(const qrtext::lua::ast::False &node)
{
	pushResult(node, readTemplate("false.t"));
}

void LuaPrinter::visit(const qrtext::lua::ast::Nil &node)
{
	pushResult(node, readTemplate("nil.t"));
}

void LuaPrinter::visit(const qrtext::lua::ast::Identifier &node)
{
	/// @todo: if some function or method will have same id as some reserved variable it will be replaced too...
	pushResult(node, mReservedVariablesConverter->convert(node.name()));
}

void LuaPrinter::visit(const qrtext::lua::ast::FunctionCall &node)
{
	const QString expression = popResult(*node.function());
	const QStringList arguments = popResults(qrtext::as<qrtext::lua::ast::Node>(node.arguments()));

	const qrtext::lua::ast::Identifier *idNode = dynamic_cast<qrtext::lua::ast::Identifier *>(node.function().data());
	const QString reservedFunctionCall = idNode
			? mReservedFunctionsConverter.convert(idNode->name(), arguments)
			: QString();

	if (reservedFunctionCall.isEmpty()) {
		pushResult(node, readTemplate("functionCall.t")
				.replace("@@FUNCTION@@", expression)
				.replace("@@ARGUMENTS@@", arguments.join(readTemplate("argumentsSeparator.t"))));
	} else {
		pushResult(node, reservedFunctionCall);
	}
}

void LuaPrinter::visit(const qrtext::lua::ast::MethodCall &node)
{
	const QString object = popResult(*node.object());
	const QString method = popResult(*node.methodName());
	const QStringList arguments = popResults(qrtext::as<qrtext::lua::ast::Node>(node.arguments()));
	pushResult(node, readTemplate("methodCall.t")
			.replace("@@OBJECT@@", object)
			.replace("@@METHOD@@", method)
			.replace("@@ARGUMENTS@@", arguments.join(readTemplate("argumentsSeparator.t"))));
}

void LuaPrinter::visit(const qrtext::lua::ast::Assignment &node)
{
	processTemplate(node, "assignment.t", { {"@@VARIABLE@@", node.variable()}, {"@@VALUE@@", node.value()} });
}

void LuaPrinter::visit(const qrtext::lua::ast::Block &node)
{
	const QStringList expressions = popResults(node.children());
	pushResult(node, expressions.join(readTemplate("statementsSeparator.t")));
}

void LuaPrinter::visit(const qrtext::lua::ast::IndexingExpression &node)
{
	processTemplate(node, "indexingExpression.t", { {"@@TABLE@@", node.table()}, {"@@INDEXER@@", node.indexer()} });
}

QString LuaPrinter::toString(const QSharedPointer<qrtext::lua::ast::Node> &node)
{
	const QSharedPointer<qrtext::core::types::TypeExpression> type = mTextLanguage.type(node);
	const QString value = popResult(*node);
	if (type->is<qrtext::lua::types::String>()) {
		return value;
	}

	if (type->is<qrtext::lua::types::Integer>()) {
		return readTemplate("intToString.t").replace("@@VALUE@@", value);
	}

	if (type->is<qrtext::lua::types::Float>()) {
		return readTemplate("floatToString.t").replace("@@VALUE@@", value);
	}

	if (type->is<qrtext::lua::types::Integer>()) {
		return readTemplate("boolToString.t").replace("@@VALUE@@", value);
	}

	return readTemplate("otherToString.t").replace("@@VALUE@@", value);
}
