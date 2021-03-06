#include "element.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QGraphicsColorizeEffect>

#include <qrkernel/settingsListener.h>

#include "models/commands/changePropertyCommand.h"

using namespace qReal;

const qreal disabledEffectStrength = 0.9;

Element::Element(ElementImpl *elementImpl
		, const Id &id
		, qReal::models::GraphicalModelAssistApi &graphicalAssistApi
		, qReal::models::LogicalModelAssistApi &logicalAssistApi
		)
	: mMoving(false)
	, mEnabled(true)
	, mId(id)
	, mElementImpl(elementImpl)
	, mLogicalAssistApi(logicalAssistApi)
	, mGraphicalAssistApi(graphicalAssistApi)
	, mController(nullptr)
{
	setFlags(ItemIsSelectable | ItemIsMovable | ItemIsFocusable | ItemClipsChildrenToShape |
			ItemClipsToShape | ItemSendsGeometryChanges);

	setAcceptDrops(true);
	setCursor(Qt::PointingHandCursor);

	updateEnabledState();
	setHideNonHardLabels(SettingsManager::value("hideNonHardLabels").toBool());
	SettingsListener::listen("hideNonHardLabels", this, &Element::setHideNonHardLabels);
}

Id Element::id() const
{
	return mId;
}

qReal::Id Element::logicalId() const
{
	return mGraphicalAssistApi.logicalId(mId);
}

QString Element::name() const
{
	return mGraphicalAssistApi.name(id());
}

void Element::updateData()
{
	setToolTip(mGraphicalAssistApi.toolTip(id()));
}

QList<ContextMenuAction*> Element::contextMenuActions(const QPointF &pos)
{
	Q_UNUSED(pos)
	return QList<ContextMenuAction*>();
}

QString Element::logicalProperty(const QString &roleName) const
{
	return mLogicalAssistApi.propertyByRoleName(logicalId(), roleName).toString();
}

void Element::setLogicalProperty(const QString &roleName, const QString &value, bool withUndoRedo)
{
	commands::AbstractCommand *command = new commands::ChangePropertyCommand(&mLogicalAssistApi
			, roleName, logicalId(), value);
	if (withUndoRedo) {
		mController->execute(command);
	} else {
		command->redo();
		delete command;
	}
}

void Element::setController(Controller *controller)
{
	mController = controller;
}

qReal::Controller * Element::controller() const
{
	return mController;
}

void Element::initTitles()
{
}

void Element::select(const bool singleSelected)
{
	if (singleSelected) {
		setSelectionState(true);
	}

	emit switchFolding(!singleSelected);
}

void Element::setSelectionState(const bool selected)
{
	if (isSelected() != selected) {
		setSelected(selected);
	}
	if (!selected) {
		select(false);
	}

	foreach (Label * const label, mLabels) {
		label->setParentSelected(selected);
	}
}

ElementImpl* Element::elementImpl() const
{
	return mElementImpl;
}

bool Element::createChildrenFromMenu() const
{
	return mElementImpl->createChildrenFromMenu();
}

void Element::updateEnabledState()
{
	const bool enabled = mLogicalAssistApi.editorManagerInterface().elements(
			Id(mId.editor(), mId.diagram())).contains(mId.type());

	mEnabled = enabled;
	if (mEnabled) {
		setGraphicsEffect(nullptr);
		setOpacity(1);
	} else {
		QGraphicsColorizeEffect * const grayScale = new QGraphicsColorizeEffect(this);
		grayScale->setColor(Qt::gray);
		grayScale->setStrength(disabledEffectStrength);
		setGraphicsEffect(grayScale);
		setOpacity(disabledEffectStrength);
	}
}

void Element::setHideNonHardLabels(bool hide)
{
	for (const Label * const label : mLabels) {
		if (label->isSelected()) {
			return;
		}
	}

	for (Label * const label : mLabels) {
		label->setVisible(label->isHard() || !hide);
	}
}

void Element::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_F2) {
		for (Label * const label : mLabels) {
			if (!label->isReadOnly()) {
				label->startTextInteraction();
				event->accept();
				return;
			}
		}
	}

	QGraphicsItem::keyPressEvent(event);
}
