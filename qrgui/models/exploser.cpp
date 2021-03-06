#include "exploser.h"

#include "models/logicalModelAssistApi.h"
#include "models/commands/explosionCommand.h"
#include "models/commands/createElementCommand.h"
#include "models/commands/renameExplosionCommand.h"
#include "models/commands/renameCommand.h"
#include "models/commands/createGroupCommand.h"
#include "controller/commands/doNothingCommand.h"

using namespace qReal;
using namespace models;
using namespace commands;

Exploser::Exploser(LogicalModelAssistApi &api)
	: mApi(api)
{
}

QMultiMap<Id, Id> Exploser::explosions(const Id &diagram) const
{
	QMultiMap<Id, Id> result;

	const IdList childTypes = mApi.editorManagerInterface().elements(diagram);

	for (const Id &child : childTypes) {
		QList<Explosion> const explosions = mApi.editorManagerInterface().explosions(child);

		for (const Explosion &explosion : explosions) {
			if (!explosion.isReusable()) {
				continue;
			}

			const Id targetNodeOrGroup = explosion.target();
			Id target;
			if (mApi.editorManagerInterface().isNodeOrEdge(targetNodeOrGroup.editor(), targetNodeOrGroup.element())) {
				target = targetNodeOrGroup;
			} else {
				const Pattern pattern = mApi.editorManagerInterface().getPatternByName(targetNodeOrGroup.element());
				target = Id(targetNodeOrGroup.editor(), targetNodeOrGroup.diagram(), pattern.rootType());
			}

			const IdList allTargets = mApi.logicalRepoApi().logicalElements(target.type());
			for (const Id &targetInstance : allTargets) {
				result.insertMulti(child, targetInstance);
			}
		}
	}

	return result;
}

IdList Exploser::elementsWithHardDependencyFrom(const Id &id) const
{
	IdList result;
	const Id targetType = id.type();
	const IdList incomingExplosions = mApi.logicalRepoApi().incomingExplosions(id);
	foreach (const Id &incoming, incomingExplosions) {
		QList<Explosion> const explosions = mApi.editorManagerInterface().explosions(incoming.type());
		foreach (const Explosion &explosion, explosions) {
			if (explosion.target() == targetType && explosion.requiresImmediateLinkage()) {
				result << incoming;
			}
		}
	}

	return result;
}

void Exploser::handleRemoveCommand(const Id &logicalId, AbstractCommand * const command)
{
	const Id outgoing = mApi.logicalRepoApi().outgoingExplosion(logicalId);
	if (!outgoing.isNull()) {
		command->addPreAction(new ExplosionCommand(mApi, nullptr, logicalId, outgoing, false));
	}

	const Id targetType = logicalId.type();
	const IdList incomingExplosions = mApi.logicalRepoApi().incomingExplosions(logicalId);
	foreach (const Id &incoming, incomingExplosions) {
		QList<Explosion> const explosions = mApi.editorManagerInterface().explosions(incoming.type());
		foreach (const Explosion &explosion, explosions) {
			if (explosion.target() == targetType && !explosion.requiresImmediateLinkage()) {
				command->addPreAction(new ExplosionCommand(mApi, nullptr, incoming, logicalId, false));
			}
		}
	}
}

AbstractCommand *Exploser::createElementWithIncomingExplosionCommand(const Id &source
		, const Id &targetType, GraphicalModelAssistApi &graphicalApi)
{
	AbstractCommand *result = nullptr;
	Id newElementId;
	if (mApi.editorManagerInterface().isNodeOrEdge(targetType.editor(), targetType.element())) {
		const QString friendlyTargetName = mApi.editorManagerInterface().friendlyName(targetType);
		newElementId = Id(targetType, QUuid::createUuid().toString());
		result = new CreateElementCommand(mApi, graphicalApi, *this, Id::rootId()
				, Id::rootId(), newElementId, false, friendlyTargetName, QPointF());
	} else {
		result = new CreateGroupCommand(mApi, graphicalApi, *this, Id::rootId()
				, Id::rootId(), targetType, false, QPointF());
		newElementId = static_cast<CreateGroupCommand *>(result)->rootId();
	}

	result->addPostAction(addExplosionCommand(source, newElementId, &graphicalApi));
	result->addPostAction(new RenameExplosionCommand(mApi, &graphicalApi, *this, newElementId));
	connect(result, SIGNAL(undoComplete(bool)), this, SIGNAL(explosionTargetRemoved()));
	return result;
}

IdList Exploser::explosionsHierarchy(const Id &oneOfIds) const
{
	// Infinite cycle may happen here in case of cyclic explosions
	const Id root = explosionsRoot(oneOfIds);
	IdList hierarchy;
	explosionsHierarchyPrivate(root, hierarchy);
	return hierarchy;
}

Id Exploser::immediateExplosionTarget(const Id &id)
{
	QList<Explosion> const explosions = mApi.editorManagerInterface().explosions(id.type());
	if (explosions.size() == 1 && explosions[0].requiresImmediateLinkage()) {
		return explosions[0].target();
	}
	return Id();
}

AbstractCommand *Exploser::addExplosionCommand(const Id &source, const Id &target
		, GraphicalModelAssistApi * const graphicalApi)
{
	AbstractCommand *result = new ExplosionCommand(mApi, graphicalApi, source, target, true);
	connectCommand(result);
	return result;
}

AbstractCommand *Exploser::removeExplosionCommand(const Id &source, const Id &target)
{
	AbstractCommand *result = new ExplosionCommand(mApi, nullptr, source, target, false);
	connectCommand(result);
	return result;
}

AbstractCommand *Exploser::renameCommands(const Id &oneOfIds, const QString &newNames) const
{
	DoNothingCommand *result = new DoNothingCommand;

	const IdList idsToRename = explosionsHierarchy(oneOfIds);
	foreach (const Id &id, idsToRename) {
		result->addPostAction(new RenameCommand(mApi, id, newNames));
	}

	if (!idsToRename.isEmpty()) {
		connectCommand(result);
	}

	return result;
}

void Exploser::explosionsHierarchyPrivate(const Id &currentId, IdList &targetIds) const
{
	targetIds << currentId;
	const IdList incomingExplosions = mApi.logicalRepoApi().incomingExplosions(currentId);
	foreach (const Id incoming, incomingExplosions) {
		explosionsHierarchyPrivate(incoming, targetIds);
	}
}

void Exploser::connectCommand(const AbstractCommand *command) const
{
	// Do not remove Qt::QueuedConnection flag.
	// Immediate refreshing may cause segfault because of deleting drag source.
	connect(command, &AbstractCommand::undoComplete, this, &Exploser::explosionsSetCouldChange, Qt::QueuedConnection);
	connect(command, &AbstractCommand::redoComplete, this, &Exploser::explosionsSetCouldChange, Qt::QueuedConnection);
}

Id Exploser::explosionsRoot(const Id &id) const
{
	Id currentId = id, previousId;
	do {
		previousId = currentId;
		currentId = mApi.logicalRepoApi().outgoingExplosion(currentId);
	} while (currentId != Id());

	return previousId;
}
