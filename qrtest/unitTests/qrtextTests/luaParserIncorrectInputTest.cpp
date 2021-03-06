#include "luaParserIncorrectInputTest.h"

#include "gtest/gtest.h"

using namespace qrtext;
using namespace qrtext::lua;
using namespace qrtext::lua::details;
using namespace qrTest;

void LuaParserIncorrectInputTest::SetUp()
{
	mErrors.clear();
	mLexer.reset(new LuaLexer(mErrors));
	mParser.reset(new LuaParser(mErrors));
}

TEST_F(LuaParserIncorrectInputTest, incorrectLogicalOperators)
{
	const QString &stream = "true ||| false";
	mParser->parse(mLexer->tokenize(stream), mLexer->userFriendlyTokenNames());
	EXPECT_FALSE(mErrors.isEmpty());
}

TEST_F(LuaParserIncorrectInputTest, incorrectArraySlice)
{
	const QString &stream = "a[i, j]";
	mParser->parse(mLexer->tokenize(stream), mLexer->userFriendlyTokenNames());
	EXPECT_FALSE(mErrors.isEmpty());
}
