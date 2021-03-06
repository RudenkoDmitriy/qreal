#include "exploserView.h"

#include <QtWidgets/QApplication>

#include <qrgui/models/models.h>
#include <qrgui/models/commands/createElementCommand.h>
#include <qrgui/dialogs/metamodelingOnFly/propertiesDialog.h>

#include "editor/editorViewScene.h"
#include "editor/sceneCustomizer.h"
#include "editor/commands/expandCommand.h"

using namespace qReal;
using namespace view::details;

ExploserView::ExploserView(const models::Models &models
		, Controller &controller
		, const SceneCustomizer &customizer
		, QObject *parent)
	: QObject(parent)
	, mLogicalApi(models.logicalModelAssistApi())
	, mGraphicalApi(models.graphicalModelAssistApi())
	, mExploser(models.exploser())
	, mController(controller)
	, mCustomizer(customizer)
{
}

void ExploserView::createAddExplosionMenu(const Element * const element
		, QMenu &contextMenu, QList<Explosion> const &explosions
		, const Id &alreadyConnectedElement) const
{
	bool hasAnyActions = false;
	const QString menuName = alreadyConnectedElement.isNull()
			? mCustomizer.addExplosionMenuName()
			: mCustomizer.changeExplosionMenuName();
	QMenu *addExplosionMenu = new QMenu(menuName);

	for (const Explosion &explosion : explosions) {
		for (const Id &elementId : mLogicalApi.logicalRepoApi().logicalElements(explosion.target())) {
			if (alreadyConnectedElement == elementId) {
				continue;
			}
			QAction *action = addExplosionMenu->addAction(mLogicalApi.logicalRepoApi().name(elementId));
			hasAnyActions = true;
			connect(action, SIGNAL(triggered()), SLOT(addExplosionActionTriggered()));
			QList<QVariant> tag;
			tag << element->logicalId().toVariant() << elementId.toVariant();
			action->setData(tag);
		}
	}

	if (hasAnyActions) {
		addExplosionMenu->addSeparator();
	}

	for (const Explosion &explosion : explosions) {
		const Id diagramType = mLogicalApi.editorManagerInterface().findElementByType(explosion.target().element());
		const QString name = mLogicalApi.editorManagerInterface().friendlyName(diagramType);
		const QString editorName = mLogicalApi.editorManagerInterface().friendlyName(Id(diagramType.editor()));
		QAction *action = addExplosionMenu->addAction(tr("New ") + editorName + "/" + name);
		hasAnyActions = true;
		connect(action, SIGNAL(triggered()), SLOT(addExplosionActionTriggered()));
		action->setData(QVariantList() << element->logicalId().toVariant() << diagramType.toVariant());
	}
	contextMenu.addMenu(addExplosionMenu);

	if (alreadyConnectedElement != Id()) {
		QAction * const gotoAction = contextMenu.addAction(mCustomizer.goToConnectedMenuName()
				, this, SLOT(goToActionTriggered()));
		gotoAction->setData(alreadyConnectedElement.toVariant());
	}
}

void ExploserView::createRemoveExplosionMenu(const Element * const element, QMenu &contextMenu
		, const Id &outgoingConnection) const
{
	if (outgoingConnection.isNull()) {
		return;
	}

	QAction * const action = contextMenu.addAction(mCustomizer.deleteExplosionMenuName());
	connect(action, SIGNAL(triggered()), SLOT(removeExplosionActionTriggered()));
	action->setData(QVariantList() << element->logicalId().toVariant() << outgoingConnection.toVariant());
}

void ExploserView::createExpandAction(const Element * const element, QMenu &contextMenu
		, const Id &alreadyConnectedElement) const
{
	if (alreadyConnectedElement.isNull()) {
		return;
	}

	const NodeElement * const node = dynamic_cast<const NodeElement * const>(element);
	if (!node) {
		return;
	}

	QAction *expandAction = contextMenu.addAction(node->isExpanded()
			? mCustomizer.collapseExplosionActionText()
			: mCustomizer.expandExplosionActionText());
	connect(expandAction, SIGNAL(triggered()), SLOT(expandExplosionActionTriggered()));

	expandAction->setData(element->id().toVariant());
}

