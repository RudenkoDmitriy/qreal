#pragma once

#include "qrtext/core/parser/operators/parserInterface.h"
#include "qrtext/core/parser/temporaryNodes/temporaryToken.h"
#include "qrtext/core/parser/temporaryNodes/temporaryErrorNode.h"
#include "qrtext/core/parser/temporaryNodes/temporaryDiscardableNode.h"

namespace qrtext {
namespace core {

/// Elementary parser that parses given token and executes given semantic action on a result. Semantic action takes
/// parsed token as a parameter.
template<typename TokenType, typename SemanticAction>
class TokenParser : public ParserInterface<TokenType>
{
public:
	/// Constructor. Takes token to parse and lambda function to execute if token is parsed successfully.
	explicit TokenParser(TokenType token, const SemanticAction &semanticAction)
		: mToken(token), mSemanticAction(semanticAction)
	{
	}

	QSharedPointer<ast::Node> parse(TokenStream<TokenType> &tokenStream
			, ParserContext<TokenType> &parserContext) const override
	{
		Q_UNUSED(parserContext);

		Token<TokenType> const token = tokenStream.next();
		if (!tokenStream.expect(mToken)) {
			return wrap(new TemporaryErrorNode());
		}

		auto node = wrap(mSemanticAction(token));
		if (node) {
			node->connect(token);
		} else {
			parserContext.reportInternalError(
					QObject::tr("Semantic action incorrectly discarded node in TokenParser"));

			return wrap(new TemporaryDiscardableNode());
		}

		return node;
	}

	QSet<TokenType> first() const override
	{
		return {mToken};
	}

private:
	TokenType mToken;
	SemanticAction mSemanticAction;
};

}
}
