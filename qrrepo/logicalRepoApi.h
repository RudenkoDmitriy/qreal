#pragma once

#include <qrkernel/roles.h>

#include "commonRepoApi.h"

namespace qrRepo {

/// Methods for working with logical model in a repository.
class LogicalRepoApi : public CommonRepoApi
{
public:
	virtual ~LogicalRepoApi(){}

	virtual void addChild(const qReal::Id &id, const qReal::Id &child) = 0;
	virtual qReal::Id otherEntityFromLink(const qReal::Id &linkId, const qReal::Id &firstNode) const = 0;

	virtual qReal::Id outgoingExplosion(const qReal::Id &id) const = 0;
	virtual qReal::IdList incomingExplosions(const qReal::Id &id) const = 0;
	virtual void addExplosion(const qReal::Id &source, const qReal::Id &destination) = 0;
	virtual void removeExplosion(const qReal::Id &source, const qReal::Id &destination) = 0;

	virtual qReal::IdList logicalElements(const qReal::Id &type) const = 0;
	virtual bool isLogicalElement(const qReal::Id &id) const = 0;

	virtual qReal::IdList elementsByType(const QString &type, bool sensitivity = false
			, bool regExpression = false) const = 0;
	virtual int elementsCount() const = 0;

	/// Returns a list of keys by that stored some meta-information.
	virtual QStringList metaInformationKeys() const = 0;

	/// Returns the meta-information about current model stored by the given key.
	virtual QVariant metaInformation(const QString &key) const = 0;

	/// Stores the meta-information for current stored binded to the given key.
	virtual void setMetaInformation(const QString &key, const QVariant &info) = 0;
};
}