void ExploserView::createConnectionSubmenus(QMenu &contextMenu, const Element * const element) const
{
	if (mLogicalApi.editorManagerInterface().isInterpretationMode()) {
		contextMenu.addSeparator();
		QAction * const changePropertiesAction = contextMenu.addAction(tr("Change Properties"));
		connect(changePropertiesAction, SIGNAL(triggered()), SLOT(changePropertiesActionTriggered()));
		changePropertiesAction->setData(element->id().toVariant());
		QAction * const changeAppearancePaletteAction = contextMenu.addAction(tr("Change Appearance"));
		connect(changeAppearancePaletteAction, SIGNAL(triggered()), SLOT(changeAppearanceActionTriggered()));
		changeAppearancePaletteAction->setData(element->id().toVariant());
		if (mLogicalApi.editorManagerInterface().getIsHidden(element->id()) == "true")
		{
			QAction * const addElementToPaletteAction = contextMenu.addAction(tr("Add element to palette"));
			connect(addElementToPaletteAction, SIGNAL(triggered()), SLOT(addElementToPaletteActionTriggered()));
			addElementToPaletteAction->setData(element->id().toVariant());
		}
	}

	QList<Explosion> const explosions = mLogicalApi.editorManagerInterface().explosions(element->id().type());
	if (explosions.isEmpty() || (explosions.count() == 1 && explosions[0].requiresImmediateLinkage())) {
		return;
	}

	// menu items "connect to"
	// TODO: move to elements, they can call the model and API themselves
	createAddExplosionMenu(element, contextMenu, explosions
			, mLogicalApi.logicalRepoApi().outgoingExplosion(element->logicalId()));

	createRemoveExplosionMenu(element, contextMenu
			, mLogicalApi.logicalRepoApi().outgoingExplosion(element->logicalId()));

	createExpandAction(element, contextMenu
			, mLogicalApi.logicalRepoApi().outgoingExplosion(element->logicalId()));
}

void ExploserView::handleDoubleClick(const Id &id)
{
	Id outgoingLink = mLogicalApi.logicalRepoApi().outgoingExplosion(id);
	if (outgoingLink.isNull()) {
		QList<Explosion> const explosions = mLogicalApi.editorManagerInterface().explosions(id);
		if (!explosions.isEmpty()) {
			const Id diagramType = mLogicalApi.editorManagerInterface()
					.findElementByType(explosions[0].target().element());
			commands::AbstractCommand *createCommand =
					mExploser.createElementWithIncomingExplosionCommand(
							id, diagramType, mGraphicalApi);
			mController.executeGlobal(createCommand);
			outgoingLink = static_cast<commands::CreateElementCommand *>(createCommand)->result();
		}
	}
	goTo(outgoingLink);
}

void ExploserView::handleCreationWithExplosion(commands::AbstractCommand *createCommand
		, const Id &source, const Id &target)
{
	if (target != Id()) {
		createCommand->addPostAction(mExploser.addExplosionCommand(source, target, &mGraphicalApi));
	} else {
		QList<Explosion> const explosions = mLogicalApi.editorManagerInterface().explosions(source);
		for (const Explosion &explosion : explosions) {
			if (explosion.source().type() == source.type() && explosion.requiresImmediateLinkage()) {
				createCommand->addPostAction(mExploser.createElementWithIncomingExplosionCommand(
						source, explosion.target(), mGraphicalApi));
			}
		}
	}
}

void ExploserView::addExplosionActionTriggered()
{
	QAction *action = static_cast<QAction *>(sender());
	const QList<QVariant> connection = action->data().toList();
	const Id source = connection[0].value<Id>();
	const Id destination = connection[1].value<Id>();
	if (action->text().startsWith(tr("New "))) {
		mController.execute(mExploser.createElementWithIncomingExplosionCommand(
				source, destination, mGraphicalApi));
	} else {
		mController.execute(mExploser.addExplosionCommand(source, destination));
	}
}

void ExploserView::goToActionTriggered()
{
	emit goTo(static_cast<QAction *>(sender())->data().value<Id>());
}

void ExploserView::removeExplosionActionTriggered()
{
	QAction *action = static_cast<QAction *>(sender());
	const QList<QVariant> connection = action->data().toList();
	const Id source = connection[0].value<Id>();
	const Id destination = connection[1].value<Id>();
	mController.execute(mExploser.removeExplosionCommand(source, destination));
}

void ExploserView::expandExplosionActionTriggered()
{
	QAction *action = static_cast<QAction *>(sender());
	const Id elem = action->data().value<Id>();
	emit expandElement(elem);
}

void ExploserView::changePropertiesActionTriggered()
{
	const QAction * const action = static_cast<const QAction *>(sender());
	const Id id = action->data().value<Id>();
	qReal::gui::PropertiesDialog * const propertiesDialog = new qReal::gui::PropertiesDialog(
			mLogicalApi.editorManagerInterface()
			, mLogicalApi.mutableLogicalRepoApi()
			, id
			, QApplication::allWidgets().isEmpty() ? nullptr : QApplication::allWidgets()[0]);
	propertiesDialog->setModal(true);
	propertiesDialog->show();
}

void ExploserView::changeAppearanceActionTriggered()
{
	const QAction * const action = static_cast<const QAction *>(sender());
	const Id id = action->data().value<Id>();
	const QString propertyValue = mLogicalApi.editorManagerInterface().shape(id);
	emit openShapeEditor(id, propertyValue, &mLogicalApi.editorManagerInterface(), false);
}

void ExploserView::addElementToPaletteActionTriggered()
{
	const QAction * const action = static_cast<const QAction *>(sender());
	const Id id = action->data().value<Id>();
	mLogicalApi.editorManagerInterface().resetIsHidden(id);
	emit refreshPalette();
}
