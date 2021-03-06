#include "generatorBase/semanticTree/semanticNode.h"

#include <qrutils/stringUtils.h>

using namespace generatorBase::semantics;
using namespace qReal;

SemanticNode::SemanticNode(const Id &idBinded, QObject *parent)
	: QObject(parent)
	, mId(idBinded)
	, mParentNode(nullptr)
	, mLabeled(false)
{
}

Id SemanticNode::id() const
{
	return mId;
}

void SemanticNode::bindTo(const Id &id)
{
	mId = id;
}

void SemanticNode::setParentNode(SemanticNode * const parent)
{
	mParentNode = parent;
}

void SemanticNode::addLabel()
{
	mLabeled = true;
}

QString SemanticNode::toString(GeneratorCustomizer &customizer, int indent, const QString &indentString) const
{
	return (mLabeled
			? utils::StringUtils::addIndent(customizer.factory()->labelGenerator(mId
					, customizer)->generate(), indent, indentString) + "\n"
			: QString()) + toStringImpl(customizer, indent, indentString);
}

SemanticNode *SemanticNode::findNodeFor(const Id &id)
{
	if (id == mId) {
		return this;
	}

	QLinkedList<SemanticNode *> const children = this->children();
	foreach (SemanticNode * const child, children) {
		SemanticNode * const searchResult = child->findNodeFor(id);
		if (searchResult) {
			return searchResult;
		}
	}

	return nullptr;
}
