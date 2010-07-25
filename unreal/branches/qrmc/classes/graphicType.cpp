#include "graphicType.h"
#include "property.h"
#include "diagram.h"
#include "utils/nameNormalizer.h"
#include "../utils/outFile.h"

#include <QDebug>

using namespace utils;
using namespace qReal;

GraphicType::ContainerProperties::ContainerProperties() : isSortContainer(false), sizeOfForestalling(0),
	sizeOfChildrenForestalling(0), isChildrenMovable(true), isMinimizingToChildren(false), isClass(false), isMaximizingChildren(false)
{}

GraphicType::ResolvingHelper::ResolvingHelper(bool &resolvingFlag)
	: mResolvingFlag(resolvingFlag)
{
	mResolvingFlag = true;
}

GraphicType::GraphicType(Diagram *diagram, qrRepo::RepoApi *api, const qReal::Id &id)
	: Type(false, diagram, api, id), mWidth(-1), mHeight(-1), mResolving(false)
{
}

GraphicType::~GraphicType()
{
}

bool GraphicType::init(QString const &context)
{
	Type::init(context);

	mIsVisible = false;
	if (mApi->hasProperty(mId, "shape"))
		mIsVisible = !mApi->stringProperty(mId, "shape").isEmpty();
	mContains << mApi->stringProperty(mId, "container").split(",", QString::SkipEmptyParts);

	foreach(Id id, mApi->children(mId)) {
		if (id.element() == "MetaEntityParent") {
			QString parentName = mApi->name(id);
			if (!mParents.contains(parentName))
				mParents.append(parentName);
			else {
				qDebug() << "ERROR: parent of node" << qualifiedName() << "duplicated";
				return false;
			}
		} else if (id.element() == "MetaEntity_Attribute") {
			Property *property = new Property(mApi, id);
			if (!property->init()) {
				delete property;
				continue;
			}
			if (!addProperty(property))
				return false;
		} else if (id.element() == "MetaEntityConnection") {
			mConnections << mApi->stringProperty(id, "type").section("::", -1);
		} else if (id.element() == "MetaEntityUsage") {
			mUsages << mApi->stringProperty(id, "type").section("::", -1);
		}
	}
	initPossibleEdges();
	return true;
}

bool GraphicType::initPossibleEdges()
{
	IdList list = mApi->elementsByType("MetaEntityPossibleEdge");
	qDebug() << "++ possible edges: " << list.size();
	return true;
}

GraphicType::ResolvingHelper::~ResolvingHelper()
{
	mResolvingFlag = false;
}

void GraphicType::copyFields(GraphicType *type) const
{
	Type::copyFields(type);
//	type->mElement = mElement;
//	type->mGraphics = mGraphics;
	type->mHeight = mHeight;
//r	type->mLogic = mLogic;
	type->mParents = mParents;
	type->mIsVisible = mIsVisible;
	type->mWidth = mWidth;
	type->mContainerProperties = mContainerProperties;
	type->mContains = mContains;
}

bool GraphicType::resolve()
{
	if (mResolvingFinished)
		return true;

	ResolvingHelper helper(mResolving);
	mParents.removeDuplicates();
	foreach (QString parentName, mParents) {
		// searching for parents in native context. if it was imported, references will remain valid
		QString qualifiedParentName = parentName.contains("::") ? parentName : nativeContext() + "::" + parentName;

		Type *parent = mDiagram->findType(qualifiedParentName);
		if (parent == NULL) {
			// didn't find in local context, try global
			parent = mDiagram->findType(parentName);
			if (parent == NULL) {
				qDebug() << "ERROR: can't find parent" << parentName << "for" << qualifiedName();
				return false;
			}
		}

		if (parent->isResolving()) {
			qDebug() << "ERROR: circular inheritance between" << parentName << "and" << qualifiedName();
			return false;
		}
		if (!parent->isResolved()) {
			if (!parent->resolve())
				return false;
		}
		foreach (Property *property, parent->properties().values())
			if (!addProperty(property->clone()))
				return false;

		GraphicType* gParent = dynamic_cast<GraphicType*>(parent);
		if (gParent)
			foreach (PossibleEdge pEdge,gParent->mPossibleEdges) {
				mPossibleEdges.append(qMakePair(pEdge.first,qMakePair(pEdge.second.first,name())));
			}
	}

	mResolvingFinished = true;
	return true;


	return true;
}

bool GraphicType::isResolving() const
{
	return mResolving;
}

bool GraphicType::addProperty(Property *property)
{
	QString propertyName = property->name();
	if (mProperties.contains(propertyName)) {
		if (mProperties[propertyName] != property && *mProperties[propertyName] != *property)
		{
			qDebug() << "ERROR: property" << propertyName << "duplicated with different attributes";
			delete property;
			return false;
		}
	}
	else
		mProperties[propertyName] = property;
	return true;
}

bool GraphicType::isGraphicalType() const
{
	return mIsVisible;
}

QString GraphicType::generateProperties(const QString &lineTemplate) const
{
	if (!mIsVisible)
		return "";
	QString propertiesString;
	foreach (Property *property, mProperties) {
		// hack: don't generate pre-defined properties
		if (property->name() == "fromPort" || property->name() == "toPort"
			|| property->name() == "from" || property->name() == "to"
			|| property->name() == "name")
		{
			qDebug() << "ERROR: predefined property" << property->name()
				<< "shall not appear in metamodels, ignored";
			continue;
		}
		QString tmp = property->generatePropertyLine(lineTemplate) + "\n";
		propertiesString += tmp.replace(elementNameTag, name());
	}
	return propertiesString;
}

QString GraphicType::generatePropertyDefaults(const QString &lineTemplate) const
{
	if (!mIsVisible)
		return "";
	QString defaultsString;
	foreach (Property *property, mProperties) {
		QString tmp = property->generateDefaultsLine(lineTemplate);
		if (!tmp.isEmpty())
			defaultsString += tmp.replace(elementNameTag, name()) + "\n";
	}
	return defaultsString;
}

QString GraphicType::generateContainers(const QString &lineTemplate) const
{
	if (!isGraphicalType() || mContains.isEmpty())
		return "";
	QString containersList;
	QString line = lineTemplate;
	foreach(QString contains, mContains) {
		containersList += "<< \"" + contains + "\" ";
	}
	line.replace(containersListTag, containersList).replace(elementNameTag, name());
	return line;
}

QString GraphicType::generateConnections(const QString &lineTemplate) const
{
	if (!isGraphicalType() || mConnections.isEmpty())
		return "";
	QString connectionsList;
	QString line = lineTemplate;
	foreach(QString connection, mConnections) {
		connectionsList += "<< \"" + connection + "\" ";
	}
	line.replace(connectionsListTag, connectionsList).replace(elementNameTag, name());
	return line;
}

QString GraphicType::generateUsages(const QString &lineTemplate) const
{
	if (!isGraphicalType() || mUsages.isEmpty())
		return "";
	QString usagesList;
	QString line = lineTemplate;
	foreach(QString usage, mUsages) {
		usagesList += "<< \"" + usage + "\" ";
	}
	line.replace(usagesListTag, usagesList).replace(elementNameTag, name());
	return line;
}

QString GraphicType::generateEnums(const QString &lineTemplate) const
{
	Q_UNUSED(lineTemplate);
	return "";
}
